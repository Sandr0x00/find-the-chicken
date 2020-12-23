# Find the chicken

Find the chicken game source for [hxp CTF 2020](https://2020.ctf.link).

## Links

- [The task](https://2020.ctf.link/internal/challenge/7e09f315-2f7b-4f0a-bcaf-934cc298e263/)
- [The participants runs](https://sandr0.xyz/shared/chicken.html)
- [The scoreboard](https://chicken.hxp.io/)

## Prerequisites

- [GBDK-2020](https://github.com/Zal0/gbdk-2020)
- [Gameboy Tile Designer (GBTD)](http://www.devrs.com/gb/hmgd/gbtd.html)

## Building

```sh
# create map
cd map
make

# create rom
# install gbdk-2020 in this folder
cd ..
make
# or ./build.sh to create the public and private roms
```

## The reference run

![HXPs reference 7441 frames run](chicken.gif)

To create GIFs from your run, use the modified [TAS-Emulator](https://sandr0.xyz/shared/tas-emulator) in the docker container (debian:buster) given in the task.

`./tas-emulator -s 50000 playback replayfile -r gif:7`

## Font used

The font used in the game can be found at [fonts2u.com](https://de.fonts2u.com/modern-dos-8x8.schriftart)
