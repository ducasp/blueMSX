#include <windows.h>
#include "ToolInterface.h"
#include "Resource.h"
#include "Language.h"
#include <string>
#include <commctrl.h>
#include <sstream>
#include <iomanip>

using namespace std;

static HWND dbgHwnd = NULL;
static HWND hwndEdit = NULL;
static FILE* logFile = NULL;
static DWORD charCount = 0;
static LanguageId langId = LID_ENGLISH;

#define IDEDITCTL           100

#define MENU_FILE_EXIT              37100
#define MENU_FILE_LOG               37101
#define MENU_EDIT_SELECTALL         37200
#define MENU_EDIT_COPY              37201
#define MENU_EDIT_CLEAR             37202
#define MENU_HELP_ABOUT             37400

static void updateWindowMenu() 
{
    HMENU hMenuFile = CreatePopupMenu();
    
    AppendMenu(hMenuFile, MF_STRING, MENU_FILE_LOG, logFile == NULL ? Language::menuFileLogToFile : Language::menuFileStopLogToFile);
    AppendMenu(hMenuFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuFile, MF_STRING, MENU_FILE_EXIT, Language::menuFileExit);
    
    HMENU hMenuEdit = CreatePopupMenu();
    AppendMenu(hMenuEdit, MF_STRING, MENU_EDIT_SELECTALL, Language::menuEditSelectAll);
    AppendMenu(hMenuEdit, MF_STRING, MENU_EDIT_COPY, Language::menuEditCopy);
    AppendMenu(hMenuEdit, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenuEdit, MF_STRING, MENU_EDIT_CLEAR, Language::menuEditClearWindow);

    HMENU hMenuHelp = CreatePopupMenu();
    AppendMenu(hMenuHelp, MF_STRING, MENU_HELP_ABOUT, Language::menuHelpAbout);

    static HMENU hMenu = NULL;
    if (hMenu != NULL) {
        DestroyMenu(hMenu);
    }

    hMenu = CreateMenu();
    AppendMenu(hMenu, MF_POPUP, (UINT)hMenuFile, Language::menuFile);
    AppendMenu(hMenu, MF_POPUP, (UINT)hMenuEdit, Language::menuEdit);
    AppendMenu(hMenu, MF_POPUP, (UINT)hMenuHelp, Language::menuHelp);
    
    SetMenu(dbgHwnd, hMenu);
}

void openLogFile(HWND hwndOwner)
{
    OPENFILENAME ofn; 
    static char pFileName[MAX_PATH];
    static char buffer[0x20000];

    pFileName[0] = 0; 

    ofn.lStructSize = sizeof(OPENFILENAME); 
    ofn.hwndOwner = hwndOwner; 
    ofn.hInstance = GetDllHinstance();
    ofn.lpstrFilter = "*.TXT\0*.*\0\0"; 
    ofn.lpstrCustomFilter = NULL; 
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0; 
    ofn.lpstrFile = pFileName; 
    ofn.nMaxFile = 1024; 
    ofn.lpstrFileTitle = NULL; 
    ofn.nMaxFileTitle = 0; 
    ofn.lpstrInitialDir = NULL; 
    ofn.lpstrTitle = Language::openWindowCaption; 
    ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT; 
    ofn.nFileOffset = 0; 
    ofn.nFileExtension = 0; 
    ofn.lpstrDefExt = NULL; 
    ofn.lCustData = 0; 
    ofn.lpfnHook = NULL; 
    ofn.lpTemplateName = NULL; 

    char  curDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, curDir);

    BOOL rv = GetSaveFileName(&ofn); 

    SetCurrentDirectory(curDir);

    if (!rv) {
        return; 
    }

    logFile = fopen(pFileName, "wb");
}

static LRESULT CALLBACK wndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) 
{
    static HBRUSH hBrush = NULL;

    switch (iMsg) {
    case WM_CREATE:
        if (hBrush == NULL) {
            hBrush = CreateSolidBrush(RGB(255, 255, 255));
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case MENU_FILE_EXIT:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;

        case MENU_HELP_ABOUT:
            {
                char text[256];
                sprintf(text, "%s\r\n\r\n%s: " __DATE__ "\r\n\r\n%s    \r\n\r\n\r\n",
                    Language::traceWindowCaption, Language::aboutBuilt, Language::aboutVisit);
                MessageBox(NULL, text, Language::traceWindowCaption, MB_ICONINFORMATION | MB_OK);
            }
            return 0;

        case MENU_EDIT_CLEAR:
            charCount = 0;
            SendMessage(hwndEdit, EM_SETSEL, 0, INT_MAX-1);
            SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)"");
            return 0;

        case MENU_EDIT_SELECTALL:
            SendMessage(hwndEdit, EM_SETSEL, 0, INT_MAX-1);
            return 0;

        case MENU_EDIT_COPY:
            SendMessage(hwndEdit, WM_COPY, 0, 0);
            return 0;

        case MENU_FILE_LOG:
            if (logFile != NULL) {
                fclose(logFile);
            }
            else {
                openLogFile(hwnd);
            }
            updateWindowMenu();
            return 0;
        }
        break;

    case WM_CTLCOLORSTATIC:
        return (LRESULT)hBrush;

    case WM_SIZE:
        if (hwndEdit != NULL) {
            RECT r;
            GetClientRect(dbgHwnd, &r);
            SetWindowPos(hwndEdit, NULL, r.left, r.top, r.right, r.bottom, SWP_NOZORDER);
        }
        break;
        
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
        }
        return 0;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        if (logFile != NULL) {
            fclose(logFile);
        }
        logFile = NULL;
        dbgHwnd = NULL;
		break;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void OnCreateTool() {
    WNDCLASSEX wndClass;

    wndClass.cbSize         = sizeof(wndClass);
    wndClass.style          = 0;
    wndClass.lpfnWndProc    = wndProc;
    wndClass.cbClsExtra     = 0;
    wndClass.cbWndExtra     = 0;
    wndClass.hInstance      = GetDllHinstance();
    wndClass.hIcon          = NULL;
    wndClass.hIconSm        = NULL;
    wndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground  = NULL;
    wndClass.lpszMenuName   = NULL;
    wndClass.lpszClassName  = "TraceWindow";

    RegisterClassEx(&wndClass);
    }

void OnDestroyTool() {
    if (dbgHwnd != NULL) {
        DestroyWindow(dbgHwnd);
    }
}

void OnShowTool() {
    if (dbgHwnd != NULL) {
        return;
    }

    Language::SetLanguage(langId);

    charCount = 0;

    dbgHwnd = CreateWindow("TraceWindow", Language::traceWindowCaption, 
                           WS_OVERLAPPEDWINDOW, 
                           CW_USEDEFAULT, CW_USEDEFAULT, 600, 440, NULL, NULL, GetDllHinstance(), NULL);

    void* hEditDS = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE, 256L);
      if (hEditDS == NULL) {
         hEditDS = GetDllHinstance();
      }
   
      hwndEdit = CreateWindow("edit", NULL, 
          WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | 
          ES_AUTOHSCROLL | ES_READONLY |
         ES_AUTOVSCROLL, 10, 10, 250, 200, dbgHwnd, (HMENU)IDEDITCTL, (HINSTANCE)hEditDS, NULL);

      // Now limit the text to the maximum possible amount.
      //
      SendMessage(hwndEdit, EM_LIMITTEXT, 0, 0);

    ShowWindow(dbgHwnd, TRUE);

    updateWindowMenu();
}

void OnEmulatorStart() {
}

void OnEmulatorStop() {
}

void OnEmulatorPause() {
}

void OnEmulatorResume() {
}

void OnEmulatorReset() {
}

void OnEmulatorTrace(const char* message)
{
    // Convert buffer from unix to dos format for printing
    char buffer[256];
    int index = 0;
    for (; message[0] != 0 && index < 254; message++) {
        if (message[0] == '\r' && message[1] == '\n') {
            continue; // Add \r later
        }
        if (message[0] == '\n') {
            buffer[index++] = '\r';
        }
        buffer[index++] = message[0];
    }
    buffer[index] = 0;
    
    if (logFile != NULL) {
        fwrite(buffer, 1, strlen(buffer), logFile);
    }

    if (hwndEdit != NULL) {
        DWORD start;
        DWORD end;
        SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);

        if (charCount >= 32000) {
            SendMessage(hwndEdit, EM_SETSEL, 0, 1000);
            SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)"");
            SendMessage(hwndEdit, EM_SETSEL, 0, INT_MAX-1);
            SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&charCount);
        }

        charCount += index;

        SendMessage(hwndEdit, EM_SETSEL, INT_MAX, INT_MAX-1);
        SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)buffer);
        SendMessage(hwndEdit, EM_SETSEL, start, end);
    }
}

void OnEmulatorSetBreakpoint(UInt16 address) {
}

void OnEmulatorSetBreakpoint(UInt16 slot, UInt16 address) {
}

void OnEmulatorSetBreakpoint(UInt16 slot, UInt16 page, UInt16 address) {
}

const char* OnGetName() {
    return Language::traceWindowName;
}

void OnSetLanguage(LanguageId languageId)
{
    langId = languageId;
    Language::SetLanguage(langId);
}
