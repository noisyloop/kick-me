# Infinati Kick

A simple kick drum synth (VST3, AU and Standalone) built with JUCE 8.

Play any MIDI note to trigger a kick (velocity sets the hit level). In the standalone app, use the TRIGGER button.

Parameters: Start Pitch, End Pitch, Pitch Decay, Amp Decay, Click, Drive, Level.

## Download

Every push builds the plugin on GitHub Actions (Actions tab > latest "Build" run > Artifacts):

- **InfinatiKick-macOS**: `InfinatiKick-macOS.dmg` containing VST3, AU and standalone app, plus `Install-InfinatiKick.command`. Universal (Apple Silicon + Intel), macOS 12+.
- **InfinatiKick-Windows**: the `.vst3` folder.

Extract the Actions artifact ZIP, open the DMG, quit your DAW and double-click
`Install-InfinatiKick.command`. Press Return to install to your user account:
VST3 in `~/Library/Audio/Plug-Ins/VST3/`, AU in
`~/Library/Audio/Plug-Ins/Components/`, and app in `~/Applications/`.
The installer clears quarantine on these trusted development bundles.
The builds are ad-hoc signed and are not notarized.

If macOS blocks the installer itself, copy the bundles from the DMG to those folders
and run this in Terminal:

    xattr -dr com.apple.quarantine "$HOME/Library/Audio/Plug-Ins/VST3/Infinati Kick.vst3" "$HOME/Library/Audio/Plug-Ins/Components/Infinati Kick.component" "$HOME/Applications/Infinati Kick.app"

On Windows, copy the `.vst3` folder to `C:\Program Files\Common Files\VST3\`.

## Build (Mac)

Prerequisites: full Xcode with its command line tools selected, and CMake (`brew install cmake`).

    cmake -S . -B build -G Xcode -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
    cmake --build build --config Release

Build products are in `build/InfinatiKick_artefacts/Release/`.
Automatic installation is disabled by default; opt in with `-DINFINATI_COPY_PLUGIN=ON`.

## Build (Windows)

Prerequisites: Visual Studio 2022 (Desktop development with C++), CMake.

    cmake -B build
    cmake --build build --config Release

Copy the `.vst3` folder from `build\InfinatiKick_artefacts\Release\VST3\` to
`C:\Program Files\Common Files\VST3\`.

The first configure downloads JUCE from GitHub, so it takes a few minutes.

## Tests

    ctest --test-dir build -C Release --output-on-failure

`Tests/RenderTests.cpp` renders the processor offline and checks silence without MIDI, hit level and decay,
velocity response, click-free retriggering, stability at extreme settings and state save/restore.
CI also runs [pluginval](https://github.com/Tracktion/pluginval) at strictness 10 on every format and `auval` on the AU.

## Ableton Live

Preferences > Plug-Ins > turn on "Use VST3 Plug-In System Folders" (and "Use Audio Units" on Mac), then
press Rescan. Create a MIDI track, load "Infinati Kick" from Plug-Ins > Instruments.
