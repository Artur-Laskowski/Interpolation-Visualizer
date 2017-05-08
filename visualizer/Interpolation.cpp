#include "stdafx.h"
#include "Interpolation.h"

double interpolate(double *xArr, double *yArr, int size, double xr)
{
	double fy = 0;
	int start = 0;
	int end = size;

start_interpolacji:
	for (int j = start; j < end; j++)
	{
		double num, den, res;
		num = den = res = 1;
		for (int i = start; i < end; i++)
		{
			if (i == j)
				continue;

			//num *= xr - xArr[i];
			//den *= xArr[j] - xArr[i];
			res *= (xr - xArr[i]) / (xArr[j] - xArr[i]);
		}

		fy += res * yArr[j];
	}

	return fy;
}

double* algorytmGaussa(double* macierz, double* wektor, double* x, int n)
{
	for (int h = 1; h < n; h++)
	{
		for (int i = h; i < n; i++)
		{
			double m = macierz[i*n + h - 1] / macierz[(h - 1)*n + h - 1];
			macierz[i*n + h - 1] = 0;
			for (int j = h; j < n; j++)
			{
				macierz[i*n + j] = macierz[i*n + j] - m*macierz[(h - 1)*n + j];
			}
			wektor[i] = wektor[i] - m*wektor[h - 1];
		}
	}

	x[n - 1] = wektor[n - 1] / macierz[(n - 1)*n + n - 1];

	for (int i = n - 2; i >= 0; i--) {
		double suma = 0;
		for (int j = i + 1; j < n; j++)
		{
			suma += macierz[i*n + j] * x[j];
		}
		x[i] = (wektor[i] - suma) / macierz[i*n + i];
	}
	return x;
}


void initSplineInterpolate(double *xArr, double *yArr, int size, double *mArray)
{
	int arrSize = size + 1;
	//calloc wypelnia wszystko zerami wiec zostaje tylko wpisac wartosci po przekatnej
	double *arr = (double *)calloc((arrSize)*(arrSize), sizeof(double));
	double *vec = (double *)calloc(arrSize, sizeof(double));

	for (int i = 0; i < arrSize; i++)
	{
		arr[i*arrSize + i] = 2;

		if (i < 1 || i >= arrSize - 1)
			continue;


		double h_j1 = xArr[i + 1] - xArr[i];
		double h_j = xArr[i] - xArr[i - 1];

		arr[i*arrSize + i + 1] = h_j1 / (h_j + h_j1); //lambda
		arr[i*arrSize + i - 1] = h_j / (h_j + h_j1); //to drugie

		vec[i] = (6.0 / (h_j + h_j1));
		vec[i] *= (((yArr[i + 1] - yArr[i]) / h_j1) - ((yArr[i] - yArr[i - 1]) / h_j));
	}

	//mArray zostanie wypelnione wartosciami M_0, ..., M_n
	algorytmGaussa(arr, vec, mArray, arrSize);
}


double splineInterpolate(double *xArr, double *yArr, int size, double xr, double* mArray)
{
	double a_j, b_j, c_j, d_j, h_j1;

	//szukanie przedzialu w ktorym znajduje sie xr
	int posIndex = size - 2;
	for (int i = 0; i < size; i++)
	{
		if (xArr[i] > xr) {
			posIndex = i - 1;
			break;
		}
	}

	a_j = yArr[posIndex];
	h_j1 = xArr[posIndex + 1] - xArr[posIndex];

	b_j = (yArr[posIndex + 1] - yArr[posIndex]) / h_j1 - ((2.0 * mArray[posIndex] + mArray[posIndex + 1]) / 6.0) * h_j1;
	c_j = mArray[posIndex] / 2.0;
	d_j = (mArray[posIndex + 1] - mArray[posIndex]) / (6.0 * h_j1);

	return a_j + b_j * (xr - xArr[posIndex]) + c_j*(xr - xArr[posIndex])*(xr - xArr[posIndex]) + d_j*(xr - xArr[posIndex])*(xr - xArr[posIndex])*(xr - xArr[posIndex]);
}