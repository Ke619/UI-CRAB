# Headcrab Updater

One-click launcher for SteamOS/SteamDeck. Opens a GUI window, click **Update**, and it fetches + runs the latest Headcrab patch from GitHub.

**No dependencies needed** — uses `yad` which is pre-installed on SteamOS.

## Files

```
headcrab-updater/
├── headcrab.png                          ← App logo
├── build.sh                              ← Build the AppImage
└── AppDir/
    ├── AppRun
    ├── headcrab-updater.desktop
    ├── headcrab.png
    └── usr/bin/
        ├── headcrab_updater.sh           ← Main launcher script
        └── headcrab.png
```

## Build the AppImage (run on SteamDeck)

```bash
chmod +x build.sh
./build.sh
```

Produces `HeadcrabUpdater-x86_64.AppImage`.

## Install on SteamDeck

1. Copy `HeadcrabUpdater-x86_64.AppImage` to `~/Desktop`
2. `chmod +x HeadcrabUpdater-x86_64.AppImage`
3. Double-click it, or add as a Non-Steam Game in Steam

## Test without building

```bash
chmod +x AppDir/usr/bin/headcrab_updater.sh
./AppDir/usr/bin/headcrab_updater.sh
```
