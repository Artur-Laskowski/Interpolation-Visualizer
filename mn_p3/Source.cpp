#include <iostream>
#include <stdlib.h>

using namespace std;

void readFromFile(char *name, int *size, double** xArr, double** yArr)
{
	FILE *fp;

	fp = fopen(name, "r");

	if (fp == NULL)
		return;

	char buf[50];

	fgets(buf, 50, fp);
	if (buf[0] <= '9' && buf[0]	>= '0')
	{
		sscanf(buf, "%d", size);
	}
	else
	{
		while (fgets(buf, 50, fp) != NULL)
		{
			(*size)++;
		}
		fp = fopen(name, "r");
		fgets(buf, 50, fp);
	}

	*xArr = (double*)malloc(sizeof(double)* *size);
	*yArr = (double*)malloc(sizeof(double)* *size);


	for (int i = 0; i < *size; i++)
	{
		int middle = 0;
		fgets(buf, 50, fp);

		middle = 0;
		for (int j = 0; j < 50; j++)
		{
			if (buf[j] == ',' || buf[j] == '\n')
			{
				buf[j] = 0;
				if (middle == 0)
					middle = j;
			}
		}
		double firstNum;
		double secondNum;
		sscanf(buf, "%lf", &(*xArr)[i]);
		sscanf(&buf[middle + 1], "%lf", &(*yArr)[i]);

	}



	fclose(fp);
}


double interpolate(double *xArr, double *yArr, int size, double xr)
{
	double fy = 0;
	for (int j = 0; j < size; j++)
	{
		double num, den;
		num = den = 1;
		for (int i = 0; i < size; i++)
		{
			if (i == j)
				continue;

			num = num * (xr - xArr[i]);
			den = den * (xArr[j] - xArr[i]);
		}

		fy += ((num / den) * yArr[j]);
	}

	return fy;
}


int main()
{
	int size;
	double *xArr, *yArr;
	readFromFile("data.csv", &size, &xArr, &yArr);

	double *xArrNew, *yArrNew;
	int ratio = 3; // 1/ratio - ile

	bool remove = true; // zostawic lub usunac


	xArrNew = (double*)malloc(size * sizeof(double));
	yArrNew = (double*)malloc(size * sizeof(double));

	//usuwamy 1/3
	int j = 0;
	for (int i = 0; i < size; i++)
	{
		if (remove) {
			if (i % ratio == 0 && i != 0)
			{
				yArr[i] = 0;
			}
			else
			{
				yArrNew[i] = yArr[i];
				xArrNew[i] = xArr[i];
				j++;
			}
		}
		else
		{
			if (i % ratio == 0 && i != 0)
			{
				yArrNew[j] = yArr[i];
				xArrNew[j] = xArr[i];
				j++;
			}
			else
			{
				yArr[i] = 0;
			}
		}
	}
	int newSize = j;

	for (int i = 0; i < size; i++)
	{
		if (yArr[i] == 0)
		{
			yArr[i] = interpolate(xArrNew, yArrNew, newSize, xArr[i]);
		}
	}
	

	//exec("visualizer.exe");
	system("\"C:\\Users\\Artur\\Documents\\Visual Studio 2017\\Projects\\mn_p3\\Debug\\visualizer.exe\"");

	return 0;
}