# Publication handoff — 0.8.0

The owner authorizes a public current-source snapshot and a public Pre-release
with both Windows ZIPs. The complete original development history must stay local.
The owner chose source availability without an open-source licence and explicitly
authorized the official AI illustration for source and binary distribution.

Target repository name: `yurodstvo-holy-fools-pad-engine`.
Inspect the authenticated account and existing repositories before creating one.
Reuse the intended existing repository; never overwrite unrelated work or force-push.
If an existing repository is private, obtain the owner's explicit confirmation
before making it public: source, history and assets become publicly accessible.
A new public repository is authorized if no intended existing repository exists.

The public snapshot must exclude `assets/reference`, original `.git`, build trees,
artifacts, local settings, temporary attachments and secrets. Publish only its
reviewed main branch and its own v0.8.0 tag. Never push all tags from the private
repository: those tags reach the historical external images.

Title/body: RELEASE-NOTES.md. Application version and public snapshot tag: v0.8.0.
Mark the release Pre-release. Upload both Windows-VST3 and Windows-Standalone ZIPs
and their SHA-256 sidecars. Verify the remote tag, asset hashes and visibility.
Only report repository/release URLs after checking them on GitHub.

GitHub installation was confirmed by plugin discovery. At handoff time, this
conversation did not expose GitHub repository/release tools or an installed gh CLI.
This is a tool-availability limitation, not a request to reconnect the account.
If needed, start a new conversation in the same project with GitHub enabled and
continue from the local reviewed public snapshot and publication manifest.