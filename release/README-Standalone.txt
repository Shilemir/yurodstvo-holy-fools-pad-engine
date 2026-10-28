YURODSTVO - Holy Fool's Synth for Arturia MiniLab3 v@VERSION@
Official stable release | Windows x64 Standalone

START
1. Verify YURODSTVO-v@VERSION@-Windows-x64-Standalone.zip with SHA256SUMS.txt.
2. Extract everything and run YURODSTVO - Holy Fool's Synth for Arturia MiniLab3.exe.
3. Select audio output/MIDI devices in Options / Audio-MIDI Settings as needed.
4. Start at a moderate Master level. No DAW or controller is needed for mouse keys.
Keep the LICENSES folder beside the application. No installer is run.
To uninstall, close the app and remove its extracted folder.
NEW INSTANCE
The Hermit is the default sound; saved host state takes precedence.

COMPUTER KEYBOARD (STANDALONE ONLY)
With the editor active, A W S E D F T G Y H U J K play chromatically from C to C.
The initial C is MIDI 48 (C2). The on-screen octave arrows move the range.
Press to play; release to begin the note's Release. Chords use the same synth
voices and glow as MIDI. Repeated key events do not retrigger a held note.
Focus loss, octave change and Silence cancel computer notes; release held keys
before playing again. Text fields and Ctrl/Alt/Command shortcuts are excluded.
Simultaneous key count depends on keyboard hardware. No global hooks are used.

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
https://github.com/Shilemir/yurodstvo-holy-fools-pad-engine/releases/tag/v1.1.0
See SOURCE-CODE.txt and YURODSTVO-v1.1.0-Corresponding-Source.zip.
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
