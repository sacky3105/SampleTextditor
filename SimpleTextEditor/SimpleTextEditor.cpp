#include <windows.h>
#include <commdlg.h>
#include <stdio.h>  // �t�@�C������ɕK�v�ȃw�b�_�t�@�C���̃C���N���[�h
#include "resource.h"  // ���\�[�X�w�b�_�t�@�C���̃C���N���[�h

// �O���[�o���ϐ�
wchar_t szClassName[] = L"SimpleTextEditor";
HWND hEdit;

// �v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void LoadFile(HWND, const wchar_t*);
void SaveFile(HWND, const wchar_t*);

// WinMain�֐�
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    // �E�B���h�E�N���X�̐ݒ�
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);  // ���j���[�̎w��
    wc.lpszClassName = szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // �E�B���h�E�N���X�̓o�^
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // �E�B���h�E�̍쐬
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

    // �E�B���h�E�̕\���ƍX�V
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // ���b�Z�[�W���[�v
    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // �G�f�B�b�g�R���g���[���̍쐬
        hEdit = CreateWindowEx(
            0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
            0, 0, 800, 600,
            hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
    }
                  break;
    case WM_SIZE: {
        // �E�B���h�E�T�C�Y�ύX���ɃG�f�B�b�g�R���g���[���̃T�C�Y�𒲐�
        MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
    }
                break;
    case WM_COMMAND: {
        // ���j���[�R�}���h�̏���
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

// �t�@�C���ǂݍ��ݏ���
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

        // �}���`�o�C�g����������C�h������ɕϊ�
        int wchars_num = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
        wchar_t* wstr = new wchar_t[wchars_num];
        MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wstr, wchars_num);

        SetWindowText(hEdit, wstr);

        delete[] buffer;
        delete[] wstr;
        fclose(file);
    }
}

// �t�@�C���ۑ�����
void SaveFile(HWND hwnd, const wchar_t* path) {
    FILE* file;
    _wfopen_s(&file, path, L"wb");
    if (file) {
        int length = GetWindowTextLength(hEdit);
        wchar_t* wbuffer = new wchar_t[length + 1];
        GetWindowText(hEdit, wbuffer, length + 1);

        // ���C�h��������}���`�o�C�g������ɕϊ�
        int chars_num = WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, NULL, 0, NULL, NULL);
        char* buffer = new char[chars_num];
        WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, buffer, chars_num, NULL, NULL);

        fwrite(buffer, sizeof(char), chars_num - 1, file);

        delete[] buffer;
        delete[] wbuffer;
        fclose(file);
    }
}

