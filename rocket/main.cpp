#include <windows.h>
#include "resource.h"
#include <tchar.h>
#include <ctime>
#include "rocket.h"
#include <WindowsX.h>
#include <CommCtrl.h>

#ifdef UNICODE
#define SPRINTF wsprintf
#else
#define SPRINTF sprintf
#endif

BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK testProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp);
HWND hGame, hFuel, hLR, hUp, hStatus;

TCHAR name[100];
int lives=0, diff=0;
void CreateRocketInTheMiddle(HWND);
void CreateGround(int);
void DrawGround(HWND);
void DrawRocket(HWND, int, int, int, RECT);
int isLanded(RECT, int);
void ChangeRocketView(int);
void DrawFireLeft(HWND, int, RECT);
void DrawFireRight(HWND, int, RECT);
void DrawFireDown(HWND, int, RECT);
void checkRect(RECT*);

Rocket *Rocky;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdShow)
{
		if(DialogBox(hInstance,MAKEINTRESOURCE(IDD_HELLO), NULL, testProc) == 0)
		return 0;

	Rocky = new Rocket(diff,diff+4);
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
}
BOOL CALLBACK testProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
	HWND hName = GetDlgItem(hwnd, IDC_NAME);
	HWND hLives = GetDlgItem(hwnd, IDC_LIVESNUM);
	HWND hDiff = GetDlgItem(hwnd, IDC_DIFF);

	switch(message)
	{
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return TRUE;
	case WM_INITDIALOG:
		{
			SendMessage(hLives, UDM_SETRANGE32, 1, 5);
			HWND hLivesText = GetDlgItem(hwnd, IDC_LIVESTEXT);
			SetWindowText(hLivesText, TEXT("1"));
			SendMessage(hDiff, TBM_SETRANGE, true, MAKELPARAM(1,3));
		}
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wp) == IDOK)
		{
			SendMessage(hName, WM_GETTEXT, WPARAM(100), LPARAM(name));
			lives = SendMessage(hLives, UDM_GETPOS32, 0, NULL);
			diff = SendMessage(hDiff, TBM_GETPOS, 0, 0);
			EndDialog(hwnd, 1);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
	switch(message)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;

	case WM_INITDIALOG:
		{
			CreateRocketInTheMiddle(hWnd);
			CreateGround(diff);
			SendMessage(GetDlgItem(hWnd, IDC_FORWARD), TBM_SETRANGE, 1, MAKELPARAM(0,100));
			SendMessage(GetDlgItem(hWnd, IDC_LR), TBM_SETRANGE, 1, MAKELPARAM(-100,100));
			SendMessage(GetDlgItem(hWnd, IDC_FORWARD), TBM_SETPOS, 1, 0);
			DrawGround(hWnd);
			SetTimer(hWnd, 1, 100, 0);

			hLR = GetDlgItem(hWnd, IDC_LR);
			hUp = GetDlgItem(hWnd, IDC_FORWARD);
			hFuel = GetDlgItem(hWnd, IDC_FUEL);
			hGame = hWnd;

			SendMessage(hLR, TBM_SETRANGE, TRUE, MAKELPARAM(-100, 100));
			SendMessage(hUp, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));

			hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_TOOLTIPS, 0, hGame, WM_USER);
			int parts[5] = {150, 300, 450,600 , -1};
			SendMessage(hStatus, SB_SETPARTS, 5, (LPARAM)parts);

			SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) TEXT("speed: 0"));

			TCHAR tlives[12];
			wsprintf(tlives, TEXT("lives: %d "), lives);
			SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM) tlives);

			SendMessage(hStatus, SB_SETTEXT, 2, (LPARAM) TEXT("fuel:"));

			SendMessage(hStatus, SB_SETTEXT, 3, (LPARAM) TEXT("engine LR:"));

			SendMessage(hStatus, SB_SETTEXT, 4, (LPARAM) TEXT("engine UP:"));

			hFuel = GetDlgItem(hWnd, IDC_FUEL);
			SendMessage(hFuel, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			SendMessage(hFuel, PBM_SETSTEP, 1, 0);
			SendMessage(hFuel, PBM_SETPOS, Rocky->Get_Fuel(), 0);
			SendMessage(hFuel, PBM_SETBKCOLOR, 0, LPARAM(RGB(0, 0, 255)));
			SendMessage(hFuel, PBM_SETBARCOLOR, 0, LPARAM(RGB(255, 0, 255)));
		}
		return TRUE;

	case WM_TIMER:
		{

			Rocky->Set_Y_engine(SendMessage(GetDlgItem(hWnd, IDC_FORWARD), TBM_GETPOS, 1, MAKELPARAM(0,100)));
			Rocky->Set_X_engine(SendMessage(GetDlgItem(hWnd, IDC_LR), TBM_GETPOS, 1, MAKELPARAM(-100,100)));

			int x_engine = Rocky->Get_X_engine();

			TCHAR engineLR[30];
			wsprintf(engineLR, TEXT("engine LR %d "), x_engine);
			SendMessage(hStatus, SB_SETTEXT, 3,(LPARAM)engineLR);

			int y_engine = Rocky->Get_Y_engine();

			TCHAR engineUp[30];
			wsprintf(engineUp, TEXT("engine UP: %d "), y_engine);
			SendMessage(hStatus, SB_SETTEXT, 4,(LPARAM)engineUp);

			int fuel = Rocky->Get_Fuel();
			TCHAR tfuel[50];
			wsprintf(tfuel, TEXT("fuel: %d "), fuel);
			SendMessage(hStatus, SB_SETTEXT, 2,(LPARAM)tfuel);
			SendMessage(hFuel, PBM_SETPOS, WPARAM(fuel), 0);
			int speed = Rocky->Get_Speed();
			TCHAR tspeed[50];
			wsprintf(tspeed, TEXT("Speed: %d"), speed);
			SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) tspeed);

			Rocky->CalculateNext();

			DrawRocket(hWnd, fuel, x_engine, y_engine, Rocky->Get_Cur_Pos());

			int landed_state = isLanded(Rocky->Get_Cur_Pos(), speed);
			ChangeRocketView(landed_state);

			if(landed_state == 1)
			{
				KillTimer(hWnd, 1);
				TCHAR tmp[150];
				wsprintf(tmp, TEXT("Congradulations %s, You won whith %d lives"), name, lives);
				MessageBox(0,tmp,TEXT("You won"),MB_OK | MB_ICONINFORMATION);
				if(lives>0)
				{
					EndDialog(hWnd,0);
					delete Rocky;
					Rocky = new Rocket(diff,diff+4);
					for(int i=0; i<3; i++)
					{
						hFireLeft[i] = 0;
						hFireRight[i] = 0;
						hFireDown[i] = 0;
					}

					DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
				}
				else
				{
					MessageBox(0,TEXT("You lose. Good bye =)"),TEXT("You lose"),MB_OK | MB_ICONINFORMATION);
					EndDialog(hWnd,0);
				}
			}
			else if( landed_state == 2 || landed_state == 3)
			{
				KillTimer(hWnd, 1);
				TCHAR tmp[150];
				wsprintf(tmp, TEXT("%s,try again. You have %d lives"), name, lives);
				MessageBox(0,tmp,TEXT("You lose"),MB_OK | MB_ICONINFORMATION);
				if(lives>0)
				{
					EndDialog(hWnd,0);
					delete Rocky;
					Rocky = new Rocket(diff,diff+4);
					for(int i=0; i<3; i++)
					{
						hFireLeft[i] = 0;
						hFireRight[i] = 0;
						hFireDown[i] = 0;
					}

					DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
				}
				else
				{
					MessageBox(0,TEXT("You lose. Good bye =)"),TEXT("You lose"),MB_OK | MB_ICONINFORMATION);
					EndDialog(hWnd,0);
				}
			}

		}
		return TRUE;

	}
	return FALSE;
}
void CreateRocketInTheMiddle(HWND hWnd)
{
	RECT r = Rocky->Get_Cur_Pos();
	hRocket = CreateWindowEx(0, TEXT("STATIC"), 0, WS_CHILD | WS_VISIBLE | SS_ICON | WS_EX_TRANSPARENT, r.left, r.top, rWidth, rHeight, hWnd, 0, GetModuleHandle(0), 0);
	ChangeRocketView(-1);
}

void CreateGround(int iDifficult)
{
	srand(time(0));


	for(int i = 0; i < gSize; i+=2)
	{
		groundArr[i] = rand() % (iDifficult + 1);
		if(i + 1 < gSize)
			groundArr[i+1] = groundArr[i];
	}

	int step = (rWidth / gWidth) + 3;
	int iHaveGoodGround = 0;
	for(int i = 0; i < gSize - step; i++)
	{
		bool good = true;
		for(int j = i; j < i + step; j++)
		{
			if(groundArr[j] > 0)
			{
				good = false;
				break;
			}
		}

		if(good)
		{
			iHaveGoodGround++;
			i += step;
		}
	}

	for(int i = 3 - (iHaveGoodGround + iDifficult); i > 0; i--)
	{
		int pos = rand() %  (gSize - step * 2);
		for(int j = pos; j < pos + (step + step * 1.25); j++)
			groundArr[j] = 0;
	}
}

void DrawGround(HWND hWnd)
{
	int X = fX0;
	int Y = gY;
	int nWidth = gWidth;
	int nHeight = gWidth;

	for(int i = 0; i < gSize; i++)
	{
		HWND hGround = CreateWindowEx(0, TEXT("STATIC"), 0, WS_CHILD | WS_VISIBLE | SS_BITMAP, X, Y, nWidth, nHeight, hWnd, 0, GetModuleHandle(0), 0);
		HBITMAP bmtGround;

		if(groundArr[i] == 0)
			bmtGround = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_GOOD_GRAUND));
		else
			bmtGround = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(IDB_BAD_GRAUND));

		SendMessage(hGround, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmtGround);
		UpdateWindow(hGround);

		X += gWidth;
	}
}

