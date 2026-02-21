#!/usr/bin/env bash
# Build BitLink (Release). Run from repo root.
set -e
cd "$(dirname "$0")/.."

echo "==> Checking JUCE submodule..."
if [[ ! -f JUCE/CMakeLists.txt ]]; then
    echo "JUCE not found. Run: git submodule update --init --recursive"
    exit 1
fi

echo "==> Configuring..."
cmake -B build -DCMAKE_BUILD_TYPE=Release

echo "==> Building..."
cmake --build build --config Release

echo "==> Done. Standalone: build/BitLink_artefacts/Standalone/BitLink.app"
echo "    AU:  build/BitLink_artefacts/AU/BitLink.component"
echo "    VST3: build/BitLink_artefacts/VST3/BitLink.vst3"
