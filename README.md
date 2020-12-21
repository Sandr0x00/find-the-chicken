# Find the chicken

Find the chicken game source for [hxp CTF 2020](https://2020.ctf.link).

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

## Font used

The font used in the game can be found at [fonts2u.com](https://de.fonts2u.com/modern-dos-8x8.schriftart)
