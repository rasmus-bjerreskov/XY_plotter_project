/*
 * Plotter.cpp
 *
 *  Created on: 14.10.2020
 *      Author: Santeri Rauhala
 */

#include "Plotter.h"

Plotter::Plotter() {
	offturn = false;

}

Plotter::~Plotter() {
	// TODO Auto-generated destructor stub
}

void switchOffturn() {
	offturn = !offturn;
}

bool getOffturn() {
	return offturn;
}

void Plotter::plotLine(int x0_l, int y0_l, int x1_l, int y1_l, int us)
{
 uint64_t cmp_value;
 // Determine approximate compare value based on clock rate and passed interval
 cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
 // disable timer during configuration
 Chip_RIT_Disable(LPC_RITIMER);

 x0 = x0_l;
 x1 = x1_l;
 y0 = y0_l;
 y1 = y1_l;
 i = 0;
 dx = abs(x1 - x0);
 dy = abs(y1 - y0);

     if (dx > dy) {
         prim1 = x1;
         prim2 = dx;
         prim3 = dy;
         //primaryIo = [Xstep];
         //secondaryIo = [Ystep];
         D = 2*dy - dx;
     }
     else {
         prim1 = y1;
         prim2 = dy;
         prim3 = dx;
         //primaryIo = [Ystep];
         //secondaryIo = [Xstep];
         D = 2*dx - dy;
     }

 // enable automatic clear on when compare value==timer value
 // this makes interrupts trigger periodically
 Chip_RIT_EnableCompClear(LPC_RITIMER);
 // reset the counter
 Chip_RIT_SetCounter(LPC_RITIMER, 0);
 Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
 // start counting
 Chip_RIT_Enable(LPC_RITIMER);
 // Enable the interrupt signal in NVIC (the interrupt controller)
 NVIC_EnableIRQ(RITIMER_IRQn);
 // wait for ISR to tell that we're done
 if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
 // Disable the interrupt signal in NVIC (the interrupt controller)
 NVIC_DisableIRQ(RITIMER_IRQn);
 }
 else {
 // unexpected error
 }
}
