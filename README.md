Pacman
======

![in-game screenshot](https://libregamewiki.org/images/1/18/Pacman.png "in-game screenshot")

This is a clone of the original pacman_sdl by Namco, as I remember, that I played for the first time on an Atari 130 XL in the early 90s.

Also, Paul Neave's pacman_sdl clone has inspired me greatly.

One of the main goals of this implementation is an SDL application with a very low CPU usage.


## Install hint ##

You have to compile the Linux version on your own. For this, you'll need
* libsdl2
* sdl2-image
* sdl2-ttf
* and sdl2-mixer.

(make sure to take the devel packages) 
Then, download and extract the zip file or clone the pacman_sdl repository.
Inside the pacman_sdl directory, run
```
./configure
make
make install
```
For more detailed instructions, you may also have a look at the [INSTALL](https://github.com/ebuc99/pacman_sdl/blob/master/INSTALL) file.

After a successful installation, you should be able to start the game via command line: `pacman_sdl`

## GamerCard (Debian 12 ARM64 / Wayland) ##

Target device: Grant Sinclair GamerCard (Raspberry Pi Zero 2W, 720×720 IPS, Labwc/Wayland).

### Runtime packages (Debian 12)

```
sudo apt install libsdl2-2.0-0 libsdl2-image-2.0-0 libsdl2-ttf-2.0-0 libsdl2-mixer-2.0-0
```

### Build packages (cross-compile VM: Debian 12 aarch64)

```
sudo apt install build-essential autoconf automake pkg-config \
  libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```

Then:

```
./autogen.sh   # if configure is missing
./configure
make
sudo make install
```

### Launch from the Labwc launcher

```
SDL_VIDEODRIVER=wayland pacman_sdl -f
```

The installed `.desktop` entry already uses that command. Fullscreen scales the 640×480 game as large as possible while keeping aspect ratio (letterboxed on the 720×720 panel).

### Gamepad (Arduino Leonardo)

Controller DB is the Vinyl Linux curated list (`data/txt/gamecontrollerdb.txt`), including:

`03000000412300003680000001010000` (Arduino Leonardo / GamerCard).

| Action | Control |
|---|---|
| Move / menu navigate | D-pad |
| Confirm (menus) | A (also Start) |
| Cancel / exit | B or Back |
| Pause (in game) | Start |

### Pi Zero 2W notes

* Soft aspect-preserving scale (no integer-only limit) for 720×720
* Nearest-neighbor scale hint for crisp pixels / lower CPU
* Audio at 22050 Hz with a smaller mixer buffer

## Install Fedora ##

```
sudo dnf install make gcc-c++ SDL2 SDL2-devel SDL2_image SDL2_image-devel SDL2_mixer SDL2_mixer-devel SDL2_ttf SDL2_ttf-devel
```
Then, download and extract the zip file or clone the pacman_sdl repository.
Inside the pacman_sdl directory, run
```
./autogen.sh
./configure
make
make install
```

## License ##
Pacman is licensed under the terms of the GNU General Public License version 2 (or any later version).
