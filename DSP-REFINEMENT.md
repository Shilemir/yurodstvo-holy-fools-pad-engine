# DSP refinement — introduced in 0.8.1, published in 0.10.1

These implemented changes are included in the v0.10.1 public prerelease.
Instrument identity, layout, illustration and existing controller assignments
remain preserved. See the release notes for changes since the previous public build.

## Pitch Bend

The previous processor routed the wheel into the relative oscillator-II detune
control. Pitch now has its own 25 ms smoother, shared by every voice and voice
stealing tail. Oscillator frequencies are `noteHz * bendRatio` and
`noteHz * bendRatio * relativeDetuneRatio`. Thus their interval is preserved.
The standard 14-bit MIDI range is exactly -100 / 0 / +100 cents at 0 / 8192 /
16383. Hardware gestures remain transient, whereas the existing `pitchDetune`
parameter remains automatable and saved. Its historical ID and host name are
retained. The old `detune` parameter retains its ID and +/-50 cent range as an
independent relative offset. Changes to relative detune no longer cancel a wheel
gesture. Both on-screen and physical wheels control the same global bend.

Saved sessions load with the same parameter IDs, values and VST3 identity, but
old sessions using pitch automation or effects will intentionally sound different
under this new processing. Keep the old binaries to reproduce their old sound.

## Cathedral Reverb (E5)

Previously the JUCE room-size macro stopped at 0.87 and the wet return was scaled
by 0.42. The replacement uses eight unequal 80–183 ms delay lines with a normalized
Hadamard feedback matrix, four input all-pass diffusers, 27 ms pre-delay, damping
and subsonic rejection. The matrix preserves energy before feedback losses;
per-line decay gain is always less than one. The requested low-frequency RT60
is `1.4 + 28.6 * amount²` seconds; damping makes broadband decay shorter and
frequency-dependent. This is a design setting, not a measured guarantee of a
30-second audible tail for every note.

The return increases to 2.0 times the unscaled network output, while the source
dry gain reaches 0.5 at maximum. This deliberate wet/dry mix is not automatic
level compensation. Macro changes are smoothed over 60 ms. Buffer storage is
allocated during preparation, not during per-sample processing.

## Deep Ambient Delay (E6)

The old return reached only 0.40, feedback 0.65. The new return reaches 1.15
(+9.17 dB for the first repeat compared with 0.40, at normal linear levels),
with feedback from 0.22 to 0.88. Unequal 570 / 760 ms stereo delays are retained;
65 percent cross-feedback distributes subsequent repeats across channels.
A 2.8 kHz low-pass softens successive repeats. Loop protection is linear below
its safety knee, unlike the old always-active tanh saturation.

Reverb, Delay and the unchanged Shimmer use parallel sends. Their returns do
not turn one another down. Master gain and the existing 0.94 output ceiling
apply after summation. Extreme settings can engage that final soft knee;
there is no automatic gain compensation driven by effect or EQ parameters.

## Listening and controller check in Ableton

1. Close the DAW, keep the previous VST3 bundle as a backup, replace the complete
   bundle with the 0.8.1 build, and rescan plugins. Do not install two bundles with
   the same internal identity at once.
2. Disable effects, hold a chord, move Pitch Bend both ways and release it.
   Both oscillator timbres should bend together and return. Repeat at both
   extremes of Oscillator Mix.
3. Play short notes with short Release. Compare E5 at 0, 25, 50, 75 and 100 percent;
   repeat for E6. Then try a long chord with all three effects enabled.
4. Check P1–P8, E1–E6, F1–F4, Mod and the mouse keyboard; save and reopen the set.
   Parameters should restore; a temporary physical wheel gesture should not.

Automated results and measured impulse energies are saved with the local builds.
Listening, physical MiniLab testing and interaction in Ableton were not performed
by the agent during this iteration.
