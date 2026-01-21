#include "tetris.h"
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>

volatile int GameState = PAUSED;
volatile int startup = 1;
volatile int message_timer = 0;
volatile int reset_music_flag = 0;

Tetromino current_piece;
uint32_t current_speed = 1;
uint32_t last_speed = 1;

uint16_t field[ROWS][COLS] = {0}; // init matrix

// debug matrix  
/*
uint16_t field[ROWS][COLS] = {
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    {Black, Black, Black, Black, Black, Black, Black, Black, Black, Black},
    
    {Green, Black, Green, Red, Black, 100, Green, Black, Green, Red}, 
    {Blue, Blue, 101, Blue, Black, Red, Blue, Blue, Blue, Blue}, 
    {White, White, White, White, Black, White, White, White, White, White}, 
    {Magenta, Magenta, 101, Magenta, Black, Magenta, Magenta, Blue, Magenta, Magenta}, 
    {Yellow, Yellow, Blue, Yellow, Black, Yellow, Yellow, Yellow, Yellow, Yellow}
};
*/



unsigned int current_score = 0;
unsigned int high_score = 0;
unsigned int lines_count = 0;

// [NEW] Variabili Power-up
int lines_since_powerup = 0;    // Contatore per spawnare powerup ogni 5 linee
volatile int slow_down_active = 0;
volatile int slow_down_timer = 0;

/*++++++++++++++++++++++++++ GRAPHIC ++++++++++++++++++++++++++ */ 

void DrawSquare(uint16_t x_grid, uint16_t y_grid, uint16_t color) {
  if (x_grid >= COLS || y_grid >= ROWS) {
    return;
  }

  uint16_t i, j;
  uint16_t x_base = X_OFFSET + (x_grid * BLOCK_SIZE);
  uint16_t y_base = Y_OFFSET + (y_grid * BLOCK_SIZE);

  // per rombo (slow down)
  int center = BLOCK_SIZE / 2;
  int radius = (BLOCK_SIZE / 2) - 2;

  for (i = 0; i < BLOCK_SIZE; i++) {
    for (j = 0; j < BLOCK_SIZE; j++) {
      uint16_t pixel_color = Black;

      // --- HALF CLEAR (x rossa) ---
      if (color == PWR_HALFCLEAR_ID) {
        if (i == j || i == j + 1 ||
          i == (BLOCK_SIZE - 1 - j) || i == (BLOCK_SIZE - 1 - j) + 1) {
          pixel_color = Red;
        } else {
          pixel_color = Black;
        }
      }
      // --- SLOW DOWN (rombo azzurro) ---
      else if (color == PWR_SLOW_ID) {
        int dist = abs((int) i - center) + abs((int) j - center);

        if (dist <= radius) {
          pixel_color = Cyan;
        } else {
          pixel_color = Black;
        }
      }
      
			// --- STANDARD BLOCKS---
      else {
        pixel_color = color;
        if (i == 0 || i == BLOCK_SIZE - 1 || j == 0 || j == BLOCK_SIZE - 1) {
          pixel_color = Black;
        }
      }

      LCD_SetPoint(x_base + i, y_base + j, pixel_color);
    }
  }
}

void DisplayTetromino(Tetromino t, uint16_t color) {
	int row, col;
	for (row = 0; row < 4; row++) {
		for (col = 0; col < 4; col++) {
			if (t.shape[row][col] != 0) {
				if ((t.y + row) >= 0) {
					DrawSquare(t.x + col, t.y + row, color);
				}
			}
		}
	}
}

void GUI_UpdateScores(void) {
	char str[16];
	sprintf(str, "%06d", high_score);
	GUI_Text(160, 50, (uint8_t *)str, White, Black);
	sprintf(str, "%06d", current_score);
	GUI_Text(160, 100, (uint8_t *)str, White, Black);
	sprintf(str, "%02d", lines_count);
	GUI_Text(160, 150, (uint8_t *)str, White, Black);
}

void RedrawField(void) {
	int r, c;
	for (r = 0; r < ROWS; r++) { 
		for (c = 0; c < COLS; c++) { 
			if (field[r][c] != 0) {
				DrawSquare(c, r, field[r][c]);
			} else {
				DrawSquare(c, r, Black);
			}
		}
	}
}

void GUI_ShowStartup(void) {
	LCD_Clear(Black); 
	LCD_DrawLine(0, 0, 239, 0, White);     
	LCD_DrawLine(0, 319, 239, 319, White); 
	LCD_DrawLine(0, 0, 0, 319, White);     
	LCD_DrawLine(239, 0, 239, 319, White); 
	
	GUI_Text(90,  120, (uint8_t *)"T", Red, Black);
	GUI_Text(100, 120, (uint8_t *)"E", Orange, Black);
	GUI_Text(110, 120, (uint8_t *)"T", Yellow, Black);
	GUI_Text(120, 120, (uint8_t *)"R", Green, Black);
	GUI_Text(130, 120, (uint8_t *)"I", Cyan, Black);
	GUI_Text(140, 120, (uint8_t *)"S", Magenta, Black);
	
	GUI_Text(75, 135, (uint8_t *)"on LandTiger", White, Black);
	GUI_Text(30, 230, (uint8_t *)" Press [KEY1] to START ", Black, White);
	GUI_Text(30, 296, (uint8_t *)"(C) 2026 Mirko Tenore", White, Black);
}

void GUI_Init(void) {
	LCD_Clear(Black); 

	LCD_DrawLine(X_OFFSET - 1, Y_OFFSET - 1, X_OFFSET + 140, Y_OFFSET - 1, White); 
	LCD_DrawLine(X_OFFSET - 1, Y_OFFSET + 280, X_OFFSET + 140, Y_OFFSET + 280, White); 
	LCD_DrawLine(X_OFFSET - 1, Y_OFFSET - 1, X_OFFSET - 1, Y_OFFSET + 280, White); 
	LCD_DrawLine(X_OFFSET + 140, Y_OFFSET - 1, X_OFFSET + 140, Y_OFFSET + 280, White); 

	GUI_Text(160, 30, (uint8_t *)"TOP", White, Black);
	GUI_Text(160, 80, (uint8_t *)"SCORE", White, Black);
	GUI_Text(160, 130, (uint8_t *)"LINES", White, Black);
	GUI_Text(2, 285, (uint8_t *)"[KEY1] PAUSE / RESUME", White, Black);
	GUI_Text(2, 300, (uint8_t *)"[KEY2] HARD DROP", White, Black);

	GUI_UpdateScores(); 
	RedrawField();
}

void ResetInfoText(void) {
	if (GameState != PAUSED) {
		GUI_Text(150, 200, (uint8_t *)"           ", Black, Black);
	}
	GUI_Text(150, 220, (uint8_t *)"           ", Black, Black);
}

/*++++++++++++++++++++++++++ POWERUP LOGIC ++++++++++++++++++++++++++ */
void SpawnPowerup(void) {
  int occupied_count = 0;
  int r, c;
  for (r = 0; r < ROWS; r++) {
    for (c = 0; c < COLS; c++) {
      if (field[r][c] != 0 && field[r][c] != PWR_HALFCLEAR_ID && field[r][c] != PWR_SLOW_ID) {
        occupied_count++;
      }
    }
  }

  if (occupied_count == 0){
		return;
	}

  int random_idx = rand() % occupied_count;
  int current_idx = 0;

  int powerup_type = (rand() % 2 == 0) ? PWR_HALFCLEAR_ID : PWR_SLOW_ID;

  for (r = 0; r < ROWS; r++) {
    for (c = 0; c < COLS; c++) {
      if (field[r][c] != 0 && field[r][c] != PWR_HALFCLEAR_ID && field[r][c] != PWR_SLOW_ID) {
        if (current_idx == random_idx) {
          field[r][c] = powerup_type;
          DrawSquare(c, r, powerup_type);
          return;
        }
        current_idx++;
      }
    }
  }
}

void ActivateSlowDown(void) {
  if (current_speed > 1) {
    slow_down_active = 1;
    last_speed = current_speed; 
    current_speed = 1; 
    LPC_TIM0 -> MR0 = 25000000 / current_speed;

		// faccio gestire al RIT i 15secondi di slow down
    // RIT a 50ms -> 15 / 0.05 = 300 ticks
    slow_down_timer = 300;
    GUI_Text(150, 220, (uint8_t * )" SLOWED ", White,Blue);
		message_timer = 300; 
  }
}

void ActivateHalfClear(void) {
  int r, c;
  int min_y = ROWS;
  for (r = 0; r < ROWS; r++) {
    for (c = 0; c < COLS; c++) {
      if (field[r][c] != 0) {
        if (r < min_y) min_y = r;
      }
    }
  }

  if (min_y == ROWS){
		return;
	}

  int total_height = ROWS - min_y;
  int lines_to_remove = total_height / 2;

  if (lines_to_remove == 0){
		return;
	}

  int temp_lines = lines_to_remove;
  while (temp_lines >= 4) {
    current_score += 600;
    temp_lines -= 4;
  }
  if (temp_lines > 0) {
    current_score += (100 * temp_lines);
  }

  int dest_row = ROWS - 1;
  int src_row = ROWS - 1 - lines_to_remove;

  for (; src_row >= 0; src_row--, dest_row--) {
    for (c = 0; c < COLS; c++) {
      field[dest_row][c] = field[src_row][c];
    }
  }
  for (; dest_row >= 0; dest_row--) {
    for (c = 0; c < COLS; c++) {
      field[dest_row][c] = 0;
    }
  }

  // RedrawField();
  GUI_Text(150, 220, (uint8_t * )" HALFCLEAR ", White, Red);
  message_timer = 100;
}

/*++++++++++++++++++++++++++ MALUS LOGIC ++++++++++++++++++++++++++ */

void ApplyRandomMalus(void) {
  int r, c;

  // check game over
  for (c = 0; c < COLS; c++) {
    if (field[0][c] != 0) {
      GameState = GAMEOVER;
      GUI_Text(148, 200, (uint8_t * )" GAME OVER ", Red, White);
      GUI_Text(20, 293, (uint8_t * )" Press [KEY1] to RESTART ", Black, White);
      return; 
    }
  }
	 // shift up
  for (r = 0; r < ROWS - 1; r++) {
    for (c = 0; c < COLS; c++) {
      field[r][c] = field[r + 1][c];
    }
  }

  for (c = 0; c < COLS; c++){
		field[ROWS - 1][c] = 0;
	}

  int blocks_placed = 0;
  while (blocks_placed < 7) {
    int random_col = rand() % COLS;
    if (field[ROWS - 1][random_col] == 0) {
      field[ROWS - 1][random_col] = DarkGrey;
      blocks_placed++;
    }
  }

  GUI_Text(150, 220, (uint8_t * )"  MALUS!  ", White, Red);
  message_timer = 100;
}

/*++++++++++++++++++++++++++ GAME LOGIC ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */ 

int CheckCollision(int next_x, int next_y, uint16_t shape[4][4]) {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (shape[r][c] != 0) {
        int target_x = next_x + c;
        int target_y = next_y + r;
        if (target_x < 0 || target_x >= COLS || target_y >= ROWS) {
          return 1;
        }
        if (target_y >= 0 && field[target_y][target_x] != 0) {
          return 1;
        }
      }
    }
  }
  return 0;
}


void CheckAndClearLines() {
  int cleared = 0;
  int r, c;
  int trigger_half_clear = 0;
  int trigger_slow_down = 0;

  for (r = 0; r < ROWS; r++) {
    int count = 0;
    int has_half_clear = 0;
    int has_slow_down = 0;

    for (c = 0; c < COLS; c++) {
      if (field[r][c] != 0) {
        count++;
        if (field[r][c] == PWR_HALFCLEAR_ID) has_half_clear = 1;
        if (field[r][c] == PWR_SLOW_ID) has_slow_down = 1;
      }
    }

    if (count == COLS) {
      cleared++;
      if (has_half_clear){
				trigger_half_clear = 1;
			}
      if (has_slow_down){
				trigger_slow_down = 1;
			}

      for (int r_move = r; r_move > 0; r_move--) {
        for (int c_mat = 0; c_mat < COLS; c_mat++) {
          field[r_move][c_mat] = field[r_move - 1][c_mat];
        }
      }
      for (int c_mat = 0; c_mat < COLS; c_mat++) {
        field[0][c_mat] = 0;
      }
      r--;
    }
  }

  if (cleared > 0) {
    char str[30];
    if (cleared == 4) {
      current_score += 600;
      sprintf(str, " TETRIS! ");
      GUI_Text(150, 200, (uint8_t * ) str, Green, White);
    } else {
      current_score += (100 * cleared);
      sprintf(str, " +%d LINES ", cleared);
      GUI_Text(150, 200, (uint8_t * ) str, Blue, White);
    }
    message_timer = 100;

    unsigned int old_lines = lines_count;
    lines_count += cleared;

    int trigger_malus = (lines_count / 10) > (old_lines / 10);
    lines_since_powerup += cleared;
    if (lines_since_powerup >= 5) {
      SpawnPowerup();
      lines_since_powerup %= 5;
    }

    if (trigger_slow_down){
			ActivateSlowDown();
		}
    if (trigger_half_clear){
			ActivateHalfClear();
		}

    if (trigger_malus) {
      ApplyRandomMalus();
    }
    RedrawField();
    GUI_UpdateScores();
  }
}


void LockPieceAndScore() {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (current_piece.shape[r][c] != 0) {
        int target_x = current_piece.x + c;
        int target_y = current_piece.y + r;
        if (target_y >= 0 && target_y < ROWS && target_x >= 0 && target_x < COLS) {
          field[target_y][target_x] = tetromino_colors[current_piece.id];
        }
      }
    }
  }

  current_score += 10;
  CheckAndClearLines();
  GUI_UpdateScores();

}

void GenerateNewTetromino(void) {
  int r, c;
  int random_idx = rand() % 7;

  current_piece.id = random_idx;
  current_piece.x = 3;
  current_piece.y = 0;
  current_piece.rotation = 0;

  for (r = 0; r < 4; r++) {
    for (c = 0; c < 4; c++) {
      current_piece.shape[r][c] = tetromino_shapes[random_idx][r][c];
    }
  }

  if (CheckCollision(current_piece.x, current_piece.y, current_piece.shape)) {
    reset_music_flag = 1;
    GameState = GAMEOVER;
    ResetInfoText();
    GUI_Text(148, 200, (uint8_t * )" GAME OVER ", Red, White);

    GUI_Text(2, 285, (uint8_t * )"[KEY1] PAUSE / RESUME", Black, Black);
    GUI_Text(2, 300, (uint8_t * )"[KEY2] HARD DROP", Black, Black);
    GUI_Text(20, 293, (uint8_t * )" Press [KEY1] to RESTART ", Black, White);

    if (current_score > high_score){
			high_score = current_score;
		}

  } else {
    DisplayTetromino(current_piece, tetromino_colors[current_piece.id]);
    LPC_TIM0 -> TC = 0;
    LPC_TIM0 -> IR = 1;

  }
}

void AttemptRotation(void) {
  if (current_piece.id == 1){
		return;
	}

  uint16_t tempShape[4][4];
  int r, c;

  for (r = 0; r < 4; r++) {
    for (c = 0; c < 4; c++) {
      tempShape[r][c] = 0;
    }
  }
  int n = (current_piece.id == 0) ? 4 : 3;
  for (r = 0; r < n; r++) {
    for (c = 0; c < n; c++) {
      tempShape[c][n - 1 - r] = current_piece.shape[r][c];
    }
  }

  if (!CheckCollision(current_piece.x, current_piece.y, tempShape)) {
    DisplayTetromino(current_piece, Black);
    current_piece.rotation = (current_piece.rotation + 1) % 4;
    for (r = 0; r < 4; r++) {
      for (c = 0; c < 4; c++) {
        current_piece.shape[r][c] = tempShape[r][c];
      }
    }
    DisplayTetromino(current_piece, tetromino_colors[current_piece.id]);
  }
}
