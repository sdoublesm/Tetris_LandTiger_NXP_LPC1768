#include "../music/music.h"
#include "../timer/timer.h"

void playNote(NOTE note) {
  if (note.freq != pause) {
    reset_timer(1);
    init_timer(1, note.freq);
    enable_timer(1);
  }
  reset_timer(2);
  init_timer(2, note.duration);
  enable_timer(2);
}

BOOL isNotePlaying(void) {
  return ((LPC_TIM1 -> TCR != 0) || (LPC_TIM2 -> TCR != 0));
}