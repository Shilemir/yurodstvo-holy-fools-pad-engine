YURODSTVO 0.10.1

# Pray for Inspiration, recurring Harmony and Release Glow

Included in the Windows x64 v0.10.1 prerelease. Instrument name, VST3 identity, all APVTS parameter IDs,
controller CC assignments, artwork, font and window size are unchanged.

## PRAY FOR INSPIRATION

The single header button generates fresh settings for E1-E8, the three EQ bands
and both independent waveform choices. Matching oscillator waveforms are valid.
Master, legacy oscillator detune, Doubt, Faith, polyphony, MIDI routing and host
configuration are excluded. There is no randomization-specific gain compensation.

Knob values span their existing ranges. Attack and Release use mostly logarithmic
time distributions with occasional linear-time draws; Release favours longer
values. EQ usually favours the centre but also permits full-range results.
Continuous controls and discrete waveform choices are independently drawn, not
selected from a preset list. Normal host parameter gestures notify the DAW;
the generated values save and automate just like manual edits. Existing host
automation can still take ownership of a parameter, as with any mouse edit.

Randomization runs on the GUI/message thread, not the audio thread. It changes
existing parameter objects, not DSP internals. Audio rendering retains the
existing gain/effect/voice smoothing and waveform crossfades; held notes are not
stopped. The warm button flash lasts roughly 0.17 seconds.

### Hardware pickup after a prayer

Because MiniLab controls are not motorized, the eleven affected absolute controls
use pickup after randomization. Move a knob or EQ fader through the new displayed
position to take control. Reaching within about one MIDI step or crossing that
position enables normal absolute control. Master and pads never enter pickup.
Pickup follows the current parameter value if it is subsequently edited by mouse.
Its transient flags are cleared when audio is prepared or a state is loaded;
they do not alter saved parameter values or the established MIDI mapping.

## Recurring HARMONY

The original modal sound and its oscillator-linked layer remain. An additional
six-mode bank per voice receives short, shaped 20 ms excitations. Its initial
strike and later strikes have their own gain envelope, independent of the main
oscillator Attack. Long 20-30 second PATIENCE settings therefore no longer hide
the entire bell layer.

Per-voice sample counters schedule new strikes after 3-9 seconds. Independent
seeds vary their timing, strength and modal colour; recurring events are not
synchronised across a chord. The bank tracks the same note/global pitch and
bounded drift as Harmony. Its existing mode-dependent decay is retained.
At zero Harmony the bank and scheduling are disabled after the macro smoothing.

Note Off stops new strikes unless the sustain pedal is still holding that voice.
Pedal release stops scheduling too. Existing resonances fade through a bounded
release gate of 4-12 seconds, in addition to their natural modal decay. They can
outlast a very short oscillator Release. Voice stealing, polyphony reduction and
Panic use the existing short retirement fade for both layers. No extra untracked
voices, audio-thread allocations, GUI timers or blocking operations are used.

## RELEASE GLOW

The engine exports the maximum main amplitude-envelope level for each MIDI pitch
across active voices and retiring voice-stealing tails. The processor publishes
128 atomic floats once per audio block; the existing 30 FPS GUI blends the warm
key colour from those levels. There is no separate visual decay timer.

Attack, Release, sustained notes, retriggering, multiple owners of one pitch and
voice stealing therefore share the audio envelope's actual state. Glow follows
the main voice envelope, not independent bell resonances or global reverb/delay/
Shimmer tails. Those may remain audible after a key has gone dark. Immediate
mouse ownership retains a subtle outline while the audio state catches up.

## Quick test in your DAW

1. Back up the old bundle, close the DAW, replace the whole VST3 bundle and rescan.
2. Hold a chord and press PRAY repeatedly. Check knobs, EQ and pad highlights;
   Master, Doubt, Faith and polyphony must keep their settings. Test pickup.
3. Set PATIENCE to 30 seconds and HARMONY high. Hold a note for 20-30 seconds;
   listen for slow returning bells. Release it, including the sustain pedal:
   existing tails may continue, but new strikes should stop.
4. Set REMEMBRANCE successively to 0.5, 5 and 30 seconds. Release a chord and
   observe its glow. Retrigger a pitch and try simultaneous mouse/MIDI playing.
5. Save/reopen the set to verify randomized parameter recall in your host.

Automated results are in TEST_REPORT.md. No physical MiniLab interaction or
Ableton listening test was performed by the agent during this iteration.
