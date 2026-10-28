# YURODSTVO 0.10.1 — Sacred Harmony and Controlled Madness

Included in the v0.10.1 public prerelease. The existing artwork, 1200 x 780
layout, font, oscillators, pad mapping, EQ and plugin identity are retained.

## Controls and compatibility

| Control | Interface name | Technical function | User / DAW CC | Parameter ID |
|---|---|---|---|---|
| E1 | DEVOTION | Octave Shimmer Reverb | 74 / 86 | shimmer |
| E2 | DUALISM | Oscillator Mix | 71 / 87 | mix |
| E3 | PATIENCE | Amplitude Attack Time | 76 / 89 | attack |
| E4 | REMEMBRANCE | Amplitude Release Time | 77 / 90 | release |
| E5 | SANCTUARY | Cathedral Reverb | 93 / 110 | reverb |
| E6 | SOLITUDE | Deep Ambient Delay | 18 / 111 | delay |
| E7 | HARMONY | Sacred Resonance | 19 / 116 | harmony (new) |
| E8 | MADNESS | Controlled Instability | 16 / 117 | madness (new) |
| F1 | MASTER | Master Volume | 82 / 14 | master |
| F2 | HELL | 120 Hz low shelf, +/-12 dB | 83 / 15 | eqBass |
| F3 | EARTH | 1 kHz bell, +/-12 dB | 85 / 30 | eqMid |
| F4 | HEAVEN | 6 kHz high shelf, +/-12 dB | 17 / 31 | eqTreble |
| Pitch | DOUBT | Global +/-100 cent pitch bend | 14-bit Pitch Bend | pitchDetune (existing) |
| Mod | FAITH | Visual position only | CC1 | transient controller |

CC assignments follow the [official Arturia implementation table](https://support.arturia.com/hc/en-us/articles/6189475866396-MiniLab-3-General-Questions).
Existing controls retain their priority over pad routing on every MIDI channel.
P1-P4 still select oscillator I's four waveforms; P5-P8 select oscillator II's.

Hover over a control for its technical explanation. The artistic names are used
in the editor; existing host automation names deliberately remain technical
(Shimmer, Attack, Release, etc.). This protects scripts or mappings using names
as well as stable IDs. Harmony and Madness are appended after all old parameters,
are automatable, and save in the existing ValueTree format. State schema 8 adds
only those two parameters; absent values in older presets migrate to zero,
including when an existing instance is reused. Original parameter ranges and
the VST3 component/controller IDs are unchanged.

## HARMONY

Each synthesizer voice has six actual damped complex resonators, with relative
frequencies 1.0, 2.01, 2.756, 4.07, 5.43 and 6.8. The combination includes
harmonic and bell-like inharmonic modes. They track the note and global bend;
their frequency coefficients interpolate at a 64-sample control interval.
Modes fade out near Nyquist instead of aliasing into the audible band.

A bounded onset strike and the voice signal excite the bank. A small contribution
of the same oscillator's band-limited saw supplies harmonically rich excitation
even when both selected oscillators are sine waves. This exciter is passed through
the resonators, not used as a separate audible sampler or bell recording.

The macro increases the modal return and extends the first mode's nominal decay
from four to twelve seconds; higher modes decay faster. The bank shares the
voice's amplitude envelope and velocity, so REMEMBRANCE also controls how long
the resonances remain audible after Note Off. Voice stealing and polyphony
reduction retain the existing short fade; there are no untracked extra voices.
Use a longer REMEMBRANCE for sustained bell layers. In 0.10.0 an additional recurring layer has its own excitation/release gate, so long Attack no longer hides all bell strikes. See [Inspiration](INSPIRATION.md).

## MADNESS and FAITH

Each voice has five independently seeded cubic-interpolated random trajectories:
oscillator I drift, oscillator II drift, amplitude, timbre and panorama.
Their targets change on unsynchronised periods of roughly 1.7 to 7 seconds,
with zero slope at segment boundaries. They run during a held note without
requiring another Note On. New notes receive distinct seeds, without shared RNG
state, allocation or locks on the audio thread.

The effective intensity is `2 * Madness`, with the existing 80 ms E8 smoothing.
This is the same maximum expression depth as version 0.9.0 with Hesitation
settled at MIDI 127, with no second scaling stage. Maximum drift is +/-24 cents
per oscillator, amplitude variation +/-30 percent and stereo pan +/-0.6.
The organic modulation algorithm and timbre blend are otherwise unchanged.
At Madness zero the module is entirely bypassed.

FAITH retains the physical MIDI CC1 and mouse-driven display position, but
has no connection to the synthesis engine. Its position is transient and resets
on state load/audio preparation. No APVTS parameter was removed: the previous
Hesitation was also only transient CC1 state. Old CC1 events are accepted and
shown, but cannot alter any audio parameter. Older presets with nonzero Madness
now use full expression depth regardless of their old CC1 position; this is the
intentional sonic change. Existing pitch, EQ, FX and waveform settings retain
their behaviour. DOUBT remains an independent global pitch bend.

Stereo voice output passes through the existing stereo EQ and parallel effects.
Their feedback architecture is unchanged. Existing final soft peak protection
remains active, with no new automatic gain compensation. Extreme dense chords
can engage that protection; it is not a promise of unchanged dynamics at every
maximum setting.

## Suggested listening test

1. Back up the previous plugin bundle, close the DAW, replace the complete VST3
   bundle and rescan. Avoid installing two bundles with the same identity.
2. Start with Harmony and Madness at zero. An older set should retain its 0.8.1
   sound. Check all pads, E1-E6, the four faders, DOUBT and the mouse keyboard.
3. Play a sine chord with a long REMEMBRANCE. Raise HARMONY through 25, 50, 75 and
   100 percent, then add SANCTUARY and DEVOTION. Try shorter notes to hear onsets.
4. Hold one note and move MADNESS through 25, 50, 75 and 100 percent. Move FAITH
   with the physical strip and mouse: the display should move without changing
   sound, even when Madness is active. Check DOUBT during a long chord.
5. Save and reopen the DAW set. Harmony/Madness and existing settings should
   restore; transient hardware expression positions should reset.

Automated validation is recorded in TEST_REPORT.md. Physical MiniLab interaction,
Ableton listening and another-computer compatibility have not been tested by
the agent during this iteration.
