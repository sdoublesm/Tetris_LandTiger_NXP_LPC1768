#ifndef TETRIS_H
#define TETRIS_H

#include "include.h"
#include "common.h"
#include "tetromino.h"

#define PLAYING 1
#define PAUSED 0
#define GAMEOVER -1

#define ROWS 20
#define COLS 10
#define X_OFFSET 1
#define Y_OFFSET 1
#define BLOCK_SIZE 14

extern volatile int GameState;
extern volatile int startup;
extern volatile int full_redraw_needed;

extern Tetromino current_piece;
extern uint32_t current_speed; // [new] for extrapoint2
extern uint32_t last_speed;

extern uint16_t field[ROWS][COLS];
extern unsigned int current_score;
extern unsigned int high_score;
extern unsigned int lines_count;
extern volatile int message_timer;
extern volatile int reset_music_flag;

// --- POWERUPS ---
#define PWR_HALFCLEAR_ID 100 
#define PWR_SLOW_ID      101
extern volatile int slow_down_active;
extern volatile int slow_down_timer;

/*++++++++++++++++++++++++++ GRAPHIC ++++++++++++++++++++++++++ */ 
void GUI_Init(void);
void GUI_ShowStartup(void);
void GUI_UpdateScores(void);
void DrawSquare(uint16_t x_grid, uint16_t y_grid, uint16_t color);
void DisplayTetromino(Tetromino t, uint16_t color);
void RedrawField(void);
void ResetInfoText(void);
/*++++++++++++++++++++++++++ POWERUP LOGIC +++++++++++++++++++++ */
void SpawnPowerup(void);
void ActivateSlowDown(void);
void ActivateHalfClear(void);
/*++++++++++++++++++++++++++ MALUS LOGIC +++++++++++++++++++++ */
void ApplyRandomMalus(void);
/*+++++++++++++++++++++++ GAME LOGIC +++++++++++++++++++++++++++ */ 
int CheckCollision(int next_x, int next_y, uint16_t shape[4][4]);
void LockPieceAndScore(void);
void CheckAndClearLines(void);
void GenerateNewTetromino(void);
void AttemptRotation(void);

#endif