//////////////////////////////////////////////////////////////////////////////
// *
// * Predmetni projekat iz predmeta OAiS DSP 2
// * Godina: 2017
// *
// * Zadatak: Ekvalizacija audio signala
// * Autor:
// *                                                                          
// *                                                                          
/////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5535.h"
#include "ezdsp5535_i2c.h"
#include "aic3204.h"
#include "ezdsp5535_aic3204_dma.h"
#include "ezdsp5535_i2s.h"
#include "ezdsp5535_sar.h"
#include "print_number.h"
#include "math.h"

#include "iir.h"
#include "processing.h"

/* Frekvencija odabiranja */
#define SAMPLE_RATE 8000L

#define PI 3.14159265
#define KONST 2*PI/8000

/* Niz za smestanje ulaznih i izlaznih odbiraka */
#pragma DATA_ALIGN(sampleBufferL,4)
Int16 sampleBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(sampleBufferR,4)
Int16 sampleBufferR[AUDIO_IO_SIZE];

/*
float alpha[] = {0.3, -0.3, 0.7};
float beta = 0.0;
Int16 ka[2] = {8192, 24576};
Int16 output1[AUDIO_IO_SIZE];
Int16 output2[AUDIO_IO_SIZE];
Int16 xHistoryLPTest[2];
Int16 yHistoryLPTest[2];
Int16 xHistoryHPTest[2];
Int16 yHistoryHPTest[2];
Int16 xHistoryPTest[3];
Int16 yHistoryPTest[3];
Int16 xHistoryLPTest1[2];
Int16 yHistoryLPTest1[2];
Int16 xHistoryHPTest1[2];
Int16 yHistoryHPTest1[2];
Int16 xHistoryPTest1[3];
Int16 yHistoryPTest1[3];
Int16 shelvingCoeffLPTest[4];
Int16 shelvingCoeffHPTest[4];
Int16 peekCoeffTest[6];
Int16 delta[AUDIO_IO_SIZE];
*/

Uint16 pressedNow;
Uint16 pressedPrevious;
               	/*	a-lp	b-p1	a-p1	b-p2	a-p2	a-hp*/
float omega[6] = {	220, 	520, 	110, 	2905, 	1355, 	5000};
float parameters[6];

Int16 k_value[11] = {0, 3274, 6551, 9828, 13105, 16382, 19659, 22936, 26213, 29490, 32767};
int k_index[4] = {7, 7, 7, 7};
Int podopseg = 1;

/* History buffers for low-pass, high-pass and peek filters */
Int16 xHistoryLP[2];
Int16 yHistoryLP[2];
Int16 xHistoryHP[2];
Int16 yHistoryHP[2];
Int16 xHistoryP1[3];
Int16 yHistoryP1[3];
Int16 xHistoryP2[3];
Int16 yHistoryP2[3];

/* Coefficients of low-pass, high-pass and peek filters */
Int16 shelvingCoeffLP[4];
Int16 shelvingCoeffHP[4];
Int16 peekCoeff1[6];
Int16 peekCoeff2[6];

/* Output buffers in equilizer */
Int16 outputBand1[AUDIO_IO_SIZE];
Int16 outputBand2[AUDIO_IO_SIZE];
Int16 outputBand3[AUDIO_IO_SIZE];
Int16 outputBand4[AUDIO_IO_SIZE];


void main( void )
{
	int i;

    /* Inicijalizaija razvojne ploce */
    EZDSP5535_init( );

    /* Inicijalizacija kontrolera za ocitavanje vrednosti pritisnutog dugmeta*/
    EZDSP5535_SAR_init();

    /* Inicijalizacija LCD kontrolera */
    //initPrintNumber();

	printf("\n Ekvalizacija audio signala \n\n");

    /* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
    aic3204_hardware_init();

/*
	aic3204_set_input_filename("../Multi_Tone.pcm");
	aic3204_set_output_filename("../output.pcm");
*/
    /* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

    aic3204_dma_init();

    /* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
    set_sampling_frequency_and_gain(SAMPLE_RATE, 0);

    for(i=0; i<6; i++)
    {
    	omega[i] = KONST*omega[i];
    }
    /* omega[] = {0.172788	0.408407	0.086394	2.281582	1.064215	3.926991}; */


/*
    delta[0] = 10000;
    for(i = 1; i < AUDIO_IO_SIZE; i++) {
    	delta[i] = 0;
    }
*/

    for(i = 0; i < 2; i++) {
    	xHistoryLP[i] = 0;
    	xHistoryHP[i] = 0;
    	yHistoryLP[i] = 0;
    	yHistoryHP[i] = 0;
/*
    	xHistoryLPTest[i] = 0;
    	xHistoryHPTest[i] = 0;
    	yHistoryLPTest[i] = 0;
    	yHistoryHPTest[i] = 0;
    	xHistoryLPTest1[i] = 0;
    	xHistoryHPTest1[i] = 0;
    	yHistoryLPTest1[i] = 0;
    	yHistoryHPTest1[i] = 0;
*/
    }

    for(i = 0; i < 3; i++) {
    	xHistoryP1[i] = 0;
    	yHistoryP1[i] = 0;
    	xHistoryP2[i] = 0;
    	yHistoryP2[i] = 0;
/*
    	xHistoryPTest[i] = 0;
    	yHistoryPTest[i] = 0;
*/
	}

    calculateParameters(parameters, omega);
    /*parameters[] = {0.840588 (1.189643), 0.917755, 0.917134 (1.090353), -0.652429, 0.258850 (3.863237), -0.414213 (-2.414214)} */

    calculateShelvingCoeff(parameters[0], shelvingCoeffLP);
    calculateShelvingCoeff(parameters[5], shelvingCoeffHP);
    calculatePeekCoeff(parameters[2], parameters[1], peekCoeff1);
    calculatePeekCoeff(parameters[4], parameters[3], peekCoeff2);

/*
    calculateShelvingCoeff(alpha[0], shelvingCoeffLPTest);
    calculateShelvingCoeff(alpha[1], shelvingCoeffHPTest);
    calculatePeekCoeff(alpha[2], beta, peekCoeffTest);
*/

    clearLCD();
    printOnLCD(podopseg, k_index[podopseg - 1]);
	
    while(1)
    {
    	aic3204_read_block(sampleBufferL, sampleBufferR);

    	pressedNow = EZDSP5535_SAR_getKey();
    	if (pressedNow != pressedPrevious && pressedNow != NoKey)
    	{

    		switch (pressedNow)
    		{
    			case SW1:
    				podopseg += 1;
    				if (podopseg == 5)
    					podopseg = 1;
    				break;
    			case SW2:
    				k_index[podopseg - 1] -= 1;
    				if (k_index[podopseg - 1] == -1)
    					k_index[podopseg - 1] = 10;
    				break;
    			case SW12:
    				break;
    			case NoKey:
    				break;
    			default:
    				break;
    		}
    		clearLCD();
    		printOnLCD(podopseg, k_index[podopseg - 1]);
			for (i=0; i<4; i++)
    		{
    			printf("%d\t", k_value[k_index[i]]);
    		}
    		printf("\n");
    	}

    	pressedPrevious = pressedNow;

/*
    	for(i=0; i<AUDIO_IO_SIZE; i++)
    	{
    		output1[i] = shelvingLP(delta[i], shelvingCoeffLPTest, xHistoryLPTest, yHistoryLPTest, ka[0]);
    		output2[i] = shelvingLP(delta[i], shelvingCoeffLPTest, xHistoryLPTest1, yHistoryLPTest1, ka[1]);
    	}

    	for(i=0; i<AUDIO_IO_SIZE; i++)
    	{
    		output1[i] = shelvingHP(delta[i], shelvingCoeffHPTest, xHistoryHPTest, yHistoryHPTest, ka[0]);
    		output2[i] = shelvingHP(delta[i], shelvingCoeffHPTest, xHistoryHPTest1, yHistoryHPTest1, ka[1]);
    	}

    	for(i=0; i<AUDIO_IO_SIZE; i++)
    	{
    		output1[i] = shelvingPeek(delta[i], peekCoeffTest, xHistoryPTest, yHistoryPTest, ka[0]);
    		output2[i] = shelvingPeek(delta[i], peekCoeffTest, xHistoryPTest1, yHistoryPTest1, ka[1]);
    	}
*/

    	for(i=0; i<AUDIO_IO_SIZE; i++)
    	{
    		outputBand1[i] = shelvingLP(sampleBufferL[i], shelvingCoeffLP, xHistoryLP, yHistoryLP, k_value[k_index[0]]);
    	}

    	for(i=0; i<AUDIO_IO_SIZE; i++)
    	{
    		outputBand2[i] = shelvingPeek(outputBand1[i], peekCoeff1, xHistoryP1, yHistoryP1, k_value[k_index[1]]);
    	}

    	for(i=0; i<AUDIO_IO_SIZE; i++)
    	{
    		outputBand3[i] = shelvingPeek(outputBand2[i], peekCoeff2, xHistoryP2, yHistoryP2, k_value[k_index[2]]);
    	}

    	for(i=0; i<AUDIO_IO_SIZE; i++)
    	{
    		outputBand4[i] = shelvingHP(outputBand3[i], shelvingCoeffHP, xHistoryHP, yHistoryHP, k_value[k_index[3]]);
    	}

		aic3204_write_block(outputBand4, outputBand4);
	}

	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}
