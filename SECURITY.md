# Security review

Kick Me is a JUCE instrument plugin (VST3/AU/Standalone). For a plugin the realistic risks
are a crash or bad audio inside the user's DAW (from saved state, host automation, or the audio
thread), and the build/release pipeline that ships the DMG people install. Reviewed at 0.3.0.

## Summary

| Area | State |
|---|---|
| Audio thread (`processBlock`) | Solid: no allocation or locks, denormals off, bounded by `tanh` |
| UI ↔ audio sharing | Solid: atomics only (`uiTrigger`, `outputPeak`), 30 Hz timer |
| Programs / presets | Solid: indices bounds-checked |
| **Release pipeline** | **Fixed: feature branches were publishing the release DMG** |
| **CI token and third-party binary** | **Fixed: least privilege, pinned and checksummed pluginval** |
| **NaN from saved state** | **Fixed: repaired on load, never sent to the host** |
| Unsigned DMG + quarantine removal | Inherent until the build is notarized (see below) |

## Findings and fixes

### Fixed: any branch push published the release DMG (`.github/workflows/build.yml`)

The **Publish release** step ran on every `push` event, on every branch, and uploaded the DMG with
`--clobber` to the `v<version>` release the README links as "Download". Pushes to feature
branches (`claude/…`, `codex/…`) did run it; for example, run 40182858953 on
`claude/adoring-brown-3j5usw` shows **Publish release: success**. So an unreviewed branch build
could replace the download, and the install instructions then have users strip Gatekeeper's
quarantine from it. *Fix:* publish only when `github.ref == 'refs/heads/main'`.

GitHub Actions is currently switched off on this repo (deliberately), so this can't happen right
now; the fix matters as soon as Actions is turned back on.

## Verification

These changes were reviewed by reading the code and have not been compiled or tested: Actions is off
and the build wasn't run locally. The new render test (`NaN in saved state…`) and the pinned
pluginval checksums are exercised the next time CI or a local `ctest` runs.

### Fixed: write token exposed to an unpinned binary

The whole workflow had `contents: write`, `actions/checkout` left that token in `.git/config`, and
pluginval was downloaded from `releases/latest` with no checksum, then run before the DMG was built
and signed. A compromised or swapped pluginval could have read the token or tampered with
the bundles that get shipped. *Fix:* `contents: read` by default (write only on the macOS job),
`persist-credentials: false`, and pluginval pinned to v1.0.4 with a SHA-256 check on each OS.

### Fixed: NaN in saved state (`Source/PluginProcessor.cpp`)

Parameter values come back from project and preset files, which can be edited or shared. JUCE parses
`value="nan"` as NaN and `NormalisableRange` clamps with `jlimit`, which passes NaN through. A NaN
start pitch or level produced NaN audio, and NaN then **latched**: `lastOut` feeds the declick
crossfade, and `NaN * 0` is still NaN, so every later block stayed NaN until the plugin was
reloaded. NaN poisons whatever comes after it in the DAW (reverbs, busses). *Fix:*
`setStateInformation` resets any non-finite parameter to its default, `processBlock` renders
silence if a parameter is non-finite, and a per-sample guard resets the voice instead of emitting
NaN. Covered by a new render test that loads a state containing `nan`.

### Inherent: ad-hoc signed DMG and quarantine removal

The build is ad-hoc signed and not notarized, so the installer (and the README one-liner) remove
`com.apple.quarantine` for Live to load it. That's a reasonable trade-off for a free plugin, and now
only `main` builds reach the download, but it does mean macOS doesn't vouch for the binary.
Notarizing with a Developer ID would remove the need.

### Low: unpinned dependencies

JUCE is fetched by the `8.0.10` tag and the GitHub Actions by major tag (`@v4`). Tags can move;
pinning to commit SHAs would make builds fully reproducible.
