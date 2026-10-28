YURODSTVO - Holy Fool's Synth for Arturia MiniLab3 v@VERSION@
Official stable release | Windows x64 VST3

INSTALLATION
1. Verify YURODSTVO-v@VERSION@-Windows-x64-VST3.zip with SHA256SUMS.txt.
2. Extract everything and close the DAW.
3. Copy the ENTIRE YURODSTVO - Holy Fool's Synth for Arturia MiniLab3.vst3 folder
   to C:\Program Files\Common Files\VST3. Preserve all Contents subfolders;
   do not install only the inner binary. Administrator permission may be needed.
4. Restart/rescan VST3 plugins and load it on a MIDI/instrument track.
5. Start at a moderate Master level; play MIDI or the interactive mouse keyboard.
Keep only one version in scanned directories; its VST3 identity is unchanged.
To uninstall, close the DAW, remove that bundle and rescan.
CONTROLS AND ARCHETYPES
E1 DEVOTION / Shimmer; E2 DUALISM / Balance; E3 PATIENCE / Attack;
E4 REMEMBRANCE / Release; E5 SANCTUARY / Reverb; E6 SOLITUDE / Synced Delay;
E7 HARMONY / Recurring Bells; E8 MADNESS / Controlled Instability.
F1 HELL / Bass; F2 EARTH / Mid; F3 HEAVEN / Treble; F4 MASTER / Volume.
Pitch Bend / DOUBT: global +/-100 cents; CC1 / FAITH: visual only.
P1-P4 choose oscillator I waveforms; P5-P8 choose oscillator II waveforms.
MiniLab User/DAW CC maps are documented in README.md. The DAW must forward MIDI.
The pad-control octave is reserved; mouse keys and octave navigation avoid it.

THE HERMIT: Contemplation. THE MARTYR: Suffering. THE INQUISITOR: Judgment.
THE HIEROPHANT: Transcendence (HELL -1.0 dB). THE FOOL (CUSTOM): your saved sound.
SAVE writes 13 sound settings to %APPDATA%\YURODSTVO\custom-preset-v1.json;
THE FOOL recalls that file when clicked. Master, expression and polyphony are
excluded. PRAY FOR INSPIRATION generates new combinations of those 13 settings.
Harmony's approved +2 dB bell calibration is already included. No extra gain
should be applied for installation. SILENCE clears voices and effect tails.

LICENCE AND SOURCE
GNU AGPL v3.0 only (AGPL-3.0-only). JUCE 9.0.4 uses its AGPL option.
Keep LICENSES and provide complete Corresponding Source when redistributing
binaries. Exact source, including JUCE, is provided in the same release:
https://github.com/Shilemir/yurodstvo-holy-fools-pad-engine/releases/tag/v1.0.0
See SOURCE-CODE.txt and YURODSTVO-v1.0.0-Corresponding-Source.zip.
Third-party notices and OFL font remain intact. Only the owner's applicable
rights in the AI PNG are dedicated under CC0-1.0; see ILLUSTRATION-NOTICE.txt.
The KJV inscription retains its attribution and separate regional-rights notice.

REQUIREMENTS AND LIMITS
Windows x64; normal Windows system libraries and a working audio output.
Official packages use the statically linked MSVC runtime. No separate font,
image, JUCE, compiler, runtime installer or driver is needed by end users.
Unsigned binaries. Tested on the development Windows machine with offline checks;
no new Ableton, physical MiniLab, listening, other-PC or all-DAW test is claimed.
Without valid host BPM, synced delay keeps the last tempo, initially 120 BPM.
Independent bell/effect tails can outlast the keyboard's main-envelope glow.
Back up older projects: extended ranges/fader order and DSP differ from early 0.x.
