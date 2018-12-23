/* set TESSDATA_PREFIX=C:\Users\Sergei\source\repos\Text_Recognizer\TextRecognizer\Debug\ */
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <Windows.h>

//include tesseract headers
#include <baseapi.h>
#include <allheaders.h>

#define WM_LOAD_FILE WM_USER

using namespace std;

string TesseractOcr(string preprocessed_file);
string TesseractPreprocess(string source_file);

string ocr_result = "";

LOGFONT lf;
int textHeight = 25;
string fontStyle = "Courier New";
HPEN pen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
string previewMessage = "Press F12 for information";

bool LoadFileMessage(HWND hWnd)
{
	return PostMessage(hWnd, WM_LOAD_FILE, NULL, NULL);
}

void LoadResultToFile(HWND hWnd, string ocrResultStr, string filename)
{
	// create new file instance on the base of loaded picture
	filename.erase(filename.find_last_of("."), string::npos);

	// writing result of ocr to the new file
	ofstream fout(filename + ".txt");
	fout << ocrResultStr.c_str();
	fout.close();
}

int LoadFileOnWindow(HWND hWnd)
{

	char fileName[MAX_PATH] = { NULL };

	OPENFILENAME openFileName;
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = hWnd;
	openFileName.hInstance = NULL;
	openFileName.lpstrFilter = "Images\0*.bmp;*.gif;*.jpeg;*.jpg;*.png;*.tiff;*.exif;*.wmf;*.emf\0\0";
	openFileName.lpstrCustomFilter = NULL;
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFile = fileName;
	openFileName.nMaxFile = sizeof(fileName);
	openFileName.lpstrFileTitle = NULL;
	openFileName.lpstrInitialDir = NULL;
	openFileName.lpstrTitle = "Select image";
	openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	openFileName.lpstrDefExt = NULL;

	if (GetOpenFileName(&openFileName))
	{
		//preprocess to convert to black white book-like text
		string preprocessed_file = TesseractPreprocess(fileName);

		//ocr and display results
		ocr_result = TesseractOcr(preprocessed_file);

		LoadResultToFile(hWnd, ocr_result, fileName);
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int prevGraphicsMode;

	switch (message)
	{
	case WM_PAINT:
		if (ocr_result != "") {
			// text output
			hdc = BeginPaint(hWnd, &ps);
			prevGraphicsMode = SetGraphicsMode(hdc, GM_ADVANCED);
			SelectObject(hdc, brush);
			SelectObject(hdc, pen);

			// text structure
			ZeroMemory(&lf, sizeof(LOGFONT));
			lf.lfHeight = textHeight;						// height
			strcpy_s(lf.lfFaceName, fontStyle.c_str());		// font style

			SelectObject(hdc, CreateFontIndirect(&lf));
			TextOut(hdc, 50, 50, ocr_result.c_str(), ocr_result.length());
		}
		else
		{
			// text output
			hdc = BeginPaint(hWnd, &ps);
			prevGraphicsMode = SetGraphicsMode(hdc, GM_ADVANCED);
			SelectObject(hdc, brush);
			SelectObject(hdc, pen);

			// text structure
			ZeroMemory(&lf, sizeof(LOGFONT));
			lf.lfHeight = textHeight;						// height
			strcpy_s(lf.lfFaceName, fontStyle.c_str());		// font style

			SelectObject(hdc, CreateFontIndirect(&lf));
			TextOut(hdc, 50, 50, previewMessage.c_str(), previewMessage.length());
		}
		break;
	case WM_LOAD_FILE:
		LoadFileOnWindow(hWnd);
		InvalidateRect(hWnd, NULL, true);
		break;
	case WM_KEYDOWN:
		switch (wParam) {
			case VK_ESCAPE:
			{
				int isExit = MessageBox(hWnd, "Exit?", "Message", MB_OKCANCEL);
				if (isExit == IDOK) {
					PostQuitMessage(0);
				}
			}
			break;
			case VK_F1:
			{
				LoadFileMessage(hWnd);
				break;
			}
			case VK_F12:
			{
				MessageBox(hWnd, "This program is created to recognize text on pictures.\nPress F1 to load image.\nPress Esc to exit programm.", "Message", MB_OKCANCEL);
				break;
			}
			break;
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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex; HWND hWnd; MSG msg;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "Recognizer";
	wcex.hIconSm = wcex.hIcon;
	RegisterClassEx(&wcex);

	hWnd = CreateWindow("Recognizer", "Recognizer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

string TesseractPreprocess(string source_file)
{
	

	char tempPath[128];
	GetTempPathA(128, tempPath);
	strcat_s(tempPath, "preprocess_ocr.bmp");

	char preprocessed_file[MAX_PATH];
	strcpy_s(preprocessed_file, tempPath);


	BOOL perform_negate = TRUE;
	l_float32 dark_bg_threshold = 0.5f; // From 0.0 to 1.0, with 0 being all white and 1 being all black 

	int perform_scale = 1;
	l_float32 scale_factor = 3.5f;

	int perform_unsharp_mask = 1;
	l_int32 usm_halfwidth = 5;
	l_float32 usm_fract = 2.5f;

	int perform_otsu_binarize = 1;
	l_int32 otsu_sx = 2000;
	l_int32 otsu_sy = 2000;
	l_int32 otsu_smoothx = 0;
	l_int32 otsu_smoothy = 0;
	l_float32 otsu_scorefract = 0.0f;


	l_int32 status = 1;
	l_float32 border_avg = 0.0f;
	PIX *pixs = NULL;
	char *ext = NULL;


	// Read in source image 
	pixs = pixRead(source_file.c_str());


	// Convert to grayscale 
	pixs = pixConvertRGBToGray(pixs, 0.0f, 0.0f, 0.0f);


	if (perform_negate)
	{
		PIX *otsu_pixs = NULL;

		status = pixOtsuAdaptiveThreshold(pixs, otsu_sx, otsu_sy, otsu_smoothx, otsu_smoothy, otsu_scorefract, NULL, &otsu_pixs);


		// Get the average intensity of the border pixels,
		//with average of 0.0 being completely white and 1.0 being completely black. 
		border_avg = pixAverageOnLine(otsu_pixs, 0, 0, otsu_pixs->w - 1, 0, 1);                               // Top 
		border_avg += pixAverageOnLine(otsu_pixs, 0, otsu_pixs->h - 1, otsu_pixs->w - 1, otsu_pixs->h - 1, 1); // Bottom 
		border_avg += pixAverageOnLine(otsu_pixs, 0, 0, 0, otsu_pixs->h - 1, 1);                               // Left 
		border_avg += pixAverageOnLine(otsu_pixs, otsu_pixs->w - 1, 0, otsu_pixs->w - 1, otsu_pixs->h - 1, 1); // Right 
		border_avg /= 4.0f;

		pixDestroy(&otsu_pixs);

		// If background is dark 
		if (border_avg > dark_bg_threshold)
		{
			// Negate image 
			pixInvert(pixs, pixs);
	
		}
	}

	if (perform_scale)
	{
		// Scale the image (linear interpolation) 
		pixs = pixScaleGrayLI(pixs, scale_factor, scale_factor);
	}

	if (perform_unsharp_mask)
	{
		// Apply unsharp mask 
		pixs = pixUnsharpMaskingGray(pixs, usm_halfwidth, usm_fract);
	}

	if (perform_otsu_binarize)
	{
		// Binarize 
		status = pixOtsuAdaptiveThreshold(pixs, otsu_sx, otsu_sy, otsu_smoothx, otsu_smoothy, otsu_scorefract, NULL, &pixs);
	}

	
	// Write the image to file 
	status = pixWriteImpliedFormat(preprocessed_file, pixs, 0, 0);
	

	string out(preprocessed_file);

	return out;

}

string TesseractOcr(string preprocessed_file)
{
	char *outText;
	Pix *image = pixRead(preprocessed_file.c_str());
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();

	
	TCHAR CurDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, CurDir);

	api->Init(CurDir, "eng");  
	api->SetPageSegMode(tesseract::PSM_AUTO_OSD); //PSM_SINGLE_BLOCK PSM_AUTO_OSD

	
	api->SetImage(image);

	outText = api->GetUTF8Text();
	
	system("cls");

	string out(outText);
	return out;
}