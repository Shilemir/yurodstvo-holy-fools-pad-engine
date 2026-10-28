# Publishing YURODSTVO 0.10.1

Repository: https://github.com/Shilemir/yurodstvo-holy-fools-pad-engine
Application version and tag: 0.10.1 / v0.10.1. Preserve older tags and releases.
Only push the reviewed public repository's main and explicitly named version tag.
Never push the original private repository's master or all historical tags.
Do not force-push, rewrite history, delete old releases or change visibility.

Commit reviewed current files before the clean Release x64 build. Both formats
must come from that same commit. Run build-distribution.ps1 and package-source.ps1;
create a draft Pre-release with RELEASE-NOTES.md and recorded validation results.
Upload both binary ZIPs, complete Corresponding Source and three SHA-256 sidecars.
Check uploaded digests, tag commit and notes, then publish and verify anonymous
downloads. Link directly to /releases/tag/v0.10.1; /releases/latest may skip prereleases.

Keep AGPL-3.0-only, JUCE's AGPL option, OFL font and original third-party notices.
Only the official PNG has the owner's limited CC0 dedication. The complete
source archive includes the exact official JUCE checkout for offline builds;
JUCE is not vendored in the Git repository.