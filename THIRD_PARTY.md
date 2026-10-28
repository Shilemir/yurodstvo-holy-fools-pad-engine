# Third-party components and artwork

The original application is **AGPL-3.0-only**; see LICENSE.txt and COPYRIGHT.md.
Dependencies retain their original licences, notices and copyright holders.
See [docs/LICENCE-AUDIT.md](docs/LICENCE-AUDIT.md) for review scope and evidence.

## JUCE 9.0.4: AGPLv3 option

This distribution elects **AGPL-3.0-only**, not the JUCE Starter/commercial EULA
path. Previous Starter statements are superseded for this distribution. No
revenue or noncommercial restriction is added. A commercial JUCE licence may
be required for a different distribution that does not comply with JUCE's AGPL.
JUCE is copyright Raw Material Software Limited and its other contributors.
Exact upstream revision: `3fb5cbc0340758596dac39f7c8de47d562b4896e`, tag `9.0.4`.

- [JUCE licence](https://github.com/juce-framework/JUCE/blob/9.0.4/LICENSE.md)
- [Module notice/version](https://github.com/juce-framework/JUCE/blob/9.0.4/modules/juce_core/system/juce_StandardHeader.h)
- [Dependency inventory](https://github.com/juce-framework/JUCE/blob/9.0.4/JUCE.spdx.json)

JUCE is a pinned external dependency, not vendored in Git. The complete
Corresponding-Source release asset includes its unmodified source and vendored
dependencies with their original notices. Optional modules are not all linked.

## VST3 SDK and platform scope

JUCE 9.0.4 includes **VST3 SDK 3.8.0 under MIT**. Root, base, pluginterfaces and
public.sdk notices are preserved; binary packages include VST3-SDK-MIT.txt.
This is the included MIT SDK, not an older VST3 licence.
[Exact notice](https://github.com/juce-framework/JUCE/blob/9.0.4/modules/juce_audio_processors_headless/format_types/VST3_SDK/LICENSE.txt).
PreSonus extension headers declare themselves public domain; their original
statements/disclaimers accompany source and binary notices, without relabelling
these third-party headers as the owner's CC0 artwork.

Only Standalone and VST3 are built. ASIO and Windows MIDI Services are disabled
by default. AAX, AU, LV2, Android/Oboe, webview and external codec SDKs are outside
these Windows targets. Audit them separately if enabling them. Optional ASIO/AAX
source offers GPLv3 alternatives; the full source archive preserves those terms
without claiming a proprietary SDK redistribution grant.

MSVC's runtime is statically linked. The compiler, Windows SDK and ordinary
Windows libraries are build tools/System Libraries, not project code relicensed
under AGPL. No Microsoft DLL bundle or driver is shipped. Their own terms apply.

## Included JUCE dependencies

These permissive licences allow inclusion in an AGPLv3 program with their notices
preserved; individual dependencies are not converted to AGPL or CC0. Versions
and paths were checked against JUCE 9.0.4's SPDX inventory and source files.

| Component | Version | Licence / binary notice |
|---|---|---|
| zlib | 1.3.2 | Zlib; zlib.txt |
| HarfBuzz | 14.2.1 | MIT-Modern-Variant; HarfBuzz.txt |
| SheenBidi | 2.9.0 | Apache-2.0; SheenBidi-Apache-2.0.txt |
| LunaSVG / PlutoVG | 3.5.0 / 1.3.2 | MIT; LunaSVG.txt / PlutoVG.txt |
| libpng | 1.6.58 | libpng-2.0; libpng.txt |
| Independent JPEG Group | 10.0 | IJG; Independent-JPEG-Group.txt |
| libwebp | 1.6.0 | BSD-3-Clause and patent grant; libwebp.txt / libwebp-PATENTS.txt |
| FLAC / Ogg / Vorbis | 1.5.0 / 1.3.6 / 1.3.7 | BSD-3-Clause; FLAC.txt / Ogg.txt / Vorbis.txt |
| Opus / opusfile / libopusenc | 1.6.1 / 0.12-59-g6dfd29e / 0.3 | BSD family; Opus.txt / Opusfile.txt / Opusenc.txt and upstream patent/IPR notices |

This software is based in part on the work of the Independent JPEG Group.
SheenBidi copyright (C) 2014-2025 Muhammad Tayyab Akram; Apache License 2.0.
JUCE-inventory.spdx.json is the complete upstream inventory, not a claim that
all its optional components are linked. Source headers retain detailed notices;
Opus COPYRIGHT/COPYING, patent grants and IPR statements are preserved.

## Ruslan Display: SIL OFL 1.1

Unmodified RuslanDisplay-Regular.ttf is embedded in both builds. Copyright 2011
Oleg Snarsky, Denis Masharov and Vladimir Rabdu. Reserved Font Names: Ruslan and
Ruslan Display. The font remains **OFL-1.1**, not AGPL or CC0. Embedding/bundling
is permitted with its full notice: [assets/fonts/OFL.txt](assets/fonts/OFL.txt).
See the [OFL FAQ](https://openfontlicense.org/ofl-faq/). Font modifications must
respect reserved names and the other OFL conditions.

## Illustration and screenshot

Only `assets/yurodstvo_icon.png` is covered by the owner's **CC0-1.0** dedication,
to the extent the owner holds applicable rights. It was generated for the owner
using AI, inspired by historical Orthodox iconography. This does not claim
exclusive copyright, universal public-domain status or clearance of all possible
third-party rights. See assets/ILLUSTRATION-NOTICE.txt and LICENSES/CC0-1.0.txt.
Fonts, libraries and the full UI screenshot are excluded from that dedication.
Historical external reference images are excluded from public distribution.

## Redistribution and host boundary

Provide Corresponding Source for the covered program and keep notices when
redistributing binaries; modified versions have sections 5 and 13 obligations
as applicable. There is no additional noncommercial restriction.
This review covers distribution of the instrument and its included components.
It does not decide whether a particular proprietary host/plugin combination is
a single derivative work, or grant a linking exception for JUCE. Do not infer a
blanket proprietary-host redistribution exception from the MIT VST3 interfaces.
## Scripture inscription

The editor quotes 1 Corinthians 3:18 in the King James Version (KJV), with that
translation and reference displayed. The quotation is not claimed as original
project text or included in the owner's CC0 artwork dedication. This notice does
not grant rights in a Bible translation where separate rights apply. Cambridge
University Press administers Crown rights in the Authorized Version in the UK;
its published permissions terms should be consulted for uses subject to those
rights. No Cambridge permission or endorsement is asserted by this project.
See https://www.cambridge.org/bibles/about/rights-and-permissions and
https://assets.cambridge.org/97805211/63347/copyright/9780521163347_copyright_info.pdf.
