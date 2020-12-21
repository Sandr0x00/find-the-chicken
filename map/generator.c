#include <stdio.h>

#define UINT8 int
#define UINT16 int

#define TILEMAP_WIDTH 512
#define TILEMAP_HEIGHT 32

UINT8 TILEMAP[TILEMAP_WIDTH * TILEMAP_HEIGHT];
// UINT8 TILEMAP[(TILEMAP_WIDTH + 28) * TILEMAP_HEIGHT];
UINT8 HEIGHTMAP[TILEMAP_WIDTH];
#define to_tmap(x,y) ((y) * TILEMAP_WIDTH + (x))


static UINT8 PERLIN_SEED = 57;
const static UINT8 hash[] = {
	208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
	185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
	9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
	70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
	203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
	164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
	228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
	232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
	193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
	101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
	135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
	114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219
};

UINT8 noise2(int x, int y) {
	UINT8 tmp = hash[(y + PERLIN_SEED) % 256];
	return hash[(tmp + x) % 256];
}

int perlin2d(int x, int y, int depth) {
	int amp = 256;
	int fin = 0;
	int div = 0;

	if (depth == 1) {
		return noise2(x, y);
	}
	for(UINT8 i = 0; i < depth; i++) {
		// div += amp;
		fin += noise2(x, y) * amp;
		amp /= 2;
		x *= 2;
		y *= 2;
	}
	return fin % 256;
}

void put_tile(UINT16 x, UINT16 y, UINT8 tile) {
	TILEMAP[y * TILEMAP_WIDTH + x] = tile;
}

UINT8 old_height = 2;

void create_tiles(UINT16 x, UINT8 y) {
	UINT8 perlin = perlin2d(x, y, 4);
	UINT8 tile = 83;
	if (perlin > 0xc0) {
		old_height += 2;
		tile = 80;
	} else if (perlin > 0xa0) {
		old_height += 1;
		tile = 80;
	} else if (perlin < 0x10) {
		old_height -= 4;
		tile = 85;
	} else if (perlin < 0x20) {
		old_height -= 3;
		tile = 85;
	} else if (perlin < 0x30) {
		old_height -= 2;
		tile = 85;
	} else if (perlin < 0x40) {
		old_height -= 1;
		tile = 85;
	}
	if (old_height < 2) {
		old_height = 2;
	} else if (old_height > 25) {
		old_height = 25;
	}
	UINT8 height = TILEMAP_HEIGHT - old_height;
	for (UINT16 y = 0; y < TILEMAP_HEIGHT; y++) {
		if (y == height) {
			put_tile(x, y, tile);
			HEIGHTMAP[x] = y;
		} else if (y > height) {
			put_tile(x, y, (noise2(x, y) % 6) + 80);
		}
	}
	if (tile == 85) {
		if (TILEMAP[to_tmap(x - 1, height - 1)] != 0) {
			TILEMAP[(height - 1) * TILEMAP_WIDTH + x] = tile;
			HEIGHTMAP[x] = height - 1;
		}
		if (TILEMAP[to_tmap(x - 1, height - 2)] != 0) {
			TILEMAP[(height - 2) * TILEMAP_WIDTH + x] = tile;
			HEIGHTMAP[x] = height - 2;
		}
		if (TILEMAP[to_tmap(x - 1, height - 3)] != 0) {
			TILEMAP[(height - 3) * TILEMAP_WIDTH + x] = tile;
			HEIGHTMAP[x] = height - 3;
		}
		if (TILEMAP[to_tmap(x - 1, height - 4)] != 0) {
			TILEMAP[(height - 4) * TILEMAP_WIDTH + x] = tile;
			HEIGHTMAP[x] = height - 4;
		}
	}
}

void create_water(UINT16 x, UINT8 width) {
	UINT8 left = HEIGHTMAP[x - 1];
	UINT8 middle = 0;
	for (UINT8 t = 0; t < width; t++) {
		if (middle == 0) {
			middle = HEIGHTMAP[x + t];
		} else {
			if (middle != HEIGHTMAP[x + t]) {
				return;
			}
		}
	}
	UINT8 right = HEIGHTMAP[x + width];
	if (left < middle && right < middle) {
		for (UINT8 t = 0; t < width; t++) {
			if ((t & 1) == 0) {
				put_tile(x + t, middle, 0x60);
			} else {
				put_tile(x + t, middle, 0x61);
			}
		}
	}
}

void map_gen(UINT8 seed) {

	PERLIN_SEED = seed;
	UINT16 x;
	UINT16 y = 0;
	for (x = 0; x < TILEMAP_WIDTH - 30; x++) {
		if (x % 128 == 0) {
			y++;
		}
		create_tiles(x, y);
	}

	// arena at the end
	for (y = TILEMAP_HEIGHT - 7; y < TILEMAP_HEIGHT - 1; y++) {
		put_tile(482, y, 0x55);
	}
	for (x = TILEMAP_WIDTH - 30; x < TILEMAP_WIDTH; x++) {
		put_tile(x, TILEMAP_HEIGHT - 1, 0x50);
		HEIGHTMAP[x] = TILEMAP_HEIGHT - 1;
	}


	UINT16 last_height = 0;
	for (x = 0; x < TILEMAP_WIDTH; x++) {
		if (HEIGHTMAP[x] < last_height) {
			for (UINT8 i = HEIGHTMAP[x]; i < last_height; i++) {
				TILEMAP[(i + 1) * TILEMAP_WIDTH + x] = 0x50;
			}
		}
		last_height = HEIGHTMAP[x];
	}

	// walls
	for (y = 0; y < TILEMAP_HEIGHT; y++) {
		for (x = 0; x < 13; x++) {
			put_tile(x, y, 0x55);
			HEIGHTMAP[x] = 0;
		}
		// end
		put_tile(TILEMAP_WIDTH - 1, y, 0);
		// wall over tunnel
		put_tile(183, y, 0x50);
		put_tile(184, y, 0x55);
		// 2nd wall
		put_tile(452, y, 0x50);
		put_tile(453, y, 0x55);
	}

	put_tile(78, TILEMAP_HEIGHT - 4, 0x59);
	put_tile(78, TILEMAP_HEIGHT - 5, 0x59);
	put_tile(78, TILEMAP_HEIGHT - 6, 0x59);
	put_tile(78, TILEMAP_HEIGHT - 7, 0x59);

	// tunnel
	put_tile(168, TILEMAP_HEIGHT - 4, 0x58);
	put_tile(168, TILEMAP_HEIGHT - 5, 0x58);
	for (x = 169; x < 199; x++) {
		put_tile(x, TILEMAP_HEIGHT - 4, 4);
		put_tile(x, TILEMAP_HEIGHT - 5, 4);
	}
	put_tile(199, TILEMAP_HEIGHT - 4, 0x58);
	put_tile(199, TILEMAP_HEIGHT - 5, 0x58);

	// create pits / water
	for (x = 1; x < TILEMAP_WIDTH - 2; x++) {
		create_water(x, 5);
		create_water(x, 4);
		create_water(x, 3);
		create_water(x, 2);
		create_water(x, 1);
	}

	put_tile(64, TILEMAP_HEIGHT - 4, 0);
	put_tile(65, TILEMAP_HEIGHT - 4, 0);
	put_tile(66, TILEMAP_HEIGHT - 4, 0);
	// create invisible crossing / crossing with death
	for (x = 56; x < 75; x++) {
		if ((x & 1) == 0) {
			put_tile(x, TILEMAP_HEIGHT - 3, 0x60);
		} else {
			put_tile(x, TILEMAP_HEIGHT - 3, 0x61);
		}
	}

	// steps
	put_tile(49, TILEMAP_HEIGHT - 10, 0x5e);
	put_tile(50, TILEMAP_HEIGHT - 10, 0x5e);
	put_tile(54, TILEMAP_HEIGHT - 13, 0x5e);
	put_tile(55, TILEMAP_HEIGHT - 13, 0x5e);
	put_tile(60, TILEMAP_HEIGHT - 13, 0x5e);
	put_tile(61, TILEMAP_HEIGHT - 13, 0x5e);
	put_tile(67, TILEMAP_HEIGHT - 11, 0x5e);
	put_tile(68, TILEMAP_HEIGHT - 11, 0x5e);

	// second flag hidden
	put_tile(251, TILEMAP_HEIGHT - 7, 0);
	put_tile(251, TILEMAP_HEIGHT - 8, 0);
	put_tile(252, TILEMAP_HEIGHT - 7, 0);
	put_tile(252, TILEMAP_HEIGHT - 8, 0);
	HEIGHTMAP[251] = TILEMAP_HEIGHT - 9;
	HEIGHTMAP[252] = TILEMAP_HEIGHT - 9;

	// info sign 1
	put_tile(20, TILEMAP_HEIGHT - 6, 5);
	put_tile(20, TILEMAP_HEIGHT - 5, 7);

	put_tile(25, TILEMAP_HEIGHT - 6, 5);
	put_tile(25, TILEMAP_HEIGHT - 5, 7);

	// breakable wall
	put_tile(30, TILEMAP_HEIGHT - 9, 0x59);
	put_tile(31, TILEMAP_HEIGHT - 9, 0x59);
	put_tile(32, TILEMAP_HEIGHT - 9, 0x59);
	put_tile(33, TILEMAP_HEIGHT - 9, 0x59);

	// put_tile(31, TILEMAP_HEIGHT - 7, 5);
	// put_tile(31, TILEMAP_HEIGHT - 6, 7);

	put_tile(33, TILEMAP_HEIGHT - 8, 0x59);
	put_tile(33, TILEMAP_HEIGHT - 7, 0x59);
	put_tile(33, TILEMAP_HEIGHT - 6, 0x59);

	// chicken sign
	put_tile(480, TILEMAP_HEIGHT - 9, 5);
	put_tile(480, TILEMAP_HEIGHT - 8, 7);

	// door
	put_tile(491, 0x1d, 113);
	put_tile(491, 0x1e, 112);
	put_tile(492, 0x1d, 114);
	put_tile(492, 0x1e, 115);

	put_tile(444, 0x1a, 5);
	put_tile(444, 0x1b, 7);

	// visible, since orb is there :)
	put_tile(375, 0x03, 0x57);
	put_tile(376, 0x03, 0x57);
	put_tile(377, 0x03, 0x56);
	put_tile(375, 0x02, 0x10);
	put_tile(376, 0x02, 0x10);
	put_tile(377, 0x02, 0x10);
	put_tile(375, 0x00, 0x10);
	put_tile(376, 0x00, 0x10);
	put_tile(377, 0x00, 0x10);
	put_tile(375, 0x01, 0x10);
	put_tile(376, 0x01, 0x10);
	put_tile(377, 0x01, 0x10);
	put_tile(375, 0x04, 0x10);
	put_tile(376, 0x04, 0x10);
	put_tile(377, 0x04, 0x10);
	put_tile(374, 0x01, 0x10);
	put_tile(374, 0x02, 0x10);
	put_tile(374, 0x03, 0x10);
	put_tile(378, 0x01, 0x10);
	put_tile(378, 0x02, 0x10);
	put_tile(378, 0x03, 0x10);

	// stairs enabled through truth
	put_tile(352, 0x10, 1);
	put_tile(353, 0x10, 1);
	put_tile(351, 0x10, 1);
	put_tile(352, 0x11, 1);
	put_tile(353, 0x11, 1);
	put_tile(351, 0x11, 1);
	put_tile(350, 0x10, 1);
	put_tile(349, 0x10, 1);
	put_tile(348, 0x10, 1);
	put_tile(347, 0x10, 1);
	put_tile(346, 0x10, 1);
	put_tile(345, 0x10, 1);
	put_tile(344, 0x10, 1);
	put_tile(343, 0x10, 1);
	put_tile(342, 0x10, 1);
	// cage
	put_tile(341, 0x10, 0x5e);
	put_tile(340, 0x10, 0x5e);
	put_tile(339, 0x10, 0x5e);
	put_tile(338, 0x10, 0x5e);
	put_tile(337, 0x10, 0x5e);
	put_tile(336, 0x10, 0x5e);
	put_tile(336, 0x0f, 0x5e);
	put_tile(336, 0x0e, 0x5e);
	put_tile(336, 0x0d, 0x5e);
	put_tile(336, 0x0c, 0x5e);
	put_tile(337, 0x0c, 0x5e);
	put_tile(338, 0x0c, 0x5e);
	put_tile(339, 0x0c, 0x5e);
	put_tile(340, 0x0c, 0x5e);
	put_tile(341, 0x0c, 0x5e);
	put_tile(341, 0x0d, 0x5e);
	put_tile(341, 0x0e, 0x58);
	put_tile(341, 0x0f, 0x58);

	put_tile(450, 0x1e, 96);
	put_tile(450, 0x1d, 0);
	put_tile(451, 0x1e, 97);
	put_tile(451, 0x1d, 0);
	put_tile(451, 0x1c, 0);
	put_tile(446, 0x1c, 1);
	put_tile(447, 0x1c, 1);
	put_tile(448, 0x1c, 1);
	put_tile(449, 0x1c, 1);
	put_tile(450, 0x1c, 1);
	put_tile(451, 0x1c, 1);

	put_tile(450, 0x18, 1);
	put_tile(451, 0x18, 1);

	put_tile(448, 0x15, 1);
	put_tile(447, 0x15, 1);

	put_tile(450, 0x12, 1);
	put_tile(451, 0x12, 1);

	put_tile(448, 0x0f, 1);
	put_tile(447, 0x0f, 1);

	put_tile(450, 0x0c, 1);
	put_tile(451, 0x0c, 1);

	put_tile(448, 0x09, 1);
	put_tile(447, 0x09, 1);

	put_tile(450, 0x06, 1);
	put_tile(451, 0x06, 1);

	put_tile(452, 0x00, 0);
	put_tile(453, 0x00, 0);
	put_tile(452, 0x02, 0);
	put_tile(453, 0x02, 0);
	put_tile(452, 0x01, 0);
	put_tile(453, 0x01, 0);

	for (y = 0x03; y < 0x1a; y+=6) {
		for (x = 454; x < 458; x++) {
			put_tile(x, y, 0x5e);
		}
	}
	for (y = 0x06; y < 0x1a; y+=6) {
		for (x = 456; x < 460; x++) {
			put_tile(x, y, 0x5e);
		}
	}

	for (y = 0; y < 0x19; y++) {
		put_tile(460, y, 0x5e);
	}


	// final climb (skippable)
	for (y = 0; y < 0x16; y++) {
		put_tile(446, y, 1);
	}
	put_tile(342, 0x10, 1);
	put_tile(342, 0x10, 1);
	put_tile(342, 0x10, 1);

	put_tile(473, 0x16, 0x51);
	put_tile(473, 0x15, 0x52);
	put_tile(473, 0x14, 0x53);
	put_tile(472, 0x16, 0x53);
	put_tile(472, 0x15, 0x52);
	put_tile(472, 0x14, 0x51);


	put_tile(472, 0x18, 0);
	put_tile(472, 0x17, 0);
	put_tile(471, 0x18, 0);
	put_tile(471, 0x17, 0);
	put_tile(470, 0x18, 0);
	put_tile(470, 0x17, 0);
	put_tile(473, 0x18, 0);
	put_tile(473, 0x17, 0);

	for (x = TILEMAP_WIDTH - 0x10; x < TILEMAP_WIDTH; x++) {
		for (y = 0; y < TILEMAP_HEIGHT; y++) {
			put_tile(x, y, 0x55);
		}
	}

	return;
}

int main() {
	map_gen(57);
	int width = TILEMAP_WIDTH;
	printf("#include <types.h>\n\n#define TILEMAP_WIDTH %d\n#define TILEMAP_HEIGHT %d\n\nconst UINT8 TILEMAP[] = {\n", width, TILEMAP_HEIGHT);
	for (int y = 0; y < TILEMAP_HEIGHT; y++) {
		printf("  ");
		for (int i = 0; i < width; i++) {
			printf("0x%02x, ", TILEMAP[y * width + i]);
		}
		printf("\n");
	}
	printf("\n};\n");
}