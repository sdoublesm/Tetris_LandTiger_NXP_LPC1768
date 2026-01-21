#ifndef TETROMINO_H
#define TETROMINO_H

#include <stdint.h>
#include "include.h"

typedef struct {
	int id;          // index from 0 to 6
	int x, y;        // pos into 20x10 field
	int rotation;    // rotation state (0, 1, 2, 3)
	uint16_t shape[4][4]; // current shape
} Tetromino;

extern const uint16_t tetromino_shapes[7][4][4];
extern const uint16_t tetromino_colors[7];

#endif