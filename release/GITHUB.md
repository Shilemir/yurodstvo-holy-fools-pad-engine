# Publishing the AGPL distribution

Repository: https://github.com/Shilemir/yurodstvo-holy-fools-pad-engine
Application version: 0.8.0. Open-source revision tag: v0.8.0-agpl.
Keep the historical v0.8.0 tag unchanged; do not force-push or rewrite history.
Only push the reviewed public repository's main branch and explicitly named tag.
Never push the original private repository's master or historical tags.

Run the clean distribution build and technical checks, then package the complete
Corresponding Source with release/package-source.ps1. Publish both Windows ZIPs,
the Corresponding-Source ZIP and their three .sha256 sidecars with RELEASE-NOTES.md
as a Pre-release. Keep the complete source archive available with the binaries.
Update any earlier release to point recipients to the new licensing revision.
Verify anonymous downloads and hashes, tag commit and GitHub's licence detection.
The Git repository excludes JUCE (CMake fetches it); the source release includes
its exact unmodified source and original notices for offline builds.

The original application and project-authored scripts/docs are AGPL-3.0-only.
JUCE uses its AGPLv3 option. Third-party terms remain intact. Only the official
PNG has the owner's CC0 dedication, to the extent of rights they can grant.
See COPYRIGHT.md, THIRD_PARTY.md and docs/LICENCE-AUDIT.md for scope and evidence.