/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_adc.c
** Last modified Date:  20184-12-30
** Last Version:        V1.00
** Descriptions:        functions to manage A/D interrupts
** Correlated files:    adc.h
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "LPC17xx.h"
#include "adc.h"
#include "../tetris.h"

//*** <<< Use Configuration Wizard in Context Menu >>> ***
// <h> ADC Exponential Smoothing
//   <e.0> Enabled
//	 <i> 1 Exponential Smoothing Enabled
//	 <i> 0 Exponential Smoothing Disabled
//   </e>
#define ADC_EXP_SMOOTH 1
// <<< end of configuration section >>>

#define SMOOTH_ALPHA 0.125

/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is ready (signal from ADC peripheral)
 *----------------------------------------------------------------------------*/
volatile uint16_t AD_current = 0;

void ADC_IRQHandler(void) {
#if ADC_EXP_SMOOTH
  static uint8_t first = 1;
	
	if(first) {
		first = 1;
		AD_current = ((LPC_ADC->ADGDR>>4) & 0xFFF);
	}
	
	AD_current = SMOOTH_ALPHA*((LPC_ADC->ADGDR>>4) & 0xFFF) + (1-SMOOTH_ALPHA)*AD_current;
#else
	ADC_current = ((LPC_ADC->ADGDR>>4) & 0xFFF);
#endif
	
	// AD_CURRENT : 0xFFF = X : 5
	uint32_t new_speed = 1 + (4 * AD_current) / 0xFFF;
	uint32_t new_MR0 = 25000000 / new_speed;
	if(slow_down_active){
		last_speed = new_speed;
	} else{
		if (new_speed != current_speed) {
			current_speed = new_speed;
			disable_timer(0);
			LPC_TIM0->MR0 = new_MR0;
			if (LPC_TIM0->TC >= new_MR0) {
				LPC_TIM0->TC = 0;
			}
			enable_timer(0);
		}
	}
}
