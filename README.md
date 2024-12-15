# Zoomer

[Boomer](https://github.com/tsoding/boomer)-like zoom application for zoomers (zoomers = people who use Wayland).

## Building and running

### Dependencies

Under Linux, you will need to have these installed:

- [`grim`](https://sr.ht/~emersion/grim/) (required for Wayland support)
- [`spectacle`](https://apps.kde.org/spectacle/) (required for Wayland support on Plasma)
- [`scrot`](https://github.com/resurrecting-open-source-projects/scrot) (required for X11 support)

### Quick Start

On an MSYS2 environment or Linux, run the following commands:

```console
$ ./build.sh
$ ./zoomer # or zoomer.exe on Windows
```

### Installing

Installing only works on Linux. Run:

```console
# ./build.sh install
```
Or:
```
# INSTALL_DIR=/path/to/custom/directory ./build.sh install
```

Uninstalling is the same process, but with the `uninstall` subcommand instead.

## Controls

| Control                                          | Action                        |
|--------------------------------------------------|-------------------------------|
| Q                                                | Quit the application.         |
| Left click + Mouse move                          | Move the camera.              |
| Scroll wheel                                     | Increase camera zoom.         |
| 0 (zero)                                         | Reset zoom.                   |
| F                                                | Toggle the flashlight.        |
| Shift + Scroll wheel (or Control + Scroll wheel) | Change the flashlight's size. |

