cd game

# fixes, and cleanup
python3 fixes.py
rm -f sprites.h
rm -f tilemap.h
rm -f tileset.h
rm -f title_tilemap.h

make clean
# create the rom with the flag
make
cp main.gb main-with-flag.gb
# create the rom with the fake flag
make clean
RELEASE=-Drelease make

