# Zoomer

[Boomer](https://github.com/tsoding/boomer)-like zoom application for zoomers (zoomers = people who use Wayland).

## Building and running

### Dependencies

- [`raylib`](https://github.com/raysan5/raylib/)
- X11
- Xcursor
- [`grim`](https://sr.ht/~emersion/grim/) (required for Wayland support)
- [`scrot`](https://github.com/resurrecting-open-source-projects/scrot) (required for X11 support)

### Quick Start

```console
$ make
$ ./main
```

### Installing

```console
$ sudo make install
```
Or:
```
$ sudo make install INSTALL_DIR=/path/to/custom/directory
```

## Controls

| Control                                          | Action                  |
|--------------------------------------------------|-------------------------|
| Q                                                | Quit the application.   |
| Left click + Mouse move                          | Move the image.         |
| Scroll wheel                                     | Resize the image.       |
| F                                                | Toggle flashlight.      |
| Shift + Scroll wheel (or Control + Scroll wheel) | Change flashlight size. |
