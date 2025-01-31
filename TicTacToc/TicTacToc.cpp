// TicTacToc.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TicTacToc.h"
#include <windowsx.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
BOOL GetCellRect(HWND hWnd, int index, RECT * pRect);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TICTACTOC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TICTACTOC));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TICTACTOC));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TICTACTOC);
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

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//Global variables for our game
const int CELL_SIZE = 100;

HBRUSH hbr1, hbr2;
int playerTurn = 1;
int gameBoard[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int winner = 0;
int wins[3]; 

BOOL GetGameBoardRect(HWND hwnd, RECT * pRect)
{
	RECT rc;
	if (GetClientRect(hwnd, &rc))
	{
		int width = rc.right - rc.left;
		int hight = rc.bottom - rc.top;

		pRect->left = (width - CELL_SIZE * 3) / 2;
		pRect->top = (hight - CELL_SIZE * 3) / 2;

		pRect->right = pRect->left + CELL_SIZE * 3;
		pRect->bottom = pRect->top + CELL_SIZE * 3;
		return TRUE;
	}

	SetRectEmpty(pRect);
	return FALSE;
}

void DrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
}

int GetCellNumberFromPoint(HWND hwnd, int x, int y)
{
	POINT pt = { x, y };
	RECT rc;

	if (GetGameBoardRect(hwnd, &rc))
	{
		if (PtInRect(&rc, pt))
		{
			// user clicked inside game board
			// NormaiIze ( 0 to 3*CELL_SIZE
			x = pt.x - rc.left;
			y = pt.y - rc.top;

			int column = x / CELL_SIZE;
			int row = y / CELL_SIZE;

			//convert to index ( 0 t 3 )
			return column + row * 3;
		}
	}
	return -1; // outside game board or failure 
}

BOOL GetCellRect(HWND hWnd, int  index, RECT * pRect)
{
	RECT rcBoard;

	SetRectEmpty(pRect);
	if (index < 0 || index > 8)
		return FALSE;

	if (GetGameBoardRect(hWnd, &rcBoard))
	{
		//Convert index from 0 - 8 into x,y pair
		int y = index / 3; //Row number
		int x = index % 3; //Column number

		pRect->left = rcBoard.left + x * CELL_SIZE + 1;
		pRect->top = rcBoard.top + y * CELL_SIZE + 1;

		pRect->right = pRect->left + CELL_SIZE - 1;
		pRect->bottom = pRect->top + CELL_SIZE - 1;

		return TRUE;
	}

	return FALSE;
}

/*
	Returns:
	0 - No winner
	1 - Player wins
	2 - Player wins
	3 - IT' a draw
*/

int GetWinner(int wins[3])
{
	int cells[] = {  0, 1, 2,
					 3, 4, 5,
					 6, 7, 8,

					 0, 3, 6,
					 1, 4, 7,
					 2, 5, 8,

					 0, 4, 8,
					 2, 4, 6 };

	// Check for winner
	for (int i = 0; i < ARRAYSIZE(cells); i += 3)
	{
		if ((0 != gameBoard[cells[i]]) && gameBoard[cells[i]] == gameBoard[cells[i + 1]] && gameBoard[cells[1]] && gameBoard[cells[i + 2]])
		{
			//We have a winner
			wins[0] = cells[i];
			wins[1] = cells[i + 1];
			wins[2] = cells[i + 2];

			return gameBoard[cells[i]];
		}
	}

	// Next, see if we have any cells left empty
	for (int i = 0; i < ARRAYSIZE(gameBoard); ++i)
		if (0 == gameBoard[i])
			return 0; // continue to play.......
		
	return 3; // it's a draw.
	
}

void ShowTurn(HWND hwnd, HDC hdc)
{
	static const WCHAR szTurn1 [] = L"Turn: Player 1";
	static const WCHAR szTurn2 [] = L"Turn: Player 2";

	const WCHAR * pszTurnText = NULL;

	switch (winner)
	{
	case 0: //continue to play
		pszTurnText = (playerTurn == 1) ? szTurn1 : szTurn2;
		break;
	case 1: //Player 1 wins
		pszTurnText = L"Player 1 is the winner!";
		break;
	case 2: //Player 2 wins
		pszTurnText = L"Player 2 is the winner!";
		break;
	case 3: //It's a draw
		pszTurnText = L"It's a draw";
		break;

	}

	RECT rc;

	if (NULL != pszTurnText && GetClientRect(hwnd, &rc))
	{
		rc.top = rc.bottom - 48;
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));
		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkMode(hdc, TRANSPARENT);
		DrawText(hdc, pszTurnText, lstrlen(pszTurnText), &rc, DT_CENTER);
	}
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		{
			hbr1 = CreateSolidBrush(RGB(255, 0, 0));
			hbr2 = CreateSolidBrush(RGB(0, 255, 0));
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case ID_FILE_NEWGAME:
				{
					int ret = MessageBox(hWnd, L"Are you sure you want to start new game?", L"NEW GAME", MB_YESNO | MB_ICONQUESTION);
					if (IDYES == ret)
					{
						// Reset and start a new game..
						playerTurn = 1;
						winner = 0;
						ZeroMemory(gameBoard, sizeof(gameBoard));
						//Force a paint message
						InvalidateRect(hWnd, NULL, TRUE); //post WM_PAINT to our windowsproc, it gets queued in our msg queue
						UpdateWindow(hWnd); // forces immediate handing of WM_PAINT
					}
				}
				break;
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

	case WM_LBUTTONDOWN:
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		// Only handle clicks if it is a player turn (i.e 1 or 2)
		if (0 == playerTurn)
			break;
		
		int index = GetCellNumberFromPoint(hWnd, xPos, yPos);
		
		HDC hdc = GetDC(hWnd);
		if (NULL != hdc)
			{

				//WCHAR temp[100];

				//wsprintf(temp, L"Index = %d", index);
				//TextOut(hdc, xPos, yPos, temp, lstrlen(temp));

				//Get cell dimension from its index
				if (index != -1)
				{
					RECT rcCell;
					if ((gameBoard[index] == 0) && GetCellRect(hWnd, index, &rcCell))
					{
						gameBoard[index] = playerTurn;

						FillRect(hdc, &rcCell, (playerTurn == 1) ? hbr1 : hbr2);

						//Check for a winner
						winner = GetWinner(wins);
						if (winner == 1 || winner == 2)
						{
							//We have a winner
							MessageBox(hWnd, (winner == 1) ? L"Player 1 is the winner" : L"Player 2 is a winner!", L"You win!", MB_OK | MB_ICONINFORMATION);
							playerTurn = 0;
						}
						else if (3 == winner)
						{
								//It's a draw
							MessageBox(hWnd, L"NO one win this time!", L"It's a draw!", MB_OK | MB_ICONEXCLAMATION);
							playerTurn = 0;
						}
						else if (0 == winner)
						{
							playerTurn = (playerTurn == 1) ? 2 : 1;
						}

						//DisPlay turn
						ShowTurn(hWnd, hdc);
					}
				}
				ReleaseDC(hWnd, hdc);

			}
	}
	break;
	case WM_GETMINMAXINFO:
		{
		MINMAXINFO * pMinMax = (MINMAXINFO*)lParam;

		pMinMax->ptMinTrackSize.x = CELL_SIZE * 5;
		pMinMax->ptMinTrackSize.y = CELL_SIZE * 5;

		}
    case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		RECT rc;

		if (GetGameBoardRect(hWnd, &rc))
		{
			RECT rcClient;

			//Display player text and turn
			if (GetClientRect(hWnd, &rcClient))
			{

				const WCHAR szPlayer1 [] = L"Player 1";
				const WCHAR szPlayer2[] = L"Player 2";

				SetBkMode(hdc, TRANSPARENT);
				//Draw player1 and player2 text
				SetTextColor(hdc, RGB(255, 0, 0));
				TextOut(hdc, 16, 16, szPlayer1, ARRAYSIZE(szPlayer2));
				SetTextColor(hdc, RGB(0, 255, 0));
				TextOut(hdc, rcClient.right - 72, 16, szPlayer2, ARRAYSIZE(szPlayer2));

				//DisPlay turn
				ShowTurn(hWnd, hdc);
			}

				FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
				//Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
		}

		for (int i = 0; i < 4; i++)
		{
			//Draw vartical lines
			DrawLine(hdc, rc.left + CELL_SIZE * i, rc.top, rc.left + CELL_SIZE * i, rc.bottom);
			//Draw Horizontal line
			DrawLine(hdc, rc.left, rc.top + CELL_SIZE * i, rc.right, rc.top + CELL_SIZE * i);
		}

		RECT rcCell;

		for (int i = 0; i < ARRAYSIZE(gameBoard); ++i)
		{
			if ((0 != gameBoard[i]) && GetCellRect(hWnd, i, &rcCell))
			{
				FillRect(hdc, &rcCell, (gameBoard[i] == 2) ? hbr2 : hbr1);
				playerTurn = (playerTurn == 1) ? 2 : 1;
			}
		}

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		DeleteObject(hbr1);
		DeleteObject(hbr2);
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
