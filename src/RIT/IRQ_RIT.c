#include "RIT.h"

#include "../led/led.h"

#include "../common.h"

#include "../timer/timer.h"

#include "../joystick/joystick.h"

#include "../button_EXINT/button.h"

#include "../adc/adc.h"

#include "../music/music.h"

#include "../tetris.h"

#define SHORT_PRESS_COUNT (SHORT_PRESS_MS / RIT_PERIOD_MS)
#define LONG_PRESS_COUNT (LONG_PRESS_MS / RIT_PERIOD_MS)

volatile uint32_t pressed_button_0 = 0;
volatile uint32_t pressed_button_1 = 0;
volatile uint32_t pressed_button_2 = 0;

volatile uint32_t pressed_joystick_up = 0;
volatile uint32_t pressed_joystick_left = 0;
volatile uint32_t pressed_joystick_right = 0;
volatile uint32_t pressed_joystick_down = 0;
volatile uint32_t pressed_joystick_select = 0;

volatile uint8_t joystick_flag = 0;
volatile uint8_t btn_flag = 0;

#define UPTICKS 1
NOTE song[] = {
	{e4, time_semiminima},  
	{b3, time_croma},  
	{c4, time_croma},  
	{d4, time_semiminima},  
	{c4, time_croma},  
	{b3, time_croma},  
	
	{a3, time_semiminima},  
	{a3, time_croma},  
	{c4, time_croma},  
	{e4, time_semiminima},  
	{d4, time_croma},  
	{c4, time_croma},  
	
	{b3, time_semiminima},  
	{b3, time_croma},  
	{c4, time_croma},  
	{d4, time_semiminima},  
	{e4, time_semiminima},  
	
	{c4, time_semiminima},  
	{a3, time_semiminima},  
	{a3, time_semiminima},  
	{pause, time_croma}, 
	
	{d4, time_semiminima},
	{d4, time_croma},
	{f4, time_croma}, 
	{a4, time_semiminima}, 
	{g4, time_croma}, 
	{f4, time_croma}, 
	
	{e4, time_semiminima},
	{e4, time_croma},
	{c4, time_croma}, 
	{e4, time_semiminima}, 
	{d4, time_croma}, 
	{c4, time_croma}, 
	
	{b3, time_semiminima}, 
	{b3, time_croma}, 
	{c4, time_croma}, 
	{d4, time_semiminima}, 
	{e4, time_semiminima}, 
	
	{c4, time_semiminima}, 
	{a3, time_semiminima}, 
	{a3, time_semiminima},
	{pause, time_semiminima}
};

NOTE game_over_melody[] = {
	{c4, time_croma},
	{b3, time_croma},
	{a3, time_croma},
	{a3b, time_croma},
	{g3, time_croma},
	{f3, time_croma},
	{e3, time_croma},
	{d3, time_croma},
	{c3b, time_croma},
	{c3, time_semibreve}
};

void RIT_IRQHandler(void) {

  // ************* MUSIC ************************
  static unsigned int currentNote = 0;
  static int ticks = 0;

  if (reset_music_flag == 1) {
    currentNote = 0;
    ticks = 0;
    reset_music_flag = 0;
  }

  if (!isNotePlaying()) {
    ticks++;
    if (ticks >= UPTICKS) {
      ticks = 0;

      if (GameState == PLAYING) {
        if (currentNote < (sizeof(song) / sizeof(song[0]))) {
          playNote(song[currentNote++]);
        } else {
          currentNote = 0;
          playNote(song[0]);
          currentNote++;
        }
      } else if (GameState == GAMEOVER) {
        if (currentNote < (sizeof(game_over_melody) / sizeof(game_over_melody[0]))) {
          playNote(game_over_melody[currentNote++]);
        }
      }
    }
  }

  // ************* SLOW DOWN ************************
  if (slow_down_active) {
    slow_down_timer--;
    if (slow_down_timer <= 0) {
      slow_down_active = 0;
      current_speed = (last_speed > 0) ? last_speed : 1;
      LPC_TIM0 -> MR0 = 25000000 / current_speed;
      if (LPC_TIM0 -> TC >= 25000000 / current_speed) {
        LPC_TIM0 -> TC = 25000000 / current_speed - 1;
      }
      GUI_Text(150, 220, (uint8_t * )"       ", Black, Black);
    }
  }

  if (GameState != PAUSED && message_timer > 0) {
    message_timer--;
    if (message_timer == 0) {
      ResetInfoText();
    }
  }

  // -------------------------------
  // JOYSTICK UP
  // -------------------------------
  if (joystick_check_dir(JOYSTICK_UP)) {
    pressed_joystick_up++;
    if (pressed_joystick_up == 1) {
      joystick_flag |= FLAG_JOYSTICK_UP;
    }
  } else
    pressed_joystick_up = 0;

  // -------------------------------
  // JOYSTICK DOWN
  // -------------------------------
  if (joystick_check_dir(JOYSTICK_DOWN)) {
    pressed_joystick_down++;
    if (pressed_joystick_down == 1) {
      joystick_flag |= FLAG_JOYSTICK_DOWN;
    }
  } else
    pressed_joystick_down = 0;

  // -------------------------------
  // JOYSTICK LEFT
  // -------------------------------
  if (joystick_check_dir(JOYSTICK_LEFT)) {
    pressed_joystick_left++;
    if (pressed_joystick_left == 1) {
      joystick_flag |= FLAG_JOYSTICK_LEFT;
    }
  } else
    pressed_joystick_left = 0;

  // -------------------------------
  // JOYSTICK RIGHT
  // -------------------------------
  if (joystick_check_dir(JOYSTICK_RIGHT)) {
    pressed_joystick_right++;
    if (pressed_joystick_right == 1) {
      joystick_flag |= FLAG_JOYSTICK_RIGHT;
    }
  } else
    pressed_joystick_right = 0;

  // -------------------------------
  // JOYSTICK SELECT
  // -------------------------------
  if (joystick_check_dir(JOYSTICK_PRESS)) {
    pressed_joystick_select++;
    if (pressed_joystick_select == 1) {
      joystick_flag |= FLAG_JOYSTICK_SELECT;
    }
  } else
    pressed_joystick_select = 0;

  // ************* BUTTONS ************************

  // --- KEY0 ---
  if (pressed_button_0 != 0) {
    if ((LPC_GPIO2 -> FIOPIN & (1 << 10)) == 0) {
      pressed_button_0++;
      if (pressed_button_0 == 2)
        btn_flag |= FLAG_BUTTON_0_SHORT;
    } else { // released
      pressed_button_0 = 0;
      NVIC_ClearPendingIRQ(EINT0_IRQn);
      NVIC_EnableIRQ(EINT0_IRQn);
      LPC_PINCON -> PINSEL4 |= (1 << 20);
    }
  }

  // --- KEY1 ---
  if (pressed_button_1 != 0) {
    if ((LPC_GPIO2 -> FIOPIN & (1 << 11)) == 0) {
      pressed_button_1++;
      if (pressed_button_1 == 2) {
        btn_flag |= FLAG_BUTTON_1_SHORT;
      }
    } else {
      pressed_button_1 = 0;
      NVIC_ClearPendingIRQ(EINT1_IRQn);
      NVIC_EnableIRQ(EINT1_IRQn);
      LPC_PINCON -> PINSEL4 |= (1 << 22);
    }
  }

  // --- KEY2 ---
  if (pressed_button_2 != 0) {
    if ((LPC_GPIO2 -> FIOPIN & (1 << 12)) == 0) {
      pressed_button_2++;
      if (pressed_button_2 == 2) {
        btn_flag |= FLAG_BUTTON_2_SHORT;
      }
    } else {
      pressed_button_2 = 0;
      NVIC_ClearPendingIRQ(EINT2_IRQn);
      NVIC_EnableIRQ(EINT2_IRQn);
      LPC_PINCON -> PINSEL4 |= (1 << 24);
    }
  }

  // -------------------------------
  // ADC Conversion
  // -------------------------------

  ADC_start_conversion();

  LPC_RIT -> RICTRL |= 0x1;
}
/******************************************************************************
 **                            End Of File
 ******************************************************************************/