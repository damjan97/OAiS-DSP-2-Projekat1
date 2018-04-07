#include "processing.h"
#include "iir.h"
#include "math.h"
#include "print_number.h"


void calculateShelvingCoeff(float c_alpha, Int16* output)
{
	/* Your code here */
	Int32 tmp;

	tmp = c_alpha * 32768;
	if(tmp > 32767)
		output[0] = 32767;
	else
		output[0] = (Int16) tmp;
	output[1] = -32768;
	output[2] = 32767;
	tmp = -tmp;
	if(tmp > 32767)
		output[3] = 32767;
	else
		output[3] = (Int16) tmp;
}

void calculatePeekCoeff(float c_alpha, float c_beta, Int16* output)
{
	/* Your code here */
	Int32 tmp;

	tmp = c_alpha * 32768;
	if(tmp > 32767)
		output[0] = 32767;
	else
		output[0] = (Int16) tmp;
	tmp = ((Int32)(-(c_beta*(1 + c_alpha)*32768))) >> 1;
	if(tmp > 32767)
		output[1] = 32767;
	else
		output[1] = (Int16) tmp;
	output[2] = 32767;
	output[3] = output[2];
	output[4] = output[1];
	output[5] = output[0];
}

Int16 shelvingHP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{
	/* Your code here */
	Int16 filterOutput, tmp3;
	Int32 output, tmp1, tmp2;

	filterOutput = first_order_IIR(input, coeff, x_history, y_history);
	tmp1 = ((input / 2) + (filterOutput / 2));
	if(tmp1 > 32767)
		tmp1 = 32767;
	tmp2 = ((input / 2) - (filterOutput / 2));
	if(tmp2 > 32767)
		tmp2 = 32767;
	tmp3 = _smpy((Int16)tmp1, k);
	output = tmp2 + tmp3;
	if(output > 32767)
		return 32767;
	else if(output < -32768)
		return -32768;
	else
		return (Int16) output;
}

Int16 shelvingLP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{
	/* Your code here */
	Int16 filterOutput, tmp3;
	Int32 output, tmp1, tmp2;

	filterOutput = first_order_IIR(input, coeff, x_history, y_history);
	tmp1 = ((input / 2) + (filterOutput / 2));
	if(tmp1 > 32767)
		tmp1 = 32767;
	tmp2 = ((input / 2) - (filterOutput / 2));
	if(tmp2 > 32767)
		tmp2 = 32767;
	tmp3 = _smpy((Int16)tmp2, k);
	output = tmp1 + tmp3;
	if(output > 32767)
		return 32767;
	else if(output < -32768)
		return -32768;
	else
		return (Int16) output;
}

Int16 shelvingPeek(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history, Int16 k)
{
	/* Your code here */
	Int16 filterOutput, tmp3;
	Int32 output, tmp1, tmp2;

	filterOutput = second_order_IIR(input, coeff, x_history, y_history);
	tmp1 = ((input >> 1) + (filterOutput >> 1));
	if(tmp1 > 32767)
		tmp1 = 32767;
	tmp2 = ((input >> 1) - (filterOutput >> 1));
	if(tmp2 > 32767)
		tmp2 = 32767;
	tmp3 = _smpy((Int16)tmp2, k);
	output = tmp1 + tmp3;
	if(output > 32767)
		return 32767;
	else if(output < -32768)
		return -32768;
	else
		return (Int16) output;
}

void calculateParameters(float* parameters, float* omega)
{
	/* 0 -> alpha(lp)	1 -> beta(peek1)	2 -> alpha(peek1)	3 -> beta(peek2)	4 -> alpha(peek2)	5 -> alpha(hp) */
	float tmp1, tmp2;

	/* alfa za niskopropusni filtar*/
	tmp1 = (2+sqrt(4 - 4*pow(cos(omega[0]), 2)))/(2*cos(omega[0]));
	tmp2 = (2-sqrt(4 - 4*pow(cos(omega[0]), 2)))/(2*cos(omega[0]));
	if(tmp1 >= 0 && tmp1 <= 1)
		parameters[0] = tmp1;
	else
		parameters[0] = tmp2;

	/* beta za peek1 */
	parameters[1] = cos(omega[1]);

	/* alfa za peek1*/
	tmp1 = (2+sqrt(4 - 4*pow(cos(omega[2]), 2)))/(2*cos(omega[2]));
	tmp2 = (2-sqrt(4 - 4*pow(cos(omega[2]), 2)))/(2*cos(omega[2]));
	if(tmp1 >= -1 && tmp1 <= 1)
		parameters[2] = tmp1;
	else
		parameters[2] = tmp2;

	/* beta za peek2 */
	parameters[3] = cos(omega[3]);

	/* alfa za peek2*/
	tmp1 = (2+sqrt(4 - 4*pow(cos(omega[4]), 2)))/(2*cos(omega[4]));
	tmp2 = (2-sqrt(4 - 4*pow(cos(omega[4]), 2)))/(2*cos(omega[4]));
	if(tmp1 >= -1 && tmp1 <= 1)
		parameters[4] = tmp1;
	else
		parameters[4] = tmp2;

	/* alfa za visokopropusni filtar*/
	tmp1 = (2+sqrt(4 - 4*pow(cos(omega[5]), 2)))/(2*cos(omega[5]));
	tmp2 = (2-sqrt(4 - 4*pow(cos(omega[5]), 2)))/(2*cos(omega[5]));
	if(tmp1 >= -1 && tmp1 <= 0)
		parameters[5] = tmp1;
	else
		parameters[5] = tmp2;
}

void printOnLCD(int b, int k)
{
	char tmp = b;
    printChar('B');
    printChar('=');
    printChar(tmp);
    printChar(' ');
    printChar(' ');
    printChar('K');
    printChar('=');
    tmp = k;
    if(k == 10)
    {
    	printChar('1');
    	printChar('.');
    	printChar('0');
    }else
    {
    	printChar('0');
    	printChar('.');
    	printChar(tmp);
    }

}

