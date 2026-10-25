# Infinati Kick

A simple kick drum synth (VST3 + Standalone) built with JUCE 8.

Play any MIDI note to trigger a kick. In the standalone app, use the TRIGGER button.

Parameters: Start Pitch, End Pitch, Pitch Decay, Amp Decay, Click, Drive, Level.

## Build (Mac)

Prerequisites: Xcode command line tools (`xcode-select --install`), CMake (`brew install cmake`).

    cd InfinatiKick
    cmake -B build -G Xcode
    cmake --build build --config Release

The VST3 is copied to `~/Library/Audio/Plug-Ins/VST3/` automatically.

## Build (Windows)

Prerequisites: Visual Studio 2022 (Desktop development with C++), CMake.

    cd InfinatiKick
    cmake -B build
    cmake --build build --config Release

The VST3 is copied to `C:\Program Files\Common Files\VST3\` automatically
(run the terminal as Administrator, or copy the .vst3 folder from
`build\InfinatiKick_artefacts\Release\VST3\` yourself).

## Ableton Live

Preferences > Plug-Ins > make sure "Use VST3 Plug-In System Folders" is on, then
press Rescan. Create a MIDI track, load "Infinati Kick" from Plug-Ins > Instruments.

The first configure downloads JUCE from GitHub, so it takes a few minutes.
