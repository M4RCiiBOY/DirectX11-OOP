#pragma once
#include "main.h"




bool InitializeDirect3d11App(HINSTANCE hInstance);
void CleanUp();
bool InitScene();
void UpdateScene();
void DrawScene();

void DetectInput(double time);
void StartTimer();
double GetFrameTime();
bool InitDirectInput(HINSTANCE hInstance);

void CreateSphere(int LatLines, int LongLines);

bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed);
int messageloop();

LRESULT CALLBACK WndProc(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);


