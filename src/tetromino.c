#include "tetromino.h"
#include "tetris.h"
// a tetromino shape is a 4x4 matrix
const uint16_t tetromino_shapes[7][4][4] = {
	{ {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} }, // I (Cyan)
	{ {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} }, // O (Yellow)
	{ {0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} }, // T (Magenta)
	{ {1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} }, // J (Blue)
	{ {0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} }, // L (Orange)
	{ {0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} }, // S (Green)
	{ {1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} }  // Z (Red)
};


const uint16_t tetromino_colors[7] = {Cyan, Yellow, Magenta, Blue, Orange, Green, Red};