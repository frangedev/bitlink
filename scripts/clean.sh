#!/usr/bin/env bash
# Remove build directory. Run from repo root.
set -e
cd "$(dirname "$0")/.."
echo "==> Removing build/"
rm -rf build
echo "==> Done."
