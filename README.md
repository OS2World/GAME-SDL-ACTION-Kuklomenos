# Kuklomenos

Kuklomenos is an abstract arcade shooter in which you defend triangular Nodes
from encroaching Blobs of Evil using three colours of shot and Capture Pods.

Original game by Martin Bays (GPLv3, 2008–2009).  
This repository contains the **ArcaOS / OS/2 port** built with GCC 9.2 and SDL2.

For gameplay details and command-line options see [README](README).  
For OS/2-specific build and installation instructions see [kuklomenos_os2.txt](/doc/kuklomenos_os2.txt).

![Kuklomenos ScreenShot](/doc/Kuklomenos.png)

---

## ArcaOS / OS/2 Port Status

| Feature       | Status                         |
|---------------|--------------------------------|
| Windowed mode | Working                        |
| Fullscreen    | Not supported                  |
| Sound         | Working (requires SDL2_mixer)  |
| Keyboard      | Working                        |
| High scores   | Local only (no network upload) |

---

## Requirements

- ArcaOS 5.x (or OS/2 Warp 4 with kLIBC)
- SDL2, SDL2\_mixer DLLs on the `LIBPATH` (install via `yum`)

---

## Building

Requires GCC 9.2 (kLIBC), SDL2 dev packages, and GNU make.  
Run from the source root on ArcaOS:

```
compile-kuklomenos.cmd
```

Output: `kuklomenos.exe`

Source layout:

```
src\            game source code (.cc / .h)
compat\SDL\     SDL1 -> SDL2 compatibility shim headers
fonts\          bitmap font data (runtime)
sounds\         OGG audio files (runtime)
```

---

## Running

Launch from the game directory so that `fonts\` and `sounds\` are reachable:

```
cd C:\Games\Kuklomenos
kuklomenos.exe
```

Settings are saved to `kuklomenosrc.txt` in the game directory (or `~\.kuklomenosrc`).

---

## Controls

| Key               | Action                      |
|-------------------|-----------------------------|
| Left / Right      | Turn                        |
| Up                | Zoom out / de-aim           |
| Down              | De-aim                      |
| 1, 2, 3           | Fire (green / yellow / red) |
| 4                 | Launch Capture Pod          |
| P                 | Pause                       |
| Q                 | Quit                        |
| Escape            | Menu                        |
| S                 | Toggle sound                |
| Z                 | Toggle zoom                 |
| R                 | Toggle rotation             |

---

## License

GNU General Public License v3 — see [COPYING](COPYING).

Copyright (C) 2008–2009 Martin Bays

## Links

- https://github.com/OS2World/GAME-SDL-ACTION-Kuklomenos
