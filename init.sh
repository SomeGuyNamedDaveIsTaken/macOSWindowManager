#!/usr/bin/env bash

# chmod u+x init.sh

# Abort on error (-e), undefined variables (-u), or failed pipe stages (-o pipefail).
# Rule of thumb: set -e fire only when exit code get ignored.
set -euo pipefail

if err=$(command -v clang 2>&1); then
    echo "clang present: $(clang --version | head -n1)"
    exit 0
fi

if [[ -n "$err" ]]; then
    echo "Error checking for clang: $err" >&2
    exit 1
fi

echo "clang not found — installing Xcode Command Line Tools..."
xcode-select --install
echo "Finish the GUI installer, then re-run ./init.sh to verify."
