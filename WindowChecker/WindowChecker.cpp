#define _WIN32_WINNT 0x0400
#pragma comment( lib, "user32.lib" )

#include "framework.h"
#include "WindowChecker.h"
#include <fstream>
#include <iostream>
#include <locale>
using namespace std;

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
HWND hMyWindow;                                      // окно приложения
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HHOOK hMouseHook;

HWND hFind;
HWND hSave;
HWND hWClass;
HWND hTitle;
HWND hPath;

TCHAR sWClass[MAX_LOADSTRING] = { 0 };
TCHAR sTitle[MAX_LOADSTRING] = { 0 };
TCHAR sPath[MAX_PATH] = { 0 };

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);


typedef BOOL(WINAPI* _QueryFullProcessImageName)(HANDLE, DWORD, LPTSTR, PDWORD);
_QueryFullProcessImageName QueryFullProcessImageNameWLocal;

//По движению мыши обновляем данные об окне, над которым находится курсор
__declspec(dllexport) LRESULT CALLBACK KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam)
{
    MOUSEHOOKSTRUCT* pMouseStruct = (MOUSEHOOKSTRUCT*)lParam;
    if (pMouseStruct != NULL) {
        HWND hwnd = WindowFromPoint(pMouseStruct->pt);
        if (hwnd != NULL) {
            while (GetParent(hwnd))
                hwnd = GetParent(hwnd);
            if (hwnd != hMyWindow) {
                GetWindowText(hwnd, sTitle, MAX_LOADSTRING);
                SetWindowText(hTitle, sTitle);
                GetClassName(hwnd, sWClass, MAX_LOADSTRING);
                SetWindowText(hWClass, sWClass);

                DWORD dwProcId = 0;

                GetWindowThreadProcessId(hwnd, &dwProcId);
                HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcId);

                if (hProc)
                {
                    DWORD maxPath = MAX_PATH;
                    QueryFullProcessImageNameWLocal(hProc, 0, sPath, &maxPath);
                    SetWindowText(hPath, sPath);
                    CloseHandle(hProc);
                }
            }

        }
    }
    return CallNextHookEx(hMouseHook,
        nCode, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HMODULE hMod = LoadLibrary(L"kernel32.dll");
    if (!hMod)
        return FALSE;
    QueryFullProcessImageNameWLocal = (_QueryFullProcessImageName)GetProcAddress(hMod, "QueryFullProcessImageNameW");

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWCHECKER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWCHECKER));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    if (hMouseHook != NULL) 
        UnhookWindowsHookEx(hMouseHook);
     
    
    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWCHECKER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWCHECKER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   hMyWindow = CreateWindowEx(WS_EX_TOPMOST, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 800, 200, nullptr, nullptr, hInstance, nullptr);

   if (!hMyWindow)
   {
      return FALSE;
   }

   ShowWindow(hMyWindow, nCmdShow);
   UpdateWindow(hMyWindow);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE: 
        {
            hFind = CreateWindow(TEXT("button"), TEXT("Найти окно"),

                WS_VISIBLE | WS_CHILD,
                10, 10, 90, 35,
                hWnd, 0, NULL, NULL
            );

            hSave = CreateWindow(TEXT("button"), TEXT("Сохранить"),

                WS_VISIBLE | WS_CHILD,
                110, 10, 90, 35,
                hWnd, 0, NULL, NULL
            );

            CreateWindow(TEXT("STATIC"), TEXT("Класс окна:"), 
                WS_VISIBLE | WS_CHILD,
                10, 55, 90, 22,
                hWnd, 0, NULL, NULL
            );

            CreateWindow(TEXT("STATIC"), TEXT("Заголовок окна:"),
                WS_VISIBLE | WS_CHILD,
                10, 87, 110, 22,
                hWnd, 0, NULL, NULL
            );

            CreateWindow(TEXT("STATIC"), TEXT("Приложение:"),
                WS_VISIBLE | WS_CHILD,
                10, 119, 90, 22,
                hWnd, 0, NULL, NULL
            );

            hWClass = CreateWindow(TEXT("STATIC"), TEXT(""),
                WS_VISIBLE | WS_CHILD,
                130, 55, 700, 22,
                hWnd, 0, NULL, NULL
            );

            hTitle = CreateWindow(TEXT("STATIC"), TEXT(""),
                WS_VISIBLE | WS_CHILD,
                130, 87, 700, 22,
                hWnd, 0, NULL, NULL
            );

            hPath = CreateWindow(TEXT("STATIC"), TEXT(""),
                WS_VISIBLE | WS_CHILD,
                130, 119, 700, 22,
                hWnd, 0, NULL, NULL
            );
        }
        break;
        case WM_COMMAND:
        {
            if (lParam == (LPARAM)hFind) {
                // Создаётся хук для мыши если ещё не создан.
                if (hMouseHook == NULL) {                
                    hMouseHook = SetWindowsHookEx(
                        WH_MOUSE_LL,
                        (HOOKPROC)KeyboardEvent,
                        hInst,
                        NULL
                    );
                }
                EnableWindow(hFind, false);
            }

            if (lParam == (LPARAM)hSave) {
                // сохраняем данные об окне в файле
                wofstream fout("window.txt", ios_base::app);
                fout.imbue(locale(""));

                //Path – D:\Users\Application\App.exe; Class – FrameDialog; Header – Настройки и опции
                wstring tmp = L"Path - " + wstring(sPath) + L"; Class - " + wstring(sWClass) + L"; Header - " + wstring(sTitle) +L"\n";
                fout << tmp;

                fout.close();
            }        
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