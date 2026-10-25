# Infinati Kick

A simple kick drum synth (VST3, AU and Standalone) built with JUCE 8.

Play any MIDI note to trigger a kick (velocity sets the hit level). In the standalone app, use the TRIGGER button.

Parameters: Start Pitch, End Pitch, Pitch Decay, Amp Decay, Click, Drive, Level.

## Download

Every push builds the plugin on GitHub Actions (Actions tab > latest "Build" run > Artifacts):

- **InfinatiKick-macOS**: `Infinati Kick.vst3.zip`, `Infinati Kick.component.zip` (AU) and the standalone app. Universal (Apple Silicon + Intel), macOS 11+.
- **InfinatiKick-Windows**: the `.vst3` folder.

The macOS builds are ad-hoc signed, not notarized. After unzipping, clear the download quarantine or Live will refuse to load it:

    xattr -dr com.apple.quarantine "Infinati Kick.vst3" "Infinati Kick.component"

Then copy `Infinati Kick.vst3` to `~/Library/Audio/Plug-Ins/VST3/` and/or `Infinati Kick.component` to `~/Library/Audio/Plug-Ins/Components/`.
On Windows, copy the `.vst3` folder to `C:\Program Files\Common Files\VST3\`.

## Build (Mac)

Prerequisites: Xcode command line tools (`xcode-select --install`), CMake (`brew install cmake`).

    cmake -B build -G Xcode
    cmake --build build --config Release

The VST3 and AU are copied to `~/Library/Audio/Plug-Ins/` automatically.

## Build (Windows)

Prerequisites: Visual Studio 2022 (Desktop development with C++), CMake.

    cmake -B build
    cmake --build build --config Release

The VST3 is copied to `C:\Program Files\Common Files\VST3\` automatically
(run the terminal as Administrator, or configure with `-DINFINATI_COPY_PLUGIN=OFF` and copy the .vst3 folder from
`build\InfinatiKick_artefacts\Release\VST3\` yourself).

The first configure downloads JUCE from GitHub, so it takes a few minutes.

## Tests

    ctest --test-dir build -C Release --output-on-failure

`Tests/RenderTests.cpp` renders the processor offline and checks silence without MIDI, hit level and decay,
velocity response, click-free retriggering, stability at extreme settings and state save/restore.
CI also runs [pluginval](https://github.com/Tracktion/pluginval) at strictness 10 on every format and `auval` on the AU.

## Ableton Live

Preferences > Plug-Ins > turn on "Use VST3 Plug-In System Folders" (and "Use Audio Units" on Mac), then
press Rescan. Create a MIDI track, load "Infinati Kick" from Plug-Ins > Instruments.
