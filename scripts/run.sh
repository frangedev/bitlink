#!/usr/bin/env bash
# Run BitLink standalone app. Builds if needed. Run from repo root.
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
APP="$ROOT/build/BitLink_artefacts/Standalone/BitLink.app"

if [[ ! -d "$APP" ]]; then
    echo "App not built. Running build..."
    "$SCRIPT_DIR/build.sh"
fi

echo "==> Launching BitLink..."
if [[ "$(uname)" == Darwin ]]; then
    open "$APP"
else
    exec "$APP/Contents/MacOS/BitLink" 2>/dev/null || exec "$ROOT/build/BitLink_artefacts/Standalone/BitLink" 2>/dev/null || true
fi
