#include "include.h"
#include "tetris.h"
#include "common.h"

// imported variables 
extern volatile uint8_t joystick_flag;
extern uint32_t pressed_joystick_down;
extern volatile uint8_t btn_flag;

extern volatile uint16_t AD_current; // potentiometer

// *********************** MAIN ********************************************************1**
int main(void) {

	SystemInit();

  // LED_init();
  joystick_init();
  LCD_Initialization();
	ADC_init();

  // BUTTON_init:
	BUTTON_init(BUTTON_1, PRIO_0);
  BUTTON_init(BUTTON_2, PRIO_0);
  BUTTON_init(BUTTON_0, PRIO_3);
	
	/*****************************************
	Timer0: Game (tetrominoes fall)
	Timer1: DAC (notes wave)
	Timer2: Music (end notes)
	*****************************************/
	
	NVIC_SetPriority(TIMER1_IRQn, 0);
	NVIC_SetPriority(TIMER2_IRQn, 1);
	NVIC_SetPriority(TIMER0_IRQn, 2);
	NVIC_SetPriority(RIT_IRQn, 3);

  // --- RIT CONFIGURATION ---
  // RIT WORKS WITH CLOCK = 100MHZ
  // ONE INTERRUPT EVERY 50ms
  uint32_t f_RIT = 100000; // [KHz]
  uint32_t tRIT = RIT_PERIOD_MS; // [ms]
  uint32_t K_RIT = tRIT * f_RIT;

  init_RIT(K_RIT);
  enable_RIT();
	
  // --- TIMER0 CONFIGURATION ---
  // to handle tetrominoes fall (every 1s)
  uint32_t f_timer0 = 25000000; // [Hz]
  uint32_t t0 = 1;
  uint32_t K_timer0 = t0 * f_timer0/current_speed;

  init_timer(0, K_timer0);
	
	GUI_ShowStartup();
  enable_timer(0);
	
	// --- CONFIGURAZIONE AUDIO (DAC) ---
	LPC_PINCON->PINSEL1 |= (1<<21);
	LPC_PINCON->PINSEL1 &= ~(1<<20);
	LPC_GPIO0->FIODIR |= (1<<26);
	
  while (1) {
		
		// ------------- PLAYING STATE ---------------------------------------

    if (GameState == PLAYING) {

      // --- move to DX ---
      if (joystick_flag & FLAG_JOYSTICK_RIGHT) {
        joystick_flag &= ~FLAG_JOYSTICK_RIGHT;
				NVIC_DisableIRQ(TIMER0_IRQn);
        if (!CheckCollision(current_piece.x + 1, current_piece.y, current_piece.shape)) {
          DisplayTetromino(current_piece, Black); // delete prevous one
          current_piece.x++;
          DisplayTetromino(current_piece, tetromino_colors[current_piece.id]); 
        }
				NVIC_EnableIRQ(TIMER0_IRQn);
      }
      // --- move to SX ---
      if (joystick_flag & FLAG_JOYSTICK_LEFT) {
        joystick_flag &= ~FLAG_JOYSTICK_LEFT;
				NVIC_DisableIRQ(TIMER0_IRQn);
        if (!CheckCollision(current_piece.x - 1, current_piece.y, current_piece.shape)) {
          DisplayTetromino(current_piece, Black);
          current_piece.x--;
          DisplayTetromino(current_piece, tetromino_colors[current_piece.id]);
        }
				NVIC_EnableIRQ(TIMER0_IRQn);
      }
      // --- rotate (UP) ---
      if (joystick_flag & FLAG_JOYSTICK_UP) {
        joystick_flag &= ~FLAG_JOYSTICK_UP;
				NVIC_DisableIRQ(TIMER0_IRQn);
        AttemptRotation();
				NVIC_EnableIRQ(TIMER0_IRQn);
      }
			// --- increase fall speed (DOWN) ---
      static int fast_mode_active = 0; 
      if (pressed_joystick_down > 0) {
        if (joystick_flag & FLAG_JOYSTICK_DOWN) {
            joystick_flag &= ~FLAG_JOYSTICK_DOWN;
        }
        if (fast_mode_active == 0) {
					fast_mode_active = 1; 
					NVIC_DisableIRQ(ADC_IRQn);
					LPC_TIM0->MR0 = 25000000/(current_speed*2);
				
					if(LPC_TIM0->TC >= 25000000/(current_speed*2)){
						LPC_TIM0->TC = 0; 
					}
        }
      }
      else { 
        if (fast_mode_active == 1) {
					fast_mode_active = 0; 
					if(current_speed == 0){
						current_speed = 1;
					}
					
					uint32_t normal_MR0 = 25000000 / current_speed;
					LPC_TIM0->MR0 = normal_MR0;
					
					if (LPC_TIM0->TC >= normal_MR0) {
							LPC_TIM0->TC = normal_MR0 - 1;
					}
					
					NVIC_EnableIRQ(ADC_IRQn); 
        }
      }
			
			// ----------- buttons -----------
			
			// --- PAUSE (KEY1) ---
      if (btn_flag & FLAG_BUTTON_1_SHORT) {
        NVIC_DisableIRQ(TIMER0_IRQn);
        btn_flag &= ~FLAG_BUTTON_1_SHORT;
        GameState = PAUSED;
				GUI_Text(150, 200, (uint8_t *)"            ", Black, Black);
        GUI_Text(160, 200, (uint8_t * )" PAUSE ", Red, White);
      }
      // --- HARD DROP (KEY2) ---
      if ((btn_flag & FLAG_BUTTON_2_SHORT) && GameState != PAUSED) {
				NVIC_DisableIRQ(TIMER0_IRQn);	// in order not to have race conditions
        NVIC_DisableIRQ(RIT_IRQn);
				btn_flag &= ~FLAG_BUTTON_2_SHORT;
        DisplayTetromino(current_piece, Black);
       
				while (!CheckCollision(current_piece.x, current_piece.y + 1, current_piece.shape)) {
          current_piece.y++;
        }
        DisplayTetromino(current_piece, tetromino_colors[current_piece.id]); // draw into the final position
        LockPieceAndScore();
        GenerateNewTetromino();
        LPC_TIM0 -> TC = 0;
				LPC_TIM0->IR = 1; // remove pending interrupts
        NVIC_EnableIRQ(TIMER0_IRQn); // re-enable timer0
				NVIC_EnableIRQ(RIT_IRQn);
			}

		// ------------- PAUSED STATE ---------------------------------------
    } else if (GameState == PAUSED) {
      if (btn_flag & FLAG_BUTTON_1_SHORT) {
        btn_flag &= ~FLAG_BUTTON_1_SHORT;

        GameState = PLAYING;
        // -- STARTUP --
        if (startup == 1) {
          srand(LPC_RIT -> RICOUNTER);
          GUI_Init();
          startup = 0;
          GenerateNewTetromino();
        }
        NVIC_EnableIRQ(TIMER0_IRQn);
				GUI_Text(150, 200, (uint8_t *)"            ", Black, Black);
      }
			
		// ------------- GAMEOVER STATE ---------------------------------------
    } else if (GameState == GAMEOVER) {
      if (btn_flag & FLAG_BUTTON_1_SHORT) {
        btn_flag &= ~FLAG_BUTTON_1_SHORT;
        current_score = 0;
        lines_count = 0;
        slow_down_active = 0;
        // malus_delay_timer = 0;
				for (int r = 0; r < ROWS; r++) {
					for (int c = 0; c < COLS; c++) {
						field[r][c] = 0;
					}
        }
				reset_music_flag=1;
        GUI_Init();
        GenerateNewTetromino();
        GameState = PLAYING;
      }
    }
  }
}
