// this is simple update programm for the windows
//
// launch with three arguments from the command-line:
// 1. DWORD dwProcessId - process identifier an existing process of destination programm
// 2. LPCTSTR lpFileDest - path to destination programm
// 3. LPCTSTR lpFileSrc - path to source (master) programm

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifndef UNICODE
#define UNICODE
#endif 


#include <Windows.h>
#include <CommCtrl.h>
#include <strsafe.h>
#include <vector>
#include <string>
#include "resource.h"
#include "Display.h"


typedef std::basic_string<TCHAR> generic_string;
typedef std::vector<generic_string> ParamVector;

namespace {
  //commandLine should contain path to n++ executable running
  ParamVector parseCommandLine(const TCHAR* commandLine) {
    ParamVector result;
    if (commandLine[0] != '\0') {
      int numArgs;
      LPWSTR* tokenizedCmdLine = CommandLineToArgvW(commandLine, &numArgs);
      if (tokenizedCmdLine != nullptr) {
        result.assign(tokenizedCmdLine, tokenizedCmdLine + numArgs);
        LocalFree(tokenizedCmdLine);
      }
    }
    return result;
  }
}

void ErrorShow(LPTSTR lpszFunction) {
  // Retrieve the system error message for the last-error code

  LPVOID lpMsgBuf;
  LPVOID lpDisplayBuf;
  DWORD dw = GetLastError();

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dw,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR)&lpMsgBuf,
    0, NULL);

  // Display the error message and exit the process
  lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
  StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf);
  MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
  //ExitProcess(dw);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_LBUTTONUP:
    MessageBox(hwnd, TEXT("WM_LBUTTONUP"), TEXT("event"), 0);
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

    EndPaint(hwnd, &ps);
  }
  break;

  default:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);

  }
  return 0;
}

BOOL InitApplication(HINSTANCE hinstance, LPCTSTR lpszClassName) {
  // Fill in the window class structure with parameters that describe the main window. 
  WNDCLASSEX wcx = {};

  wcx.cbSize = sizeof(wcx);                                                 // size of structure 
  wcx.style = CS_HREDRAW | CS_VREDRAW;                                      // class style 
  wcx.lpfnWndProc = WindowProc;                                             // window procedure 
  wcx.cbClsExtra = 0;                                                       // number of extra bytes to allocate following the window-class structure 
  wcx.cbWndExtra = 0;                                                       // number of extra bytes to allocate following the window instance 
  wcx.hInstance = hinstance;                                                // instance 
  wcx.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));  // class icon
  wcx.hCursor = LoadCursor(NULL, IDC_ARROW);                                // class cursor 
  wcx.hbrBackground = (HBRUSH)COLOR_WINDOW;                                 // class background brush 
  wcx.lpszMenuName = NULL;                                                  // menu  
  wcx.lpszClassName = lpszClassName;                                        // class name 
  wcx.hIconSm = (HICON)LoadImage(hinstance,                                 // small icon that is associated with the window clas 
    MAKEINTRESOURCE(IDI_ICON1),
    IMAGE_ICON,
    GetSystemMetrics(SM_CXSMICON),
    GetSystemMetrics(SM_CYSMICON),
    LR_DEFAULTCOLOR);

  // Register the window class. 
  return RegisterClassEx(&wcx);
}

BOOL InitInstance(HINSTANCE hinstance, int nCmdShow, LPCTSTR lpClassName) {
  int width = 300, height = 100;
  RECT rc;
  priDisplayRect(rc);
  centerRect(rc, width, height);

  // Create the main window 
  HWND hwnd = CreateWindowEx(
    0,                   // extended window style
    lpClassName,         // class name
    L"Update",           // window name
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // style of the window WS_OVERLAPPEDWINDOW
    rc.left,             // x
    rc.top,              // y  CW_USEDEFAULT
    width,               // width
    height,              // height
    (HWND)NULL,          // parent or owner window    
    (HMENU)NULL,         // menu
    hinstance,           // instance handle
    (LPVOID)NULL);       // no window-creation data 

  if (!hwnd) {
    ErrorShow(TEXT("InitInstance. Create the main window error."));
    return FALSE;
  }

  // Show the window and send a WM_PAINT message to the window procedure. 
  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  // Create static
  RECT rcClient;  // Client area of parent window.
  GetClientRect(hwnd, &rcClient);
  int offsetSC = (int)height / 10;

  HWND hwndSC = CreateWindowEx(
    0,
    WC_STATIC,
    (LPTSTR)NULL,
    WS_CHILD | WS_VISIBLE,
    rcClient.left + offsetSC,             // x
    rcClient.bottom - 40 - offsetSC,      // y
    rcClient.right - 2 * offsetSC,        // width
    14,                                   // height
    hwnd,
    (HMENU)0,
    hinstance,
    (LPVOID)NULL);

  if (!hwndSC) {
    ErrorShow(TEXT("InitInstance. Create static error."));
    return FALSE;
  }


  // Create progress bar
  int heightPB = GetSystemMetrics(SM_CYVSCROLL); // Height of scroll bar arrow.
  int offsetPB = (int)height / 10;

  HWND hwndPB = CreateWindowEx(
    0,
    PROGRESS_CLASS,
    (LPTSTR)NULL,
    WS_CHILD | WS_VISIBLE,
    rcClient.left + offsetPB,             // x
    rcClient.bottom - heightPB - offsetPB,// y
    rcClient.right - 2 * offsetPB,        // width
    heightPB,                             // height
    hwnd,
    (HMENU)0,
    hinstance,
    (LPVOID)NULL);

  if (!hwndPB) {
    ErrorShow(TEXT("InitInstance. Create progress bar error."));
    return FALSE;
  }

  DWORD cb = 200000;       // Size of file and count of bytes read.
  // Set the range and increment of the progress bar. 
  //SendMessage(hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); // cb / 2048
  SendMessage(hwndPB, PBM_SETPOS, (WPARAM)33, 0);
  //SendMessage(hwndPB, PBM_SETSTEP, (WPARAM)10, 0);
  //SendMessage(hwndPB, PBM_STEPIT, 0, 0);
  SendMessage(hwndSC, WM_SETTEXT, 0, (LPARAM)L"abracadabra");

  return TRUE;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
  const wchar_t CLASS_NAME[] = L"Update prog - ReNewC";
  ParamVector params = parseCommandLine(pCmdLine);

  if (!InitApplication(hInstance, CLASS_NAME))
    return 1;

  if (!InitInstance(hInstance, nCmdShow, CLASS_NAME))
    return 2;

  // Run the message loop.
  MSG msg = { 0 };
  BOOL bRet;
  while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
    if (bRet == -1) {
      // handle the error and possibly exit
      return 3;
    }
    else {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return 0;
}