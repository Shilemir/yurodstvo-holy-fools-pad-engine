#pragma once
#include <array>
#include <atomic>
#include <cstdint>

// One message-thread producer, one audio-thread consumer. Requests are absolute
// note targets, not MIDI channels: GUI notes must never share physical ownership.
class MouseNoteInput
{
    static_assert(std::atomic<uint64_t>::is_always_lock_free,"Mouse input requires lock-free 64-bit atomics");
public:
    void post(int note) noexcept
    {
        const uint64_t request = (static_cast<uint64_t>(++serial) << 8) | static_cast<uint64_t>(note + 1);
        // Publish the cancellation watermark before exposing a queued event.
        latest.store(request, std::memory_order_release);
        const unsigned w = write.load(std::memory_order_relaxed), next = (w + 1) % capacity;
        if (next != read.load(std::memory_order_acquire))
        {
            queue[w] = request; write.store(next, std::memory_order_release);
        }
        // Even if the bounded queue fills, latest retains the final release.
    }
    template<class Callback> void drain(Callback&& callback) noexcept
    {
        unsigned r = read.load(std::memory_order_relaxed);
        for (unsigned count = 0; count < capacity; ++count)
        {
            if (r == write.load(std::memory_order_acquire)) break;
            const auto request = queue[r]; r = (r + 1) % capacity;
            read.store(r, std::memory_order_release); deliver(request, callback);
        }
        deliver(latest.load(std::memory_order_acquire), callback);
    }
    // Audio lifecycle/panic cancels already submitted gestures without changing
    // the producer cursor or racing the SPSC queue. Stale events are skipped.
    uint32_t watermark() const noexcept { return static_cast<uint32_t>(latest.load(std::memory_order_acquire) >> 8); }
    void suppressThrough(uint32_t cutoff) noexcept
    {
        if (static_cast<int32_t>(cutoff - consumed) > 0) consumed = cutoff;
    }
    void suppress() noexcept { suppressThrough(watermark()); }
private:
    template<class Callback> void deliver(uint64_t request, Callback& callback) noexcept
    {
        const auto seq = static_cast<uint32_t>(request >> 8);
        if (static_cast<int32_t>(seq - consumed) <= 0) return;
        consumed = seq; callback(static_cast<int>(request & 255) - 1);
    }
    static constexpr unsigned capacity = 256;
    std::array<uint64_t, capacity> queue {};
    std::atomic<unsigned> write {0}, read {0};
    std::atomic<uint64_t> latest {0};
    uint32_t serial = 0, consumed = 0;
};
