# Infinati Kick

A ridiculous synthwave kick drum synth (VST3, AU and Standalone) built with JUCE 8.

![Infinati Kick](docs/screenshot.png)

Play any MIDI note to trigger a kick (velocity sets the hit level). TRIGGER plays a hit, RANDOMIZE rolls a new kick and plays it.

Parameters: Start Pitch, End Pitch, Pitch Decay, Amp Decay, Click, Drive, Level.

## Download (macOS)

**[Download InfinatiKick.dmg](https://github.com/noisyloop/kick-me/releases/latest/download/InfinatiKick.dmg)** (Apple Silicon + Intel, macOS 11+)

Open the DMG, then paste this into Terminal. It copies the plugin to `~/Library/Audio/Plug-Ins/VST3/InfinatiKick.vst3` and clears the download quarantine in one step (the build is not notarized, so Live won't load it otherwise):

    V=~/Library/Audio/Plug-Ins/VST3; mkdir -p "$V" && rm -rf "$V/InfinatiKick.vst3" "$V/Infinati Kick.vst3" && ditto /Volumes/InfinatiKick/InfinatiKick.vst3 "$V/InfinatiKick.vst3" && xattr -cr "$V/InfinatiKick.vst3" && echo "Infinati Kick installed"

It also removes an older `Infinati Kick.vst3` if you installed one from a previous build. Then rescan in Live (see below).

## Build (Mac)

Prerequisites: Xcode command line tools (`xcode-select --install`), CMake (`brew install cmake`).

    cmake -B build -G Xcode
    cmake --build build --config Release

`InfinatiKick.vst3` and `InfinatiKick.component` are copied to `~/Library/Audio/Plug-Ins/` automatically.

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
velocity response, click-free retriggering, stability at extreme settings, state save/restore, value text and randomize.
CI also runs [pluginval](https://github.com/Tracktion/pluginval) at strictness 10 on every format and `auval` on the AU.

## Ableton Live

Preferences > Plug-Ins > turn on "Use VST3 Plug-In System Folders" (and "Use Audio Units" on Mac), then
press Rescan. Create a MIDI track, load "Infinati Kick" from Plug-Ins > Instruments.
