#ifndef TETRIS_H
#define TETRIS_H

#include "include.h"
#include "common.h"
#include "tetromino.h"

// Costanti di Gioco
#define PLAYING 1
#define PAUSED 0
#define GAMEOVER -1

// Dimensioni Campo
#define ROWS 20
#define COLS 10
#define X_OFFSET 1
#define Y_OFFSET 1
#define BLOCK_SIZE 14

// Variabili Globali Condivise
extern volatile int GameState;
extern volatile int startup;
extern volatile int full_redraw_needed;

extern Tetromino current_piece;
extern uint16_t field[ROWS][COLS];
extern unsigned int current_score;
extern unsigned int high_score;
extern unsigned int lines_count;

// --- Funzioni Grafiche ---
void GUI_Init(void);
void GUI_ShowStartup(void);
void GUI_UpdateScores(void);
void DrawSquare(uint16_t x_grid, uint16_t y_grid, uint16_t color);
void DisplayTetromino(Tetromino t, uint16_t color);
void RedrawField(void);

// --- Funzioni Logiche ---
int CheckCollision(int next_x, int next_y, uint16_t shape[4][4]);
void LockPieceAndScore(void);
void CheckAndClearLines(void);
void GenerateNewTetromino(void);
void AttemptRotation(void);

#endif