# ctrDoom2D
## What?
Doom 2D is an old Russian Doom fangame, originally made for DOS in 1996 by Prikol Software.

It was ported to SDL in 2011 by Rembo. This is a port of that SDL port to Nintendo 3DS/libctru.

## Building
You should have the latest devkitARM and libctru installed and in your PATH.

`make` builds `ctrd2d.elf` and `ctrd2d.3dsx`.

`make cia` builds `ctrd2d.cia`.

## Installing
If you have a way to install CIAs, build and install the CIA. If you only have the Homebrew Launcher, build the 3DSX and copy it to `sdmc:/3ds/`.

Then, copy the `gamedata/ctrd2d` folder to `sdmc:/3ds/`.

## Running
If you've installed the CIA, just launch the title; if you're using HBL, run it and Doom 2D should be in the menu.

Default controls:
- D-Pad: movement/aiming
- A: jump
- B: shoot
- Y: use
- START: menu
- L/R: change weapon; Y/N in menu prompts

If it crashes for whatever reason, it should freeze for a few seconds, allowing you to read the console.

## TODO
- [ ] Music
- [ ] Some way to specify command line args or a config menu
- [ ] Faster graphics (should probably write a hardware renderer or something)
- [ ] NDSP sound (apparently CSND is deprecated?)
- [ ] Better timing/FPS cap
- [ ] Strafe button
