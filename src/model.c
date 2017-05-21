//
// MODEL.C
// Written by J.W.Meissner
//

#include "stm32f4xx.h"

#include "hw.h"
#include "model.h"

double Tp = 0.01f;
double T1 = 1;
double T2 = 3;
double zeta = 0.3;
double k = 1;

double a1, a2, a3, b0, b1, b2, b3;
double x, y;
double w[4] = { 0, 0, 0, 0 };

double a = 0.0f;
double b = 0;
double c = 0;
double d = 0;

void initializeParameters(void) {

	a = 2 / Tp;
	b = a * a * a * T1 * T2 * T2;
	c = a * a * (T2 * T2 + 2 * zeta * T1 * T2);
	d = a * (T1 + 2 * zeta * T2);

	a1 = (3 + d - c - 3 * b);
	a2 = (3 - d - c + 3 * b);
	a3 = (1 - d + c - b);

	b0 = k;
	b1 = 3 * k;
	b2 = 3 * k;
	b3 = k;
	x = 2000;
}

double calculateResponse(double input) {
	y = (b0 * input + w[0]) / (b + c + d + 1);
	w[0] = b1 * input + w[1] - a1 * y;
	w[1] = b2 * input + w[2] - a2 * y;
	w[2] = b3 * input - a3 * y;
	return y;
}

