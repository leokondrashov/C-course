/*! \file squareEquation.c
 * 
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define INF_ROOTS -1
#define EPS 0.000001

int SolveSquareEquation(double a, double b, double c, double *x1, double *x2);
int SolveLinearEquation(double a, double b, double *x);

int main() {
	printf("$ Square equation solver\n");
	printf("$ (c) Leo, 2018\n\n");
	
	double a = 0, b = 0, c = 0;
	printf("$ Enter square equations coefficients:\n");
	scanf("%lf %lf %lf", &a, &b, &c);
	
	double x1 = 0, x2 = 0;
	int RootCount = SolveSquareEquation(a, b, c, &x1, &x2);

	switch (RootCount) {
		case 0:
			printf("$ No roots\n");
			break;
		case 1:
			printf("$ One root:\n");
			printf("$ x = %lf\n", x1);
			break;
		case 2:
			printf("$ Two roots:\n");
			printf("$ x1 = %lf, x2 = %lf\n", x1, x2);
			break;
		case INF_ROOTS:
			printf("$ Any number\n");
			break;
		default:
			printf("$ ERROR: RootCount = %d\n", RootCount);
			return 1;
	}
}

/*!
*Square Equation Solver
*
*\param [in] a the first coefficient of equaion
*\param [in] b the second coefficient of equaion
*\param [in] c the third coefficient of equaion
*\param [out] *x1 pointer to the first root of equation
*\param [out] *x2 pointer to the second root of equation
*
*\note If number of roots is less then two, corresponding roots is set to 0
*\note Returns INF_ROOTS if any number is solution
*
*\return Number of roots
*/
int SolveSquareEquation(double a, double b, double c, double *x1, double *x2) {
	assert(x1 != NULL);
	assert(x2 != NULL);	
	assert(x1 != x2);

	assert(isfinite(a));
	assert(isfinite(b));
	assert(isfinite(c));

	if (a == 0) {
		*x2 = 0;
		return SolveLinearEquation(b, c, x1);
	} else {
		double d = b * b - 4 * a * c;
		if (fabs(d) < EPS) {
			*x1 = -b / (2 * a);
			*x2 = 0;
			return 1;
		} else if (d < 0) {
			*x1 = 0;
			*x2 = 0;
			return 0;
		} else {
			double sqrt_d = sqrt(d);
			*x1 = (-b + sqrt_d) / (2 * a);
			*x2 = (-b - sqrt_d) / (2 * a);
			return 2;
		}
	}
	return 0;
}

/*!
*Linear Equation Solver
*
*Solves equations of the form a*x + b = 0
*
*\param [in] a the first coefficient of equaion
*\param [in] b the second coefficient of equaion
*\param [out] *x pointer to the root of equation
*
*\note If there is no roots *x is set to 0
*\note Returns INF_ROOTS if any number is solution
*
*\return Number of roots
*/
int SolveLinearEquation(double a, double b, double *x) {
	assert(x != NULL);
	
	assert(isfinite(a));
	assert(isfinite(b));
	
	if (a == 0) {
		*x = 0;
		return (b == 0) ? INF_ROOTS : 0;
	} else {
		*x = -b / a;
		return 1;
	}
}