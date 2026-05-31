```text
╔════════════════════════════════════════════════════════╗
║               _                                        ║
║   __      __ (_)  _ __    _ __ ___     __ _   _ __     ║
║   \ \ /\ / / | | | '_ \  | '_ ` _ \   / _` | | '_ \    ║
║    \ V  V /  | | | | | | | | | | | | | (_| | | | | |   ║
║     \_/\_/   |_| |_| |_| |_| |_| |_|  \__,_| |_| |_|   ║
╚════════════════════════════════════════════════════════╝
```

Pure C macOS window manager utility.
Maps keyboard shortcut/chord -> open/focus application.

- Headless LaunchAgent: no menu bar icon, no preferences window, no Dock presence.
- Starts at login.

## Table of Contents

- [Overview](#overview)
- [Getting Started](#getting-started)
- [Install](#install)
- [Uninstall](#uninstall)
- [References](#references)

## Overview

1. On startup, check for accessibility permissions.
    - If not trusted, wait/retry, don't exit.
2. Load config (chord to bundle ID).
3. Create tap, run loop.
4. On keydown, ignoring autorepeat, map chord to bundle ID.
5. If match for chord and app for bundle ID exists, open/focus the application.
    - Swallow the chord press to prevent any possible existing default behaviour for chord.
    - If app not running then launch (macOS will focus automatically), otherwise focus app.

## Getting Started

1. Install Xcode Command Line Tools if clang is not already available.

```bash
./init.sh
```

2. Use make to compile the window manager.

```bash
make
```

3. Run in foreground for testing.

```bash
./winman
./winman --help
```

## Install

```bash
# 1. Copy binary to a stable location.
mkdir -p ~/bin
cp winman ~/bin/

# 2. Create plist ($USER/$HOME expanded at write time).
cat > com.$USER.winman.plist <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>Label</key>
    <string>com.$USER.winman</string>

    <key>ProgramArguments</key>
    <array>
        <string>$HOME/bin/winman</string>
    </array>

    <key>RunAtLoad</key>
    <true/>

    <key>KeepAlive</key>
    <true/>

    <key>StandardOutPath</key>
    <string>/tmp/winman.log</string>

    <key>StandardErrorPath</key>
    <string>/tmp/winman.err</string>
</dict>
</plist>
EOF

# 3. Copy plist to LaunchAgents.
cp com.$USER.winman.plist ~/Library/LaunchAgents/

# 4. Load plist using launchctl.
launchctl load ~/Library/LaunchAgents/com.$USER.winman.plist
```

## Uninstall

```bash
launchctl unload ~/Library/LaunchAgents/com.$USER.winman.plist
rm ~/Library/LaunchAgents/com.$USER.winman.plist
rm ~/bin/winman
defaults delete com.$USER.winman
```

## References

- []()
- []()
- []()
