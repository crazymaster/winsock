#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <windows.h>

#define WM_SOCKET 10000

char error[256];

// �v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
SOCKET InitializeWinsock();
int GetSocketIndex(SOCKET sock, SOCKET sockServer[], int nMaxSocketCount);


// �G���g���|�C���g
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int showCmd)
{
  WNDCLASSEX wc;  // �E�B���h�E�N���X�\����
  HWND hWnd;      // �E�B���h�E�n���h��
  MSG msg;        // ���b�Z�[�W�\����

  // �E�B���h�E�N���X����ݒ�(�\����)
  wc.cbSize = sizeof(wc);               // �\���̃T�C�Y
  wc.style = CS_HREDRAW | CS_VREDRAW;   // �X�^�C��
  wc.lpfnWndProc = WndProc;             // �E�B���h�E�v���V�[�W��
  wc.cbClsExtra = 0;                    // �g�����P
  wc.cbWndExtra = 0;                    // �g�����Q
  wc.hInstance = hInst;                 // �C���X�^���X�n���h��
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // �A�C�R��
  wc.hIconSm = wc.hIcon;                // �q�A�C�R��
  wc.hCursor = LoadCursor(NULL, IDC_ARROW); // �}�X�J�[�\��
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // �E�B���h�E�w�i
  wc.lpszMenuName = NULL;               // ���j���[��
  wc.lpszClassName = "fooWindowClass";  // �E�B���h�E�N���X��

  // �E�B���h�E�N���X�̓o�^
  if(RegisterClassEx(&wc) == 0){ return 1; }

  // �E�B���h�E���쐬
  hWnd = CreateWindow(
      wc.lpszClassName,      // �E�B���h�E�N���X��
      "winsock_server",       // �^�C�g���o�[�ɕ\�����镶����
      WS_OVERLAPPEDWINDOW,   // �E�B���h�E�̎��
      CW_USEDEFAULT,         // �E�B���h�E��\������X���W
      CW_USEDEFAULT,         // �E�B���h�E��\������Y���W
      CW_USEDEFAULT,         // �E�B���h�E��
      CW_USEDEFAULT,         // �E�B���h�E����
      NULL,                  // �e�E�B���h�E�̃E�B���h�E�n���h��
      NULL,                  // ���j���[�n���h��
      hInst,                 // �C���X�^���X�n���h��
      NULL                   // ���̑��̍쐬�f�[�^
      );
  if(hWnd == NULL){ return 1; }

  // �E�B���h�E��\��
  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);

  // ���b�Z�[�W���[�v
  while(1) {
    BOOL ret = GetMessage(&msg, NULL, 0, 0);  // ���b�Z�[�W���擾����
    if(ret == 0 || ret == -1) { // �A�v���P�[�V�����I���܂���GetMessage()���s�Ȃ�
      break;
    } else {
      // ���b�Z�[�W����������
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return 0;
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
  static HMENU hMenu;
  static int nMaxSocketCount = 64;
  static SOCKET sockListen = INVALID_SOCKET;
  static SOCKET sockServer[64];
  static HWND   hWndListBox = NULL;
  struct sockaddr_in client;
  struct hostent *host;
  char szBuf[256], szData[32768], szData2[32768];
  static char szHostName[64][256];
  int i, len;

  switch(msg) {
    case WM_SOCKET:
      switch (WSAGETSELECTEVENT(lp)){
        case FD_ACCEPT:
          i = GetSocketIndex(INVALID_SOCKET, sockServer, nMaxSocketCount);
          len = sizeof(client);
          sockServer[i] = accept(sockListen ,(struct sockaddr *)&client, &len);
          WSAAsyncSelect(sockServer[i], hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
          host = gethostbyaddr((const char *)&client.sin_addr,sizeof(client.sin_addr), AF_INET);
          strcpy(szHostName[i], host->h_name);
          _stprintf(szBuf, _T("No%d(%s) �ڑ�"), i + 1,szHostName[i]);
          SendMessage(hWndListBox, LB_ADDSTRING, 0, (LPARAM)szBuf);
          break;
        case FD_READ:
          i = GetSocketIndex((SOCKET)wp, sockServer, nMaxSocketCount);
          recv(sockServer[i], szData,sizeof(szData) , 0);
          _stprintf(szBuf, _T("No%d(%s) ���b�Z�[�W"), i + 1,szHostName[i]);
          SendMessage(hWndListBox, LB_ADDSTRING, 0, (LPARAM)szBuf);
          _stprintf(szData2, _T("%s: "),szHostName[i]);
          strncat(szData2,szData,strlen(szData));
          for (i = 0; i < nMaxSocketCount; i++){
            send(sockServer[i], szData2, strlen(szData2)+1, 0);
          }
          break;
        case FD_CLOSE:
          i = GetSocketIndex((SOCKET)wp, sockServer, nMaxSocketCount);

          _stprintf(szBuf, TEXT("No%d �ؒf"), i + 1);
          SendMessage(hWndListBox, LB_ADDSTRING, 0, (LPARAM)szBuf);

          shutdown(sockServer[i], SD_BOTH);
          closesocket(sockServer[i]);
          sockServer[i] = INVALID_SOCKET;
          break;
      }
      break;

    case WM_CREATE:   // �E�B���h�E���쐬���ꂽ��
      hMenu = LoadMenu(NULL, "WIN_MENU"); // ���j���[���\�[�X�����[�h
      SetMenu(hWnd, hMenu);               // �E�B���h�E�Ƀ��j���[���\�[�X���蓖��
      hWndListBox = CreateWindowEx(0, _T("LISTBOX"), NULL,
          WS_CHILD | WS_VISIBLE | WS_VSCROLL,
          0, 0, 0, 0, hWnd, (HMENU)1, ((LPCREATESTRUCT)lp)->hInstance, NULL);

      sockListen = InitializeWinsock();
      if (sockListen == INVALID_SOCKET)
        return -1;

      for (i = 0; i < nMaxSocketCount; i++)
        sockServer[i] = INVALID_SOCKET;

      WSAAsyncSelect(sockListen, hWnd, WM_SOCKET, FD_ACCEPT);

      break;
    case WM_SIZE:
      MoveWindow(hWndListBox, 0, 0, LOWORD(lp), HIWORD(lp), TRUE);
      break;
    case WM_COMMAND:  // ���j���[���I�����ꂽ��
      switch ( LOWORD(wp)) {

        case 40003: DestroyWindow(hWnd); // �I��
                    break;
        case 40013: MessageBox(NULL, "ver1.00��", "�o�[�W�������", MB_OK);
                    break;
      }
      break;

    case WM_CLOSE:    // �E�B���h�E��������Ƃ�
      SetMenu(hWnd, NULL);  // �E�B���h�E���烁�j���[�폜
      DestroyMenu(hMenu);   // ���j���[�o�[���\�[�X�j��
      hMenu = NULL;
      break;                // �E�B���h�E����鏈����DefWindowProc()�ɔC����

    case WM_DESTROY:  // �E�B���h�E���j�����ꂽ��
      for (i = 0; i < nMaxSocketCount; i++) {
        if (sockServer[i] != INVALID_SOCKET) {
          shutdown(sockServer[i], SD_BOTH);
          closesocket(sockServer[i]);
        }
      }
      if (sockListen != INVALID_SOCKET) {
        closesocket(sockListen);
        WSACleanup();
      }
      PostQuitMessage(0);
      return 0;
  }	
  // ���̃��b�Z�[�W�̓f�t�H���g�̏������s��
  return DefWindowProc(hWnd, msg, wp, lp);
}

SOCKET InitializeWinsock()
{
  WSADATA wsaData;
  struct sockaddr_in addr;
  SOCKET sockListen;
  BOOL yes = 1;

  if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
    _stprintf(error,_T("WSAStartup failed\n"));
    MessageBox(NULL, error, NULL, MB_ICONWARNING);
    return INVALID_SOCKET;
  }

  sockListen = socket(AF_INET,SOCK_STREAM,0);
  if(sockListen == INVALID_SOCKET){
    _stprintf(error,_T("socket : %d\n"),WSAGetLastError());
    WSACleanup();
    MessageBox(NULL, error, NULL, MB_ICONWARNING);
    return INVALID_SOCKET;
  }
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  addr.sin_addr.S_un.S_addr = INADDR_ANY;
  setsockopt(sockListen,SOL_SOCKET,SO_REUSEADDR,(const char *)&yes,sizeof(yes));

  if (bind(sockListen, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
    _stprintf(error,_T("bind : %d\n"), WSAGetLastError());
    closesocket(sockListen);
    WSACleanup();
    MessageBox(NULL, error, NULL, MB_ICONWARNING);
    return INVALID_SOCKET;
  }

  if (listen(sockListen, 5) != 0) {
    _stprintf(error,_T("listen : %d\n"), WSAGetLastError());
    closesocket(sockListen);
    WSACleanup();
    MessageBox(NULL, error, NULL, MB_ICONWARNING);
    return INVALID_SOCKET;
  }

  return sockListen;
}

int GetSocketIndex(SOCKET sock, SOCKET sockServer[], int nMaxSocketCount)
{
  int i;

  for (i = 0; i < nMaxSocketCount; i++) {
    if (sockServer[i] == sock)
      break;
  }

  return i;
}
