// visualizer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "visualizer.h"
#include <cstdio>
#include <cmath>
#include "interpolation.h"

#define MAX_LOADSTRING 100

#define OFFSET_X 100
#define OFFSET_Y 200

#define FILE_NAME "data.csv"
#define INTERPOLATION_SIZE 10
#define START 0
#define END -1
#define RATIO 10
#define REMOVE false
#define MAX_VAL 100
//#define LAGRANGE true

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
RECT rect;


double *xArr, *yArr, *xArrOriginal, *yArrOriginal, scaleX, scaleY, yMax = 0, yMin = INFINITY;
int size, winWidth, winHeight;
double offsetX, offsetY;
double *xArr2, *yArr2;
bool moving = false;
int movingStartPointX, movingStartPointY;
double movingOffsetX = 0, movingOffsetY = 0;
double zoom = 1;
double errorSum;
double errorSumSplines;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                readFromFile(char*, int*, double**, double**);
void                initSplineInterpolate(double *, double *, int, double *);
double              splineInterpolate(double *, double *, int, double, double*);
double              interpolate(double*, double*, int, double);
void                drawFunction(double*, double*, COLORREF, HDC);
void                initialize();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	initialize();


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VISUALIZER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VISUALIZER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VISUALIZER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VISUALIZER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


void initialize()
{
	char *name = FILE_NAME;

	//wczytanie funkcji z pliku, zmiana globalnej zmiennej size, xArr, yArr
	readFromFile(name, &size, &xArr, &yArr);
	if (END != -1)
		size = END;

	//kopiowanie nienaruszonej funkcji
	xArrOriginal = (double*)malloc(sizeof(double)* size);
	yArrOriginal = (double*)malloc(sizeof(double)* size);

	for (int i = 0; i < size; i++)
	{
		xArrOriginal[i] = xArr[i];
		yArrOriginal[i] = yArr[i];
	}

	double *xArrNew, *yArrNew;

	int ratio = RATIO; // 1/ratio - ile
	bool remove = REMOVE; // zostawic lub usunac

	//krotsze tablice z punktami ktore zostana
	xArrNew = (double*)malloc(size * sizeof(double));
	yArrNew = (double*)malloc(size * sizeof(double));


	int j = 0;
	for (int i = 0; i < size; i++) {
		if (remove) {
			if (i % ratio == 0) {
				yArr[i] = 0;
			}
			else {
				yArrNew[j] = yArr[i];
				xArrNew[j] = xArr[i];
				j++;
			}
		}
		else {
			if (i % ratio == 0) {
				yArrNew[j] = yArr[i];
				xArrNew[j] = xArr[i];
				j++;
			}
			else {
				yArr[i] = 0;
			}
		}
	}
	int newSize = j;

	//tablica z wartosciami M_0, M_1, ..., M_n
	double* mArray = (double *)calloc(newSize + 1, sizeof(double));
	//wypelnienie tablicy M
	initSplineInterpolate(xArrNew, yArrNew, newSize, mArray);
	//nowa tablica na wyniki z CSI
	yArr2 = (double *)malloc(size * sizeof(double));
	for (int i = 0; i < size; i++)
	{
		yArr2[i] = yArr[i];
	}

	for (int i = 0; i < size; i++)
	{
		if (yArr[i] == 0)
		{
			//if (i < 80 || i > size - 80) continue;
			yArr[i] = interpolate(xArrNew, yArrNew, newSize, xArr[i]);
			errorSum += abs(yArrOriginal[i] - yArr[i]) / yArrOriginal[i];
			yArr2[i] = splineInterpolate(xArrNew, yArrNew, newSize, xArr[i], mArray);
			errorSumSplines += abs(yArrOriginal[i] - yArr2[i]) / yArrOriginal[i];
		}
	}

	errorSum /= size - newSize;
	errorSumSplines /= size - newSize;

	for (int i = START; i < size; i++)
	{
		if (yArrOriginal[i] > yMax && yArrOriginal[i] < MAX_VAL)
			yMax = yArrOriginal[i];

		if (yArrOriginal[i] < yMin && yArrOriginal[i] > -MAX_VAL)
			yMin = yArrOriginal[i];
	}
}


void readFromFile(char *name, int *size, double** xArr, double** yArr)
{
	FILE *fp;

	fp = fopen(name, "r");

	if (fp == NULL)
		return;

	char buf[50];

	fgets(buf, 50, fp);
	if (buf[0] <= '9' && buf[0] >= '0')
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

	*xArr = (double*)malloc(sizeof(double) * *size);
	*yArr = (double*)malloc(sizeof(double) * *size);


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

double transformX(double x)
{
	double xPrev = (x - offsetX) * scaleX + OFFSET_X / 2 + movingOffsetX / zoom;
	xPrev = xPrev + movingOffsetX - winWidth / 2;
	xPrev = xPrev * zoom;
	xPrev = xPrev - movingOffsetX + winWidth / 2;
	return xPrev;
}

double transformY(double y)
{
	double yPrev = (y - offsetY) * scaleY + OFFSET_Y / 2 - movingOffsetY / zoom;
	yPrev = yPrev - movingOffsetY - winHeight / 2;
	yPrev = yPrev * zoom;
	yPrev = yPrev + movingOffsetY + winHeight / 2;
	return yPrev;
}


void drawFunction(double* x, double* y, COLORREF color, HDC hdc)
{
	HPEN hPen = CreatePen(PS_SOLID, 3, color);
	SelectObject(hdc, hPen);

	double xPrev = transformX(x[START]);
	double yPrev = transformY(y[START]);

	for (int i = START + 1; i < size; i++)
	{
		MoveToEx(hdc, xPrev, winHeight - yPrev, NULL);
		xPrev = transformX(x[i]);
		yPrev = transformY(y[i]);
		
		LineTo(hdc, xPrev, winHeight - yPrev);
	}
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

			double xMax = xArr[size - 1];
			double xMin = xArr[START];
			offsetY = yMin;

			GetWindowRect(hWnd, &rect);
			winWidth = rect.right - rect.left;
			winHeight = rect.bottom - rect.top - 50;

			scaleY = (double)(winHeight - OFFSET_Y) / (yMax - yMin);
			scaleX = (double)(winWidth - OFFSET_X) / (xMax - xMin);

			offsetX = xArr[START];

			hdc = GetDC(hWnd);

			//Rectangle(hdc, 0, 0, winWidth, winHeight);
			

			char text[100];
			sprintf(text, "%.2f", yMin);
			double xPrev = transformX(xArr[0]);
			double yPrev = transformY(yMin);
			TextOutA(hdc, xPrev, winHeight - yPrev, text, strlen(text));

			sprintf(text, "Sredni blad: %.6f",errorSum);
			TextOutA(hdc, 10, 10, text, strlen(text));

			sprintf(text, "Sredni blad sklejanych: %.6f", errorSumSplines);
			TextOutA(hdc, 10, 30, text, strlen(text));

			xMax = transformX(xMax);

			MoveToEx(hdc, xPrev, winHeight - yPrev, NULL);
			LineTo(hdc, xMax, winHeight - yPrev);

			double yMax2 = transformY(yMax);

			MoveToEx(hdc, xPrev, winHeight - yMax2, NULL);
			LineTo(hdc, xMax, winHeight - yMax2);

			sprintf(text, "%f", yMax);
			TextOutA(hdc, xPrev, winHeight - yMax2, text, 10);



			drawFunction(xArrOriginal, yArrOriginal, RGB(0, 0, 255), hdc);

			drawFunction(xArr, yArr, RGB(255, 0, 0), hdc);

			drawFunction(xArr, yArr2, RGB(0, 255, 0), hdc);


			ReleaseDC(hWnd, hdc);
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_LBUTTONDOWN:
		{
			moving = true;
			movingStartPointX = LOWORD(lParam) - movingOffsetX*zoom;
			movingStartPointY = HIWORD(lParam) - movingOffsetY*zoom;
		}
		break;
	case WM_LBUTTONUP:
		{
			moving = false;
		}
		break;
	case WM_MOUSEMOVE:
		{
			if (!moving)
				break;
			movingOffsetX = (LOWORD(lParam) - movingStartPointX) / zoom;
			movingOffsetY = (HIWORD(lParam) - movingStartPointY) / zoom;
			InvalidateRect(NULL, &rect, TRUE);
			WndProc(hWnd, WM_PAINT, NULL, NULL);
		}
		break;
	case WM_MOUSEWHEEL:
		{
		if (HIWORD(wParam) == 65416)
			zoom -= zoom / 4.0;
		else
			if (HIWORD(wParam) == 120)
				zoom += zoom / 4.0;

		InvalidateRect(NULL, &rect, TRUE);
		WndProc(hWnd, WM_PAINT, NULL, NULL);
		}
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
