#!/usr/bin/env python3

def fix(file):
    # The generated files by GBTD can not be used, we have to fix them
    with open(f"{file}.c", "r") as f:
        lines = f.read().replace("\n\n\n", "\n\n").split("\n")

    new_content = []
    for line in lines:
        if line.startswith("unsigned char"):
            new_content.append("#include <types.h>")
            name = line.split(" ")[2]
            new_content.append(f"const UINT8 {name} =")
        else:
            new_content.append(line)

    with open(f"{file}.c", "w") as f:
        for line in new_content:
            f.write(line)
            f.write("\n")

fix("sprites")
fix("tileset")
fix("title_tilemap")