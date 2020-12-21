#include <gb/gb.h>

#include "tileset.c"
#include "tilemap.c"
#include "hud.c"
#include "sprites.c"
#include "title_tilemap.c"

#include <stdio.h>

#define TRUE 1
#define FALSE 0

// TODO: use these
#define GRAVITY 2
// #define MAX_VEL 7
#define TILE_WIDTH 8

// #define WIDTH 144
// #define HEIGHT 160

#define SPRITE_ID_PLAYER_ZOPF 1
#define SPRITE_ID_PLAYER_HEAD 2
#define SPRITE_ID_PLAYER_BODY 3
#define SPRITE_ID_PLAYER_SWORD 5
#define SPRITE_ID_CHICKEN_LT 6
#define SPRITE_ID_CHICKEN_LB 7
#define SPRITE_ID_CHICKEN_RT 8
#define SPRITE_ID_CHICKEN_RB 9

// vscode does not find gb/gb.h, so it complains -.-
// TODO: delete this
#ifndef J_UP
	#define UINT8 int
	#define BOOLEAN int
	#define UINT16 int
	#define BYTE int
	#define INT16 int
	#define UBYTE int
	#define INT32 int
	#define INT8 int
	#define S_FLIPX 00100000
	#define S_FLIPY 01000000
	#define SHOW_BKG
	#define SHOW_WIN
	#define SHOW_SPRITES
	#define HIDE_SPRITES
	#define HIDE_WIN
	#define SPRITES_8x8
	#define J_UP 0
	#define J_DOWN 0
	#define J_LEFT 0
	#define J_RIGHT 0
	#define J_SELECT 0
	#define J_A 0
	#define J_B 0
	#define J_START 0
	#define get_bkg_tiles(x,y,width,height,cells)
	#define set_sprite_tile(a,b)
	#define move_sprite(a,b,c)
	#define get_sprite_prop(a) 0
	#define set_sprite_prop(a,b)
	#define set_win_tiles(x,y,width,height,cells)
	#define set_bkg_tiles(x,y,width,height,cells)
	#define wait_vbl_done()
	#define joypad() 0
	#define move_bkg(a,b)
	#define scroll_bkg(a,b)
	#define set_bkg_data(a,b,c)
	#define move_win(a,b)
	#define set_sprite_data(a,b,c)
#endif

UINT8 SPRITE_ZOPF[2];
const UINT8 SPRITE_ANIM_ZOPF_M[] = {
	0x7f, 0x7f
};
const UINT8 SPRITE_ANIM_ZOPF_F[] = {
	0x13, 0x12
};
UINT8 SPRITE_HEAD[2];
const UINT8 SPRITE_ANIM_HEAD_M[] = {
	0x00, 0x00
};
const UINT8 SPRITE_ANIM_HEAD_F[] = {
	0x14, 0x14
};
UINT8 SPRITE_BODY[4];
const UINT8 SPRITE_ANIM_BODY_M[] = {
	0x02, 0x01, 0x02, 0x03
};
const UINT8 SPRITE_ANIM_BODY_F[] = {
	0x16, 0x15, 0x16, 0x17
};

UINT8 SPRITE_ROLL[16];
const UINT8 SPRITE_HEAD_ROLL_M[] = {
	0x18, 0, 0x19, 0, 0x18, 1, 0x19, 1, // Right
	0x18, 2, 0x19, 2, 0x18, 3, 0x19, 3  // Left
};
const UINT8 SPRITE_HEAD_ROLL_F[] = {
	0x1a, 0, 0x1b, 0, 0x1a, 1, 0x1b, 1, // Right
	0x1a, 2, 0x1b, 2, 0x1a, 3, 0x1b, 3  // Left
};

const UINT8 ANIM_PLAYER_ATTACK[] = {
	128, 6, 6, 5, 5, 4, 4
};

const UINT8 title_animations[] = {
	0x04, 0x0e, 0x38,
	0x05, 0x0e, 0x35,
	0x06, 0x0e, 0x33,
	0x07, 0x0e, 0x31,
	0x08, 0x0e, 0x31,
	0x0B, 0x0e, 0x2e,
	0x0c, 0x0e, 0x39,
	0x0d, 0x0e, 0x2d,
	0x0e, 0x0e, 0x35,
	0x0f, 0x0e, 0x39,
	0x05, 0x0c, 0x11,
	0x08, 0x0c, 0x12,
};

#define PLAYER_DIRECTION_RIGHT 0
#define PLAYER_DIRECTION_LEFT  5

#define PLAYER_POS_X 80
#define PLAYER_POS_Y 102

// Variables containing player state
INT16 player_x;
UINT8 player_y;
UINT8 player_direction;
UINT8 player_animation_frame;
BOOLEAN is_player_walking;
#define PLAYER_HEIGHT 7
#define PLAYER_WIDTH 7

UINT8 scene;

// blocker: 80 - 95          0b11110000 => 0b01010000
#define blocker(cell)   (((cell) & 0xf0) == 0x50)
// breakable: 88 - 95        0b11111000 => 0b01011000
#define breakable(cell) (((cell) & 0xf8) == 0x58)
// death: 96 - 111           0b11110000 => 0b01100000
#define death(cell)     (((cell) & 0xf0) == 0x60)
// flag: 8 - 15              0b11111000 => 0b00001000
#define flag(cell)      (((cell) & 0xf8) == 0x8)
// trigger block
#define trigger(cell)   ((cell) == 112)
// info block
#define info(cell)      ((cell) == 5)

#define to_world(coord) ((coord) / TILE_WIDTH)
#define to_vram(coord) (to_world(coord) % 32)

#define to_tmap(x,y) ((y) * TILEMAP_WIDTH + (x))

// Level scrolling data
BOOLEAN grounded;
BOOLEAN crouch;
INT8 tileCounterX = 0;
UINT16 invulnerable;

UINT16 transition_time;
UINT16 transition_d = 0;
UINT8 transition_speedup = 1;


INT16 scroll;

// Velocity and direction
INT8 velX;
INT8 velY;

// attacking
UINT8 attack_timer;
UINT8 attack_cool;
UINT8 attack_direction;

UINT8 flags = 0;
UINT8 health;

UINT8 camera_pos;

#define X_OFFSET 0x20

// Draw stuff over our map, which we can clear, since the TILEMAP is const and therefore unchangeable. 2 tiles per X should be enough?
UINT8 interactive_layer[4 * TILEMAP_WIDTH];
// Tile 1:
// interactive_layer[X * 4 + 0] = Y1
// interactive_layer[X * 4 + 1] = TILE1
// Tile 2:
// interactive_layer[X * 4 + 2] = Y2
// interactive_layer[X * 4 + 3] = TILE2

UINT8 global_timer;
UINT8 air_timer;

BOOLEAN disable_animation = FALSE;
BOOLEAN info_shown;

INT8 has_orb;

// signs

// HXP semi-proudly presents..
const UINT8 SIGN1[] = {
	0x48, 0x58, 0x50, 0x20, 0x73, 0x65, 0x6d, 0x69, 0x2d, 0x0a, 0x70, 0x72, 0x6f, 0x75, 0x64, 0x6c, 0x79, 0x0a, 0x70, 0x72, 0x65, 0x73, 0x65, 0x6e, 0x74, 0x73, 0x2e, 0x2e
};

// Find the chicken
const UINT8 SIGN2[] = {
	0x46, 0x49, 0x4e, 0x44, 0x20, 0x54, 0x48, 0x45, 0x0a, 0x43, 0x48, 0x49, 0x43, 0x4b, 0x45, 0x4e, 0x20, 29
};

const UINT8 SIGN_ORB[] = {
	0x46, 0x69, 0x6e, 0x64, 0x20, 0x74, 0x68, 0x65, 0x0a, 0x4f, 0x52, 0x42, 0x20, 0x6f, 0x66, 0x20, 0x54, 0x52, 0x55, 0x54, 0x48
};
const UINT8 SIGN_GO[] = {
	0x47, 0x6f, 0x20, 0x6f, 0x6e, 0x0a, 0x61, 0x64, 0x76, 0x65, 0x6e, 0x74, 0x75, 0x72, 0x65, 0x72
};

const UINT8 FLAG[] = {
#ifdef release
	// fake flag
	0x68, 0x78, 0x70, 0x7b, 0x0a, 0x74, 0x68, 0x69, 0x73, 0x5f, 0x69, 0x73, 0x0a, 0x5f, 0x6e, 0x6f, 0x74, 0x5f, 0x74, 0x68, 0x65, 0x5f, 0x0a, 0x72, 0x65, 0x61, 0x6c, 0x5f, 0x66, 0x6c, 0x61, 0x67, 0x7d
#else
	// real flag
	0x68, 0x78, 0x70, 0x7b, 0x0a, 0x59, 0x34, 0x79, 0x5f, 0x79, 0x6f, 0x75, 0x0a, 0x5f, 0x66, 0x30, 0x75, 0x6e, 0x64, 0x5f, 0x74, 0x68, 0x33, 0x0a, 0x5f, 0x63, 0x68, 0x31, 0x63, 0x6b, 0x65, 0x6e, 0x7d
#endif
};

const UINT8 SHOW_CHICKEN[] = {
	0x73, 0x68, 0x6f, 0x77, 0x0a, 0x63, 0x68, 0x69, 0x63, 0x6b, 0x65, 0x6e, 0x0a, 28
};

// void debug(const UINT8 x, UINT8 no) {
// 	if (no > 100) {
// 		set_sprite_tile(20 + x, 0x30 + no / 100);
// 		move_sprite(20 + x, 0x20 + x * 8, 50);
// 		no %= 100;
// 	}
// 	if (no > 10) {
// 		set_sprite_tile(21 + x, 0x30 + no / 10);
// 		move_sprite(21 + x, 0x28 + x * 8, 50);
// 		no %= 10;
// 	}
// 	set_sprite_tile(22 + x, 0x30 + no / 10);
// 	move_sprite(22 + x, 0x38 + x * 8, 50);
// }

// Update the tiles of the sprite to animate it.
//
// sprite_id: the id ("nb") of the sprite to update
// anim:      pointer to the animation data
// direction: direction of the animation (= offset of the animation in the global animation data)
// frame:     the new frame of the animation that will be displayed
//
// Returns the next frame of the animation.
void update_sprite_animation(UINT8 sprite_id, const UINT8 *anim, UINT8 direction, UINT8 frame) {
	UINT8 flip = direction == PLAYER_DIRECTION_LEFT;
	UINT8 tile_id = anim[frame];

	// reset sprite
	set_sprite_prop(sprite_id, get_sprite_prop(sprite_id) & ~(S_FLIPX | S_FLIPY));
	if (flip) {
		set_sprite_prop(sprite_id, get_sprite_prop(sprite_id) | S_FLIPX);
	}

	set_sprite_tile(sprite_id, tile_id);
}

void roll_head(UINT8 sprite_id, const UINT8 *anim, UINT8 frame) {
	UINT8 pos = (player_direction == PLAYER_DIRECTION_LEFT) * 8 + (frame * 2);
	UINT8 sprite = anim[pos];
	UINT8 props = anim[pos + 1];

	set_sprite_tile(sprite_id, sprite);

	// reset
	set_sprite_prop(sprite_id, get_sprite_prop(sprite_id) & ~(S_FLIPX | S_FLIPY));

	switch (props) {
		case 1:
			props = S_FLIPX | S_FLIPY;
			break;
		case 2:
			props = S_FLIPX;
			break;
		case 3:
			props = S_FLIPY;
			break;
		default:
			return;
	}

	set_sprite_prop(sprite_id, get_sprite_prop(sprite_id) | props);
}

void start_transition() {
	transition_time = 60;
	transition_d = 0;
	transition_speedup = 1;
}

void brk(const UINT8 x, const UINT8 y, UINT8 *next_cell) {
	switch (next_cell[0]) {
		case 0x58:
			next_cell[0] = 0x59;
			break;
		case 0x59:
			next_cell[0] = 0;
			if (to_world(player_x) > 260 && to_world(player_x) < 285) {
				if (x == 0x0e) {
					if (y == 0x1d) {
						next_cell[0] = 7;
					} else if (y == 0x1c) {
						next_cell[0] = 9;
					}
				} else if (x == 0x0f && y == 0x1c) {
					next_cell[0] = 10;
				}
			}
			break;
		case 0x5e:
		case 0x5f:
			// breaking invisible blocks.
			next_cell[0] = 0;
			break;
		default:
			return;
	}
	set_bkg_tiles(x, y, 1, 1, next_cell);
}

void attack() {
	UINT8 i;
	UINT8 x;
	UINT8 y;
	UINT8 cell[1];
	y = to_vram(player_y);

	if (attack_direction == 0) {
		x = to_vram(player_x) + 1;
	} else {
		x = to_vram(player_x) - 1;
	}

	for (i = 0; i < 2; i++) {
		get_bkg_tiles(x, y, 1, 1, cell);
		if (breakable(cell[0])) {
			brk(x, y, cell);
		}
		x++;
	}
}

void display_msg(UINT8 x, UINT8 y, const UINT8 *text, const UINT8 length, const UINT8 reset) {
	UINT8 i;
	UINT8 id = 10;
	if (info_shown) {
		// already showing, return
		return;
	}
	for (i = 0; i < length; i++) {
		x += 8;
		if (text[i] == 0) {
			break;
		}
		if (text[i] == 0x20) {
			continue;
		}
		if (text[i] == 0x0a) {
			y += 8;
			x -= reset;
			continue;
		}
		set_sprite_tile(id, text[i]);
		move_sprite(id, x, y);
		id++;
	}
	info_shown = TRUE;
}

void hide_info_msg() {
	UINT8 i;
	if (!info_shown) {
		return;
	}
	info_shown = FALSE;
	for (i = 10; i < 40; i++) {
		move_sprite(i, 0, 0);
	}
}

void info_msg() {
	UINT8 text[11] = { 0, 0x2f, 0x34, 0x20, 0x46, 0x6c, 0x61, 0x67, 0x73, 0x0a, 0x1e };
	UINT8 cell[1];
	UINT16 p_pos = to_world(player_x);
	UINT16 y = to_world(player_y);
	switch (p_pos) {
		case 479:
		case 480:
			if (y > 0x16) {
				display_msg(50, 50, SHOW_CHICKEN, 14, 0x20);
			}
			break;
		case 490:
		case 491:
		case 492:
			if (y > 0x1B) {
				if (flags != 0xf) {
					text[0] = 0x30 + (flags & 1) + ((flags & 2) >> 1) + ((flags & 4) >> 2) + ((flags & 8) >> 3);
					display_msg(0x30, 100, text, 11, 0x30);
				} else {
					start_transition();
				}
				break;
			}
		case 19:
		case 20:
			if (y > 0x19) {
				display_msg(20, 50, SIGN1, 28, 0x48);
			}
			break;
		case 24:
		case 25:
			get_bkg_tiles(0x12, 0x10, 1, 1, cell);
			if (info_shown || cell[0] != 0) {
				// already showing, return
				return;
			}

			for (p_pos = 1; p_pos < 7; p_pos++) {
				for (y = 1; y < 18; y++) {
					cell[0] = title_tilemap[p_pos * title_tilemapWidth + y];
					set_bkg_tiles((y + 0x10) % 32, p_pos + 0x0f, 1, 1, cell);
				}
			}
			break;
		case 443:
		case 444:
			if (y > 0x19) {
				if (has_orb) {
					display_msg(20, 50, SIGN_GO, 16, 0x38);
				} else {
					display_msg(20, 50, SIGN_ORB, 21, 0x38);
				}
			}
			break;
		default:
			if (info_shown) {
				hide_info_msg();
			}
			break;
	}
}

// ============================================================
// Map Generation
// ============================================================

void put_tile(const UINT16 x, const UINT16 y, const UINT8 tile, const UINT8 pos) {
	interactive_layer[x * 4 + 2 * pos] = y;
	interactive_layer[x * 4 + 2 * pos + 1] = tile;
}

void map_gen() {
	UINT16 i;
	// we have to reset the entire interactive_layer because it does not get initialized to 0 for some reason?
	for (i = 0; i < 4 * TILEMAP_WIDTH; i++) {
		interactive_layer[i] = 0;
	}

	// flag 1
	put_tile(190, 0x13, 7, 0);
	put_tile(190, 0x12, 9, 1);
	put_tile(191, 0x12, 10, 0);

	// flag 2
	put_tile(270, 0x1d, 0x58, 1);
	put_tile(270, 0x1c, 0x58, 0);
	put_tile(271, 0x1d, 0x58, 1);
	put_tile(271, 0x1c, 0x58, 0);

	// flag 3
	put_tile(338, 0xf, 7, 0);
	put_tile(338, 0xe, 9, 1);
	put_tile(339, 0xe, 10, 0);

	// flag 4
	put_tile(471, 0x18, 7, 0);
	put_tile(471, 0x17, 9, 1);
	put_tile(472, 0x17, 10, 0);

	// invisible steps to orb (sorry dear reverser)
	put_tile(397, 0x15, 0x5f, 0);

	put_tile(393, 0x11, 0x5f, 0);
	put_tile(394, 0x11, 0x5f, 0);

	put_tile(387, 0x0e, 0x5f, 0);
	put_tile(388, 0x0e, 0x5f, 0);

	put_tile(392, 0x0A, 0x5f, 0);
	put_tile(393, 0x0A, 0x5f, 1);

	put_tile(374, 0x08, 0x5f, 1);
	put_tile(377, 0x08, 0x5f, 1);
	put_tile(378, 0x08, 0x5f, 1);
	put_tile(379, 0x08, 0x5f, 1);
	put_tile(382, 0x08, 0x5f, 1);
	put_tile(385, 0x08, 0x5f, 1);
	put_tile(388, 0x08, 0x5f, 1);

	put_tile(371, 0x06, 0x5f, 0);
	put_tile(372, 0x06, 0x5f, 0);

	// orb, caution is adviced: game-breaking
	put_tile(376, 0x2, 6, 0);
}

// ============================================================
// Background animation
// ============================================================

struct bg_animation {
	UINT8 x;
	UINT8 y;
	UINT8 state;
	UINT8 frames[4];
};

#define ANIMATION_COUNT 32
struct bg_animation bg_animations[ANIMATION_COUNT];

void register_animation(const UINT8* tile, const UINT8 vram_x, const UINT8 vram_y) {
	UINT8 i;
	if (tile[0] == 0x60 || tile[0] == 0x61 || tile[0] == 0xa || tile[0] == 6) {
		for (i = 0; i < ANIMATION_COUNT; i++) {
			if (bg_animations[i].state == 0) {
				bg_animations[i].x = vram_x % 32;
				bg_animations[i].y = vram_y % 32;
				bg_animations[i].state = tile[0];
				switch (tile[0]) {
					case 96:
						bg_animations[i].frames[0] = 96;
						bg_animations[i].frames[1] = 98;
						bg_animations[i].frames[2] = 97;
						bg_animations[i].frames[3] = 99;
						break;
					case 97:
						bg_animations[i].frames[0] = 97;
						bg_animations[i].frames[1] = 99;
						bg_animations[i].frames[2] = 96;
						bg_animations[i].frames[3] = 98;
						break;
					case 10:
						bg_animations[i].frames[0] = 10;
						bg_animations[i].frames[1] = 11;
						bg_animations[i].frames[2] = 12;
						bg_animations[i].frames[3] = 13;
						break;
					case 6:
						bg_animations[i].frames[0] = 6;
						bg_animations[i].frames[1] = 2;
						bg_animations[i].frames[2] = 3;
						bg_animations[i].frames[3] = 6;
						break;
				}
				break;
			}
		}
	}
}

void clear_animation(const UINT8 x, const UINT8 y) {
	UINT8 i;
	for (i = 0; i < ANIMATION_COUNT; i++) {
		if (bg_animations[i].x == x && bg_animations[i].y == y) {
			bg_animations[i].state = 0;
			break;
		}
	}
}

void animate_background() {
	// current vram positions
	UINT8 cell[1];
	UINT8 i, x, y;
	//                0b1111    0b0000
	if ((global_timer & 0xf) != 0) {
		// do NOT update on every frame!
		return;
	}
	for (i = 0; i < ANIMATION_COUNT; i++) {
		if (bg_animations[i].state != 0) {
			x = to_vram(player_x);
			// only update if visible, no idea if this even helps
			if (x < 9) {
				if (bg_animations[i].x > 23 + x) {
					// right edge
				} else if (bg_animations[i].x < x + 13) {
					// left edge
				} else {
					// the part in between
					continue;
				}
			} else if (x > 21) {
				if (bg_animations[i].x > x - 9) {
					// right edge
				} else if (bg_animations[i].x < x - 19) {
					// left edge
				} else {
					// the part in between
					continue;
				}
			} else {
				if (bg_animations[i].x < x + 13 && bg_animations[i].x > x - 10) {
					// stuff i can see
				} else {
					// the part in between
					continue;
				}
			}
			x = bg_animations[i].x;
			y = bg_animations[i].y;

			//                 0b111111
			switch (global_timer & 0x3f) {
				case 0x30: // 0b110000
					cell[0] = bg_animations[i].frames[0];
					break;
				case 0x20: // 0b100000
					cell[0] = bg_animations[i].frames[1];
					break;
				case 0x10: // 0b010000
					cell[0] = bg_animations[i].frames[2];
					break;
				case 0: // 0b000000
					cell[0] = bg_animations[i].frames[3];
					break;
				default:
					// should be handled above
					return;
			}
			set_bkg_tiles(x, y, 1, 1, cell);
		}
	}

}

// ============================================================
// Visual updates: Background and sprites
// ============================================================

void draw_background_vertical(const UINT8 vram_x, const UINT16 tilemap_x, const UINT16 tilemap_y) {
	UINT8 tile[1];
	UINT8 vram_y;
	UINT16 layer_idx = tilemap_x * 4;
	for(vram_y = 0; vram_y < 32; vram_y++) {
		tile[0] = TILEMAP[(tilemap_y + vram_y) * TILEMAP_WIDTH + tilemap_x];
		// overwrite everything which is not 0, you know why :)
		if (interactive_layer[layer_idx] == vram_y && interactive_layer[layer_idx + 1]) {
			tile[0] = interactive_layer[layer_idx + 1];
		} else if (interactive_layer[layer_idx + 2] == vram_y && interactive_layer[layer_idx + 3]) {
			tile[0] = interactive_layer[layer_idx + 3];
		}
		if (has_orb && tile[0] == 6) {
			tile[0] = TILEMAP[(tilemap_y + vram_y) * TILEMAP_WIDTH + tilemap_x];
		}
		set_bkg_tiles(vram_x, vram_y, 1, 1, tile);
		register_animation(tile, vram_x, vram_y);
	}
}

void aura_layer(UINT8 x, UINT8 y) {
	UINT8 cell[1];

	get_bkg_tiles(x, y, 1, 1, cell);
	switch (cell[0]) {
		case 0:
		case 4:
			cell[0] = 0x10;
			break;
		case 1:
		case 0x5f:
			cell[0] = 0x5e;
			break;
		default:
			return;
	}
	set_bkg_tiles(x, y, 1, 1, cell);

}

void aura_x() {
	// weirdly enough, this is pretty small in asm and performant?
	UINT8 x, y;

	if (has_orb < 1) {
		return;
	}
	if (velX < 0) {
		x = to_vram(player_x);
		y = to_vram(player_y);
		y = (y - 3) % 32;
		x = (x - 2) % 32;
		aura_layer(x, y); // -3
		y++;
		y %= 32;
		aura_layer(x, y); // -2
		y++;
		y++;
		y++;
		y %= 32;
		aura_layer(x, y); // +1
		y++;
		y %= 32;
		aura_layer(x, y); // +2
		x--;
		goto small_x;
	} else if (velX > 0) {
		x = to_vram(player_x);
		y = to_vram(player_y);
		y = (y - 3) % 32;
		x = (x + 2) % 32;
		aura_layer(x, y); // -3
		y++;
		y %= 32;
		aura_layer(x, y); // -2
		y++;
		y++;
		y++;
		y %= 32;
		aura_layer(x, y); // +1
		y++;
		y %= 32;
		aura_layer(x, y); // +2
		x++;
		goto small_x;
	} else {
		return;
	}
small_x:
	x %= 32;
	y--;
	y %= 32;
	aura_layer(x, y); // +1
	y--;
	y %= 32;
	aura_layer(x, y); // 0
	y--;
	y %= 32;
	aura_layer(x, y); // -1
	y--;
	y %= 32;
	aura_layer(x, y); // -2
}

void aura_y() {
	UINT8 x, y;

	if (has_orb < 1) {
		return;
	}

	if (velY < 0) {
		x = to_vram(player_x);
		y = to_vram(player_y);
		x = (x - 3) % 32;
		y -= 2;
		y %= 32;
		aura_layer(x, y); // -3
		x += 6;
		x %= 32;
		aura_layer(x, y); // +3
		y--;
		goto small_y;
	} else if (velY > 0) {
		x = to_vram(player_x);
		y = to_vram(player_y);
		x = (x - 3) % 32;
		y++;
		y %= 32;
		aura_layer(x, y); // -3
		x += 6;
		x %= 32;
		aura_layer(x, y); // +3
		y++;
		goto small_y;
	} else {
		return;
	}
small_y:
	y %= 32;
	x--;
	x %= 32;
	aura_layer(x, y); // +2
	x--;
	x %= 32;
	aura_layer(x, y); // +1
	x--;
	x %= 32;
	aura_layer(x, y); // 0
	x--;
	x %= 32;
	aura_layer(x, y); // -1
	x--;
	x %= 32;
	aura_layer(x, y); // -2
}

/**
Updates background based on current velocity in x-dir.
*/
void update_background() {
	UINT16 loc;
	UINT8 vram;
	UINT8 i;

	tileCounterX += velX;
	// always scroll bg
	scroll_bkg(velX, 0);

	if (tileCounterX >= 8) {
		// walking right
		tileCounterX = tileCounterX % 8;
		scroll++;
		loc = 31 + scroll;
		vram = (scroll - 1) % 32;
	} else if (tileCounterX <= -8) {
		// walking left
		tileCounterX = ((tileCounterX * -1) % 8) * -1;
		scroll--;
		loc = scroll;
		vram = scroll % 32;
	} else {
		// standing still or moving to little: no need to update background
		return;
	}
	aura_x();
	// we do not need to handle scrolling over the edge.
	for (i = 0; i < ANIMATION_COUNT; i++) {
		if (bg_animations[i].x == (UINT8) vram) {
			bg_animations[i].state = 0;
		}
	}
	draw_background_vertical(vram, loc, 0);
}

/*
Spiral transition effect
*/
void transition() {
	static INT8 x = 0;
	static INT8 y = 0;
	UINT8 i;
	static UINT16 next_change = 0;
	static UINT8 transition_direction = 0;
	const UINT8 black_tile[1] = { 26 };

	if (transition_d == 0) {
		x = to_vram(player_x);
		y = to_vram(player_y) - 2;
		next_change = 1;
		transition_direction = J_UP;
	}

	for (i = 0; i < transition_speedup; i++) {
		transition_d++;

		if (transition_direction == J_UP) {
			y--;
			if (y < 0) {
				y = 0x1f;
			}
		} else if (transition_direction == J_LEFT) {
			x--;
		} else if (transition_direction == J_DOWN) {
			y++;
			if (y > 0x1f) {
				y = 0;
			}
		} else if (transition_direction == J_RIGHT) {
			x++;
		}
		set_bkg_tiles(x, y, 1, 1, black_tile);
		if (transition_d == (next_change * next_change / 4 + 1)) {
			if (transition_direction == J_UP) {
				transition_direction = J_LEFT;
			} else if (transition_direction == J_LEFT) {
				transition_direction = J_DOWN;
			} else if (transition_direction == J_DOWN) {
				transition_direction = J_RIGHT;
			} else if (transition_direction == J_RIGHT) {
				transition_direction = J_UP;
			}
			next_change++;
		}
	}
	if (transition_d % 3 == 0) {
		transition_speedup++;
	}
}

void use() {
	UINT8 cell[1];
	UINT16 y = to_vram(player_y);
	UINT16 x = to_vram(player_x);
	UINT8 i;

	if (has_orb == 1) {
		// drop in *front* of me >:)
		// remove from window
		cell[0] = 26;
		set_win_tiles(9, 0, 1, 1, cell);
		// add to vram
		cell[0] = 6;
		x++;
		x %= 32;
		set_bkg_tiles(x, y, 1, 1, cell);
		register_animation(cell, x, y);
		x = to_world(player_x) + 1;
		has_orb = -19;
		goto update_layer;
	} else if (!has_orb) {
		for (i = 0; i < 2; i++) {
			get_bkg_tiles(x, y, 1, 1, cell);
			if (cell[0] == 6) {
				clear_animation(x, y);
				// add to window
				cell[0] = 19;
				set_win_tiles(9, 0, 1, 1, cell);
				// remove from vram
				cell[0] = 0;
				set_bkg_tiles(x, y, 1, 1, cell);
				x = to_world(player_x);
				has_orb = 20;
				// aura();
				goto update_layer;
			}
			x++;
			x %= 32;
		}
	}
	return;
update_layer:
	interactive_layer[x * 4] = y;
	interactive_layer[x * 4 + 1] = cell[0];
}

// ============================================================
// Status bar updates: Health and Flags
// ============================================================

/*
Player grabs a flag.
*/
void catch_flag(UINT8 x, UINT8 y) {
	UINT8 cell[1];
	// find all 3 sprites
	const UINT8 cells[4] = {0,0,0,0};
	UINT16 flag_no;
	get_bkg_tiles(x, y, 1, 1, cell);
	if (cell[0] == 8) {
		y = y - 1;
	} else if (cell[0] == 9) {
		// everything fine
	} else {
		x = x - 1;
	}

	// misuse flag_no as world_space
	flag_no = to_world(player_x);

	if (flag_no < 200) {
		flag_no = 1;
	} else if (flag_no < 300) {
		flag_no = 2;
	} else if (flag_no < 400) {
		flag_no = 4;
	} else {
		flag_no = 8;
	}

	// because of the or, we could in theory ignore clearing the flag for real, but we'll do it anyways
	flags |= flag_no;
	// hide flag for now
	set_bkg_tiles(x, y, 2, 2, cells);
	clear_animation(x + 1, y);

	// really clear flag, and show it as catched
	cell[0] = 22;
	if (flag_no == 1) {
		flag_no = 190;
		y = 12;
	} else if (flag_no == 2) {
		flag_no = 251;
		y = 14;
	} else if (flag_no == 4) {
		flag_no = 338;
		y = 16;
	} else if (flag_no == 8) {
		flag_no = 471;
		y = 18;
	} else {
		return;
	}
	set_win_tiles(y, 0, 1, 1, cell);
	put_tile(flag_no, 0, 0, 0);
	put_tile(flag_no, 0, 0, 1);
	flag_no++;
	put_tile(flag_no, 0, 0, 0);
	put_tile(flag_no, 0, 0, 1);
}

/*
Player loses one life.
*/
void die() {
	const UINT8 cell[1] = { 21 };
	if (invulnerable) {
		return;
	}
	health--;
	invulnerable = 100;

	switch (health) {
		case 2:
			set_win_tiles(5, 0, 1, 1, cell);
			break;
		case 1:
			set_win_tiles(3, 0, 1, 1, cell);
			break;
		case 0:
			set_win_tiles(1, 0, 1, 1, cell);
			break;
	}
}

// ============================================================
// Collisions
// ============================================================

void collision() {
	UINT8 x_cell[2];
	UINT16 x;
	UINT8 x_arr[2];
	UINT16 y;
	UINT8 y_cell_l[1];
	UINT8 y_cell_r[1];
	UINT8 y_cell[1];
	UINT8 i;

	if (crouch) {
		y = to_vram(player_y);
	} else {
		y = to_vram(player_y) - 1;
	}
	// collision left & right
	for (i = 0; i < 2; i++) {
		if (velX != 0) {
			if (i == 0) {
				// right
				x = to_vram(player_x + velX + PLAYER_WIDTH);
			} else {
				// left
				x = to_vram(player_x + velX);
			}

			get_bkg_tiles(x, y, 1, 2, x_cell);
			if (death(x_cell[0]) || (!crouch && death(x_cell[1]))) {
				die();
			}
			if (blocker(x_cell[0]) || (!crouch && blocker(x_cell[1]))) {
				velX = 0;
			}
			if (flag(x_cell[0])) {
				catch_flag(x, y);
			} else if (flag(x_cell[1])) {
				catch_flag(x, y + 1);
			}

		}
	}

	if (velY > 7) {
		velY = 7;
	}

	if (velY >= 0) {
		// collision bottom
		x_arr[0] = to_vram(player_x + velX + PLAYER_WIDTH);
		x_arr[1] = to_vram(player_x + velX);

		// check if we hit the ground with this velocity, velY > 0
		for (i = velY; i > 0; i--) {
			y = to_vram(player_y + i + PLAYER_HEIGHT);
			get_bkg_tiles(x_arr[0], y, 1, 1, y_cell_l);
			get_bkg_tiles(x_arr[1], y, 1, 1, y_cell_r);
			if (!blocker(y_cell_l[0]) && !blocker(y_cell_r[0])) {
				// determine velocity to hit ground
				if (i != (UINT8) velY) {
					velY = i + 1;
				}
				break;
			}
		}

		y = to_vram(player_y + velY + PLAYER_HEIGHT);
		get_bkg_tiles(x_arr[0], y, 1, 1, y_cell_l);
		get_bkg_tiles(x_arr[1], y, 1, 1, y_cell_r);
		if (death(y_cell_r[0]) || death(y_cell_l[0])) {
			die();
		}
		// start falling when not on blocker anymore
		if (blocker(y_cell_l[0]) || blocker(y_cell_r[0])) {
			grounded = TRUE;
			velY = 0;
		} else {
			grounded = FALSE;
			if (air_timer % 10 == 0) {
				velY++;
			}
		}
	} else {
		if (air_timer % 10 == 0) {
			velY++;
		}
		// ignore top collision when crouched
		if (crouch) {
			return;
		}
		y = to_vram(player_y + velY + 1) - 1;
		// collision top
		// right side
		x = to_vram(player_x + velX + PLAYER_WIDTH);
		get_bkg_tiles(x, y, 1, 1, y_cell);
		if (blocker(y_cell[0])) {
			velY = 1;
		}
		if (breakable(y_cell[0])) {
			brk(x, y, y_cell);
		}
		// left side
		if (to_vram(player_x + velX) == x) {
			// already checked this
			return;
		}
		x = to_vram(player_x + velX);
		get_bkg_tiles(x, y, 1, 1, y_cell);
		if (blocker(y_cell[0])) {
			velY = 1;
		}
		if (breakable(y_cell[0])) {
			brk(x, y, y_cell);
		}
	}
}

// void wait(UINT8 frames) {
// 	UINT8 i;
// 	for (i = 0; i < frames; i++) {
// 		wait_vbl_done();
// 	}
// }

// ============================================================
// Start screen
// ============================================================

void display_menu() {
	UINT8 i, j;
	const UINT8 cells[1] = { 0 };
	scene = 1;
	player_x = 80;

	// Copy the tileset in the video memory
	set_bkg_data(0, 128, TILESET);

	// Copy the tilemap in the video memory
	set_bkg_tiles(0, 0, title_tilemapWidth, title_tilemapHeight, title_tilemap);

	move_bkg(0, 0);

	// Display the Background and Window layers
	HIDE_WIN;

	// clear all animations
	for (i = 0; i < ANIMATION_COUNT; i++) {
		bg_animations[i].state = 0;
	}

	// Press Start blinking
	for (i = 0, j = 0; i < 12; i++) {
		bg_animations[i].x = title_animations[j];
		j++;
		bg_animations[i].y = title_animations[j];
		j++;
		bg_animations[i].state = title_animations[j];
		j++;
		bg_animations[i].frames[0] = bg_animations[i].state;
		bg_animations[i].frames[1] = bg_animations[i].state;
		bg_animations[i].frames[2] = bg_animations[i].state;
		bg_animations[i].frames[3] = 0x00;
	}

	if (SPRITE_HEAD[0] == SPRITE_ANIM_HEAD_M[0]) {
		i = 11;
	} else {
		i = 10;
	}
	bg_animations[i].frames[0] = 0;
	bg_animations[i].frames[1] = 0;
	bg_animations[i].frames[2] = 0;

	move_sprite(SPRITE_ID_PLAYER_ZOPF, 0x34, 0x6c);
	move_sprite(SPRITE_ID_PLAYER_HEAD, 0x3c, 0x6c);
	move_sprite(SPRITE_ID_PLAYER_BODY, 0x3c, 0x74);
	set_sprite_tile(SPRITE_ID_PLAYER_ZOPF, SPRITE_ZOPF[0]);
	set_sprite_tile(SPRITE_ID_PLAYER_HEAD, SPRITE_HEAD[0]);
	set_sprite_tile(SPRITE_ID_PLAYER_BODY, SPRITE_BODY[0]);
	set_sprite_prop(SPRITE_ID_PLAYER_ZOPF, get_sprite_prop(SPRITE_ID_PLAYER_ZOPF) & ~(S_FLIPX | S_FLIPY));
	set_sprite_prop(SPRITE_ID_PLAYER_HEAD, get_sprite_prop(SPRITE_ID_PLAYER_HEAD) & ~(S_FLIPX | S_FLIPY));
	set_sprite_prop(SPRITE_ID_PLAYER_BODY, get_sprite_prop(SPRITE_ID_PLAYER_BODY) & ~(S_FLIPX | S_FLIPY));

	if (flags == 0x0f) {
		j = 12;
		for (i = 0; i < 4; i++) {
			set_sprite_tile(i + SPRITE_ID_CHICKEN_LT, j);
			j++;
		}
		move_sprite(SPRITE_ID_CHICKEN_LT, 0x60, 0x6c);
		move_sprite(SPRITE_ID_CHICKEN_LB, 0x60, 0x74);
		move_sprite(SPRITE_ID_CHICKEN_RT, 0x68, 0x6c);
		move_sprite(SPRITE_ID_CHICKEN_RB, 0x68, 0x74);

		display_msg(0x18, 0x48, FLAG, 33, 0x30);
	}

	transition_d = 0;
}

void start_game() {
	UINT8 i;
	// Initialize player's state
	player_x = PLAYER_POS_X;
	player_y = 0x80;
	player_direction = PLAYER_DIRECTION_RIGHT;
	player_animation_frame = 0;
	is_player_walking = FALSE;

	hide_info_msg();

	// init the two sprites used for the player
	move_sprite(SPRITE_ID_PLAYER_ZOPF, PLAYER_POS_X - 8, player_y);
	move_sprite(SPRITE_ID_PLAYER_HEAD, PLAYER_POS_X, player_y);
	move_sprite(SPRITE_ID_PLAYER_BODY, PLAYER_POS_X, player_y + 8);
	// "white" is transparent by default
	//set_sprite_prop(SPRITE_ID_PLAYER_BODY, S_PALETTE);

	move_sprite(SPRITE_ID_CHICKEN_LT, 0, 0);
	move_sprite(SPRITE_ID_CHICKEN_LB, 0, 0);
	move_sprite(SPRITE_ID_CHICKEN_RT, 0, 0);
	move_sprite(SPRITE_ID_CHICKEN_RB, 0, 0);

	map_gen();

	for (i = 0; i < ANIMATION_COUNT; i++) {
		clear_animation(bg_animations[i].x, bg_animations[i].y);
	}
	// load tile set
	set_bkg_data(0, 128, TILESET);
	for (i = 0; i < 32; i++) {
		draw_background_vertical(i, i, 0);
	}
	move_bkg(X_OFFSET, 0x78);

	// copy tilemap in the Window layer
	set_win_tiles(0, 0, WINDOW_TILEMAP_WIDTH, WINDOW_TILEMAP_HEIGHT, WINDOW_TILEMAP);
	// move Window layer to display it at the bottom of the screen
	move_win(7, 136);
	// Display the Background and Window layers
	SHOW_WIN;

	player_x += X_OFFSET;
	player_x -= 8;
	player_y -= 7;

	velX = 0;
	velY = 2;

	scene = 0;

	flags = 0;
	health = 3;
	scroll = 0;
	camera_pos = 0x70;
	grounded = FALSE;

	attack_timer = 0;
	attack_cool = 0;
	has_orb = FALSE;
	crouch = FALSE;
	tileCounterX = 0;
	invulnerable = 0;
	air_timer = 0;
	info_shown = FALSE;
}

int main(void) {
	INT8 attack_offset = 0;
	UINT8 keys = 0;
	UINT8 frame_skip = 8;
	UINT8 i, x, y;
	UINT8 cell[1];
	UINT8 global_animation = 0;
	// 8 frames = ~133 ms between each animation frames

	// display 8x8 sprites
	SPRITES_8x8;
	// load sprites' tiles in video memory
	set_sprite_data(0, 126, SPRITES);

	SHOW_SPRITES;
	SHOW_BKG;

	SPRITE_ZOPF[0] = SPRITE_ANIM_ZOPF_M[0];
	SPRITE_HEAD[0] = SPRITE_ANIM_HEAD_M[0];
	SPRITE_ZOPF[1] = SPRITE_ANIM_ZOPF_M[1];
	SPRITE_HEAD[1] = SPRITE_ANIM_HEAD_M[1];
	for (i = 0; i < 4; i++) {
		SPRITE_BODY[i] = SPRITE_ANIM_BODY_M[i];
	}
	for (i = 0; i < 16; i++) {
		SPRITE_ROLL[i] = SPRITE_HEAD_ROLL_M[i];
	}

	transition_time = 0;

	display_menu();

	global_timer = 0;

	while (TRUE) {
		// Wait for v-blank (screen refresh)
		wait_vbl_done();
		global_timer++;
		keys = joypad();

		if (scene == 0) {
			if (velY != 0) {
				air_timer++;
			}
			if (invulnerable > 0) {
				invulnerable--;
			}
			if (transition_time > 0) {
				transition_time--;
				transition();
				if (transition_time == 0) {
					display_menu();
				}
				continue;
			}

			// Left & Right
			if (keys & J_LEFT) {
				velX = -1;
				player_direction = PLAYER_DIRECTION_LEFT;
				is_player_walking = TRUE;
			} else if (keys & J_RIGHT) {
				velX = 1;
				player_direction = PLAYER_DIRECTION_RIGHT;
				is_player_walking = TRUE;
			} else {
				velX = 0;
				is_player_walking = FALSE;
				// Force refresh of the animation frame
				frame_skip = 1;
			}

			// A: use
			if (has_orb > 1) {
				has_orb--;
			} else if (has_orb < 0) {
				has_orb++;
			}
			if (keys & J_A) {
				use();
			}

			// B: Attack
			if (keys & J_B) {
				if (!attack_timer && !attack_cool) {
					attack_cool = 20;
					attack_timer = 7;
					attack_direction = player_direction;
					if (player_direction == PLAYER_DIRECTION_RIGHT) {
						attack_offset = 8;
					} else {
						attack_offset = -8;
					}
					move_sprite(SPRITE_ID_PLAYER_SWORD, PLAYER_POS_X + attack_offset, player_y - camera_pos + 8);
					attack();
				}
			}

			if (attack_cool) {
				attack_cool--;
			}
			if (attack_timer) {
				attack_timer--;
				update_sprite_animation(
					SPRITE_ID_PLAYER_SWORD,
					ANIM_PLAYER_ATTACK,
					attack_direction,
					attack_timer);
			}

			if (keys & J_SELECT) {
				disable_animation = !disable_animation;
			}

			//Jumping conditions
			if (grounded) {
				if(!crouch && (keys & J_UP)) {
					velY = -3;
					grounded = FALSE;
					air_timer = 0;
					// frame_skip = 1;
				} else if (keys & J_DOWN) {
					if (!crouch) {
						crouch = TRUE;
						frame_skip = 1;
						player_animation_frame = 0;
					}
				}
			}
			if (!(keys & J_DOWN)) {
				if (crouch) {
					y = to_vram(player_y + velY + 1) - 1;
					// collision top
					for (i = 0; i < 2; i++) {
						x = to_vram(player_x + velX + PLAYER_WIDTH * i);
						get_bkg_tiles(x, y, 1, 1, cell);
						if (blocker(cell[0])) {
							// skip uncrouching if we find at least one blocker
							goto stay_crouched;
						}
					}
					frame_skip = 1;
					crouch = FALSE;
				}
			}

stay_crouched:
			collision();
			aura_y();

			info_msg();

			if (health == 0) {
				start_transition();
			}

			player_x += velX;
			player_y += velY;
			update_background();
			if (crouch) {
				y = player_y + 8 - camera_pos - 1;
			} else {
				y = player_y - camera_pos - 1;
			}
			if (player_direction == PLAYER_DIRECTION_LEFT) {
				move_sprite(SPRITE_ID_PLAYER_ZOPF, PLAYER_POS_X + 8, y);
			} else {
				move_sprite(SPRITE_ID_PLAYER_ZOPF, PLAYER_POS_X - 8, y);
			}
			move_sprite(SPRITE_ID_PLAYER_HEAD, PLAYER_POS_X, player_y - camera_pos - 1);
			move_sprite(SPRITE_ID_PLAYER_BODY, PLAYER_POS_X, player_y + 8 - camera_pos - 1);

			if (!disable_animation) {
				animate_background();
			}

			// damn, fancy shit
			if (player_y < 0x70) {
				if (camera_pos > 0) {
					camera_pos--;
					scroll_bkg(0, -1);
				}
			} else if (player_y < 0xa0) {
				if (camera_pos > 0x30) {
					camera_pos--;
					scroll_bkg(0, -1);
				} else if (camera_pos < 0x30) {
					camera_pos++;
					scroll_bkg(0, 1);
				}
			} else {
				if (camera_pos < 0x70) {
					camera_pos++;
					scroll_bkg(0, 1);
				}
			}

			if (grounded) {
				velY = 0;
				air_timer = 0;
			}

			if (global_timer % 16 == 0) {
				global_animation++;
				global_animation %= 4;
			}

			if ((invulnerable & 0x8) == 0x8) {
				frame_skip = 1;
				set_sprite_tile(SPRITE_ID_PLAYER_ZOPF, 127);
				set_sprite_tile(SPRITE_ID_PLAYER_HEAD, 127);
				set_sprite_tile(SPRITE_ID_PLAYER_BODY, 127);
				continue;
			}
			if (is_player_walking) {
				// skip all 16 frames
				frame_skip--;
				if (!frame_skip) {
					frame_skip = 16;
				} else {
					continue;
				}
			} else {
				player_animation_frame = 0;
			}

			// Update sprites' tiles
			if (crouch) {
				set_sprite_tile(SPRITE_ID_PLAYER_HEAD, 127);
				roll_head(SPRITE_ID_PLAYER_BODY, SPRITE_ROLL, player_animation_frame);
			} else {
				update_sprite_animation(
					SPRITE_ID_PLAYER_ZOPF,
					SPRITE_ZOPF,
					player_direction,
					global_animation % 2
				);
				update_sprite_animation(
					SPRITE_ID_PLAYER_HEAD,
					SPRITE_HEAD,
					player_direction,
					global_animation % 2
				);
				update_sprite_animation(
					SPRITE_ID_PLAYER_BODY,
					SPRITE_BODY,
					player_direction,
					player_animation_frame
				);

			}
			player_animation_frame++;
			player_animation_frame %= 4;
		} else if (scene == 1) {
			// title screen
			animate_background();

			if (transition_d > 0) {
				transition_d--;
			} else {
				if (keys & J_LEFT) {
					SPRITE_ZOPF[0] = SPRITE_ANIM_ZOPF_F[0];
					SPRITE_HEAD[0] = SPRITE_ANIM_HEAD_F[0];
					SPRITE_ZOPF[1] = SPRITE_ANIM_ZOPF_F[1];
					SPRITE_HEAD[1] = SPRITE_ANIM_HEAD_F[1];
					for (i = 0; i < 4; i++) {
						SPRITE_BODY[i] = SPRITE_ANIM_BODY_F[i];
					}
					for (i = 0; i < 16; i++) {
						SPRITE_ROLL[i] = SPRITE_HEAD_ROLL_F[i];
					}
					transition_d = 20;
					set_sprite_tile(SPRITE_ID_PLAYER_ZOPF, 19);
					set_sprite_tile(SPRITE_ID_PLAYER_HEAD, 20);
					set_sprite_tile(SPRITE_ID_PLAYER_BODY, 22);

					for (i = 0; i < 3; i++) {
						bg_animations[10].frames[i] = 0;
						bg_animations[11].frames[i] = bg_animations[11].state;
					}
				} else if (keys & J_RIGHT) {
					SPRITE_ZOPF[0] = SPRITE_ANIM_ZOPF_M[0];
					SPRITE_HEAD[0] = SPRITE_ANIM_HEAD_M[0];
					SPRITE_ZOPF[1] = SPRITE_ANIM_ZOPF_M[1];
					SPRITE_HEAD[1] = SPRITE_ANIM_HEAD_M[1];
					for (i = 0; i < 4; i++) {
						SPRITE_BODY[i] = SPRITE_ANIM_BODY_M[i];
					}
					for (i = 0; i < 16; i++) {
						SPRITE_ROLL[i] = SPRITE_HEAD_ROLL_M[i];
					}
					transition_d = 20;
					set_sprite_tile(SPRITE_ID_PLAYER_ZOPF, 127);
					set_sprite_tile(SPRITE_ID_PLAYER_HEAD, 0);
					set_sprite_tile(SPRITE_ID_PLAYER_BODY, 2);

					for (i = 0; i < 3; i++) {
						bg_animations[11].frames[i] = 0;
						bg_animations[10].frames[i] = bg_animations[10].state;
					}
				}
			}

			if (keys & J_START) {
				start_game();
			}
		}
	}
}
