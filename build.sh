# fixes, and cleanup
python3 fixes.py
rm -f sprites.h
rm -f tilemap.h
rm -f tileset.h
rm -f title_tilemap.h

mkdir -p build

make clean
# create the rom with the flag
make
mv main.gb build/main-with-flag.gb
# create the rom with the fake flag
make clean
RELEASE=-Drelease make
mv main.gb build/main.gb
