/*
 *  ======== gtz.c ========
 */    

#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gtz.h"



void clk_SWI_Generate_DTMF(UArg arg0);
void clk_SWI_GTZ_0697Hz(UArg arg0);

extern void task0_dtmfGen(void);
extern void task1_dtmfDetect(void);

extern char digit;
extern int sample, mag1, mag2, freq1, freq2, gtz_out[8];
extern short coef[8];



/*
 *  ======== main ========
 */
void main(void)
{


	System_printf("\n I am in main :\n");
	System_flush();
	/* Create a Clock Instance */
    Clock_Params clkParams;

    /* Initialise Clock Instance with time period and timeout (system ticks) */
    Clock_Params_init(&clkParams);
    clkParams.period = 1;
    clkParams.startFlag = TRUE;

    /* Instantiate ISR for tone generation  */
	Clock_create(clk_SWI_Generate_DTMF, TIMEOUT, &clkParams, NULL);

    /* Instantiate 8 parallel ISRs for each of the eight Goertzel coefficients */
	Clock_create(clk_SWI_GTZ_0697Hz, TIMEOUT, &clkParams, NULL);

	mag1 = 32768.0; mag2 = 32768.0; freq1 = 697; // I am setting freq1 = 697Hz to test my GTZ algorithm with one frequency.

	/* Start SYS_BIOS */
    BIOS_start();
}

/*
 *  ====== clk0Fxn =====
 *  Dual-Tone Generation
 *  ====================
 */
void clk_SWI_Generate_DTMF(UArg arg0)
{
	static int tick;


	tick = Clock_getTicks();

//	sample = (int) 32768.0*sin(2.0*PI*freq1*TICK_PERIOD*tick) + 32768.0*sin(2.0*PI*freq2*TICK_PERIOD*tick);
	sample = (int) 32768.0*sin(2.0*PI*freq1*TICK_PERIOD*tick) + 32768.0*sin(2.0*PI*0*TICK_PERIOD*tick);
	sample = sample >>12;
}

/*
 *  ====== clk_SWI_GTZ =====
 *  gtz sweep
 *  ====================
 */
void clk_SWI_GTZ_0697Hz(UArg arg0)
{
   	static int N = 0;
   	static int Goertzel_Value = 0;

   	static short delay;
   	static short delay_1 = 0;
   	static short delay_2 = 0;

   	int prod1, prod2, prod3;

   	short input, coef_1;

   	// Increment the sample count
   	//N++;

   	// Get the input sample
   	input = (short)sample;
    //input = input <<1;
   	// Get the Goertzel coefficient for the first frequency (697 Hz)
   	coef_1 = coef[0];

   	// Calculate the Goertzel algorithm
   	prod1 = (delay_1*coef_1)>>14;
   	delay = input + (short)prod1 - delay_2;

   	// Update the delay values
   	delay_2 = delay_1;
   	delay_1 = delay;
    N++;
   	// Check if the required number of samples (N) have been processed
   	if (N==206)
   	{
   	   prod1 = (delay_1 * delay_1);
   	   prod2 = (delay_2 * delay_2);
   	   prod3 = (delay_1 * coef_1)>>14;
   	   prod3 = prod3 * delay_2;
   	   Goertzel_Value = (prod1 + prod2 - prod3)>>15;
   	   Goertzel_Value <<= 8; // Scale up value for sensitivity
   	   gtz_out[0]=Goertzel_Value;
   	   N = 0;
   	   delay_1 = delay_2 = 0;
   	}

}

// to be completed






