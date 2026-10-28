# YURODSTVO 1.1.0 — stable update

The editor displays **v1.1**. This stable update replaces the official 1.0.0
download. Windows x64 Release builds and their complete Corresponding Source
are available from the v1.1.0 GitHub Release.

## Calibrated factory sounds

| Parameter | The Hermit | The Martyr | The Inquisitor | The Hierophant (unchanged) |
|---|---:|---:|---:|---:|
| Oscillator I | Sine | Triangle | Square | Triangle |
| Oscillator II | Triangle | Sine | Sine | Sine |
| Devotion | 18.4% | 5.4% | 100.0% | 80.0% |
| Dualism | -36.76% | -27.9% | -59.1% | 0.0% |
| Patience | 4.000 s | 1.011 s | 0.005 s | 3.000 s |
| Remembrance | 8.000 s | 13.905 s | 1.115 s | 15.000 s |
| Sanctuary | 54.1% | 23.7% | 35.0% | 90.0% |
| Solitude | 15.0% | 49.8% | 0.0% | 35.0% |
| Harmony | 33.0% | 100.0% | 100.0% | 100.0% |
| Madness | 8.0% | 100.0% | 66.6% | 15.0% |
| Hell | 0.0 dB | -0.6 dB | -7.5 dB | -1.0 dB |
| Earth | 0.0 dB | +0.3 dB | 0.0 dB | +2.0 dB |
| Heaven | 0.0 dB | +6.6 dB | +2.9 dB | +3.0 dB |

These are plain parameter values, not APVTS normalized values. Dualism uses
`mix = 0.5 + displayed_percent / 200`. Waveform choices are Sine=0,
Triangle=1, Sawtooth=2, Square=3. Factory selection leaves Master, Doubt,
Faith, polyphony and device settings untouched. The Fool's persistent custom
file and save/load format are unchanged.

New processors use The Hermit as their parameter defaults, with eight voices
and the existing Master default of 0.65. The editor recognizes a matching
archetype without changing parameters. Host state restoration takes precedence;
opening an editor never reloads the default sound.

## Standalone computer keyboard

| Key | A | W | S | E | D | F | T | G | Y | H | U | J | K |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Offset from visible C | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 |
| Note | C | C# | D | D# | E | F | F# | G | G# | A | A# | B | C |

The initial C is MIDI 48, called C2 by this instrument. Existing octave arrows
move both the visible and computer range; the waveform-pad octave is excluded.
Computer notes use the same engine, envelope, glow and voice counter as MIDI,
with independent note ownership so releasing a computer key does not release
an overlapping MIDI or mouse note. Velocity is fixed at 0.8.

Only the active Standalone editor accepts these keys. Text editors and
Ctrl/Alt/Command combinations are excluded. There are no global hooks and no
computer-key listener in VST3. Repeats do not retrigger notes. Focus loss,
editor closure, octave changes, state restore, audio reset and Silence cancel
computer notes; held keys must be released before being played again.
Key-state polling at 30 Hz supplements JUCE keyboard/focus callbacks.
Simultaneous key count depends on the physical keyboard's rollover support.

## Verified tooltip routing

Both CC columns below are real, existing MIDI input routes: MiniLab User
controls and MiniLab DAW mode. The DAW column does not mean VST3 automation
requires CC messages; native host parameter automation remains available.

| Control | MiniLab User CC | MiniLab DAW CC |
|---|---:|---:|
| Devotion | 74 | 86 |
| Dualism | 71 | 87 |
| Patience | 76 | 89 |
| Remembrance | 77 | 90 |
| Sanctuary | 93 | 110 |
| Solitude | 18 | 111 |
| Harmony | 19 | 116 |
| Madness | 16 | 117 |
| Hell | 82 | 14 |
| Earth | 83 | 15 |
| Heaven | 85 | 30 |
| Master | 17 | 31 |

Doubt uses MIDI Pitch Bend. Faith uses MIDI CC 1 and does not modulate audio.
No MIDI mapping or parameter ID has changed.

## Interface and scope

The warm brown monitor contains only MIDI event/note and output dBFS. Its
smaller height provides more separation between Polyphony and Archetypes.
Visible E1–E8 and F1–F4 labels are removed; artistic names, controls and values
remain. Tooltips use short English function names and verified routing.

DSP algorithms, the existing +2 dB Harmony calibration, VST3 identity,
parameter IDs, illustration, font, KJV inscription and AGPL-3.0-only licensing
are preserved. See [TEST_REPORT.md](TEST_REPORT.md) for validation and limits.
