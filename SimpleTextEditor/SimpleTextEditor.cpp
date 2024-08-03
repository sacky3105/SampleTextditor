#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <richedit.h>  // Rich Editコントロールのヘッダーファイル
#include "resource.h"

wchar_t szClassName[] = L"SimpleTextEditor";
HWND hEdit;
HWND hButton;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void LoadFile(HWND, const wchar_t*);
void SaveFile(HWND, const wchar_t*);
void FindText(HWND);
void ReplaceText(HWND hEdit, const wchar_t* findText, const wchar_t* replaceText, BOOL replaceAll);
INT_PTR CALLBACK ReplaceDlgProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    wc.lpszClassName = szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        szClassName,
        L"Simple Text Editor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        LoadLibrary(TEXT("Msftedit.dll"));
        hEdit = CreateWindowEx(
            0, MSFTEDIT_CLASS, L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
            0, 50, 800, 500,
            hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
        if (!hEdit) {
            MessageBox(hwnd, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR);
        }
        hButton = CreateWindow(
            L"BUTTON",  // Predefined class; Unicode assumed
            L"Click Me", // Button text
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // Styles
            10,         // x position
            10,         // y position
            100,        // Button width
            30,         // Button height
            hwnd,       // Parent window
            (HMENU)IDC_BUTTON1, // No menu.
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
            NULL);      // Pointer not needed.

        if (!hButton) {
            MessageBox(hwnd, L"Could not create button.", L"Error", MB_OK | MB_ICONERROR);
        }
    }
                  break;
    case WM_SIZE: {
        MoveWindow(hEdit, 0, 50, LOWORD(lParam), HIWORD(lParam) - 50, TRUE);
    }
                break;
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_FILE_OPEN: {
            OPENFILENAME ofn;
            wchar_t szFile[260] = { 0 };

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
            ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn)) {
                LoadFile(hwnd, ofn.lpstrFile);
            }
        }
                         break;
        case ID_FILE_SAVE: {
            OPENFILENAME ofn;
            wchar_t szFile[260] = { 0 };

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
            ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetSaveFileName(&ofn)) {
                SaveFile(hwnd, ofn.lpstrFile);
            }
        }
                         break;
        case ID_EDIT_FIND: {
            FindText(hwnd);
        }
                         break;
        case ID_EDIT_REPLACE: {
            DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_REPLACE_DIALOG), hwnd, ReplaceDlgProc, (LPARAM)hEdit);
        }
                            break;
        case IDC_BUTTON1: {
            MessageBox(hwnd, L"Button clicked!", L"Info", MB_OK | MB_ICONINFORMATION);
        }
                        break;
        }
    }
                   break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void LoadFile(HWND hwnd, const wchar_t* path) {
    FILE* file;
    _wfopen_s(&file, path, L"rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        rewind(file);

        char* buffer = new char[fileSize + 1];
        fread(buffer, sizeof(char), fileSize, file);
        buffer[fileSize] = '\0';

        int wchars_num = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
        wchar_t* wstr = new wchar_t[wchars_num];
        MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wstr, wchars_num);

        SetWindowText(hEdit, wstr);

        delete[] buffer;
        delete[] wstr;
        fclose(file);
    }
}

void SaveFile(HWND hwnd, const wchar_t* path) {
    FILE* file;
    _wfopen_s(&file, path, L"wb");
    if (file) {
        int length = GetWindowTextLength(hEdit);
        wchar_t* wbuffer = new wchar_t[length + 1];
        GetWindowText(hEdit, wbuffer, length + 1);

        int chars_num = WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, NULL, 0, NULL, NULL);
        char* buffer = new char[chars_num];
        WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, buffer, chars_num, NULL, NULL);

        fwrite(buffer, sizeof(char), chars_num - 1, file);

        delete[] buffer;
        delete[] wbuffer;
        fclose(file);
    }
}

void FindText(HWND hwnd) {
    FINDREPLACE fr;
    ZeroMemory(&fr, sizeof(fr));
    fr.lStructSize = sizeof(fr);
    fr.hwndOwner = hwnd;
    fr.Flags = FR_DOWN;
    fr.lpstrFindWhat = new wchar_t[80];
    fr.wFindWhatLen = 80;

    HWND hFind = FindText(&fr);

    if (hFind) {
        int start = 0, end = 0;
        SendMessage(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);

        FINDTEXT ft;
        ft.chrg.cpMin = end;
        ft.chrg.cpMax = -1;
        ft.lpstrText = fr.lpstrFindWhat;

        int pos = SendMessage(hEdit, EM_FINDTEXT, fr.Flags, (LPARAM)&ft);
        if (pos != -1) {
            SendMessage(hEdit, EM_SETSEL, pos, pos + wcslen(fr.lpstrFindWhat));
            SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
        }
    }

    delete[] fr.lpstrFindWhat;
}

INT_PTR CALLBACK ReplaceDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;
    static wchar_t findText[100];
    static wchar_t replaceText[100];

    switch (message) {
    case WM_INITDIALOG:
        hEdit = (HWND)lParam;
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_REPLACE || LOWORD(wParam) == IDC_REPLACEALL) {
            GetDlgItemText(hDlg, IDC_FINDTEXT, findText, 100);
            GetDlgItemText(hDlg, IDC_REPLACETEXT, replaceText, 100);

            if (LOWORD(wParam) == IDC_REPLACE) {
                ReplaceText(hEdit, findText, replaceText, FALSE);
            }
            else if (LOWORD(wParam) == IDC_REPLACEALL) {
                ReplaceText(hEdit, findText, replaceText, TRUE);
            }
            return (INT_PTR)TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}

void ReplaceText(HWND hEdit, const wchar_t* findText, const wchar_t* replaceText, BOOL replaceAll) {
    int findTextLen = wcslen(findText);
    int replaceTextLen = wcslen(replaceText);
    CHARRANGE chrg;
    FINDTEXT ft;

    ft.chrg.cpMin = 0;
    ft.chrg.cpMax = -1;
    ft.lpstrText = (LPWSTR)findText;

    while (SendMessage(hEdit, EM_FINDTEXT, FR_DOWN, (LPARAM)&ft) != -1) {
        chrg.cpMin = ft.chrg.cpMin;
        chrg.cpMax = ft.chrg.cpMax;
        SendMessage(hEdit, EM_EXSETSEL, 0, (LPARAM)&chrg);
        SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)replaceText);

        if (!replaceAll) {
            break;
        }
        ft.chrg.cpMin = chrg.cpMin + replaceTextLen;
    }
}
