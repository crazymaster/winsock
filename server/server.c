#include <winsock2.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <windows.h>

#define WM_SOCKET 10000

char error[256];

// プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
SOCKET InitializeWinsock();
int GetSocketIndex(SOCKET sock, SOCKET sockServer[], int nMaxSocketCount);


// エントリポイント
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int showCmd)
{
  WNDCLASSEX wc;  // ウィンドウクラス構造体
  HWND hWnd;      // ウィンドウハンドル
  MSG msg;        // メッセージ構造体

  // ウィンドウクラス情報を設定(構造体)
  wc.cbSize = sizeof(wc);               // 構造体サイズ
  wc.style = CS_HREDRAW | CS_VREDRAW;   // スタイル
  wc.lpfnWndProc = WndProc;             // ウィンドウプロシージャ
  wc.cbClsExtra = 0;                    // 拡張情報１
  wc.cbWndExtra = 0;                    // 拡張情報２
  wc.hInstance = hInst;                 // インスタンスハンドル
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // アイコン
  wc.hIconSm = wc.hIcon;                // 子アイコン
  wc.hCursor = LoadCursor(NULL, IDC_ARROW); // マスカーソル
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // ウィンドウ背景
  wc.lpszMenuName = NULL;               // メニュー名
  wc.lpszClassName = "fooWindowClass";  // ウィンドウクラス名

  // ウィンドウクラスの登録
  if(RegisterClassEx(&wc) == 0){ return 1; }

  // ウィンドウを作成
  hWnd = CreateWindow(
      wc.lpszClassName,      // ウィンドウクラス名
      "winsock_server",       // タイトルバーに表示する文字列
      WS_OVERLAPPEDWINDOW,   // ウィンドウの種類
      CW_USEDEFAULT,         // ウィンドウを表示するX座標
      CW_USEDEFAULT,         // ウィンドウを表示するY座標
      CW_USEDEFAULT,         // ウィンドウ幅
      CW_USEDEFAULT,         // ウィンドウ高さ
      NULL,                  // 親ウィンドウのウィンドウハンドル
      NULL,                  // メニューハンドル
      hInst,                 // インスタンスハンドル
      NULL                   // その他の作成データ
      );
  if(hWnd == NULL){ return 1; }

  // ウィンドウを表示
  ShowWindow(hWnd, SW_SHOW);
  UpdateWindow(hWnd);

  // メッセージループ
  while(1) {
    BOOL ret = GetMessage(&msg, NULL, 0, 0);  // メッセージを取得する
    if(ret == 0 || ret == -1) { // アプリケーション終了またはGetMessage()失敗なら
      break;
    } else {
      // メッセージを処理する
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return 0;
}

// ウィンドウプロシージャ
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
          _stprintf(szBuf, _T("No%d(%s) 接続"), i + 1,szHostName[i]);
          SendMessage(hWndListBox, LB_ADDSTRING, 0, (LPARAM)szBuf);
          break;
        case FD_READ:
          i = GetSocketIndex((SOCKET)wp, sockServer, nMaxSocketCount);
          recv(sockServer[i], szData,sizeof(szData) , 0);
          _stprintf(szBuf, _T("No%d(%s) メッセージ"), i + 1,szHostName[i]);
          SendMessage(hWndListBox, LB_ADDSTRING, 0, (LPARAM)szBuf);
          _stprintf(szData2, _T("%s: "),szHostName[i]);
          strncat(szData2,szData,strlen(szData));
          for (i = 0; i < nMaxSocketCount; i++){
            send(sockServer[i], szData2, strlen(szData2)+1, 0);
          }
          break;
        case FD_CLOSE:
          i = GetSocketIndex((SOCKET)wp, sockServer, nMaxSocketCount);

          _stprintf(szBuf, TEXT("No%d 切断"), i + 1);
          SendMessage(hWndListBox, LB_ADDSTRING, 0, (LPARAM)szBuf);

          shutdown(sockServer[i], SD_BOTH);
          closesocket(sockServer[i]);
          sockServer[i] = INVALID_SOCKET;
          break;
      }
      break;

    case WM_CREATE:   // ウィンドウが作成されたら
      hMenu = LoadMenu(NULL, "WIN_MENU"); // メニューリソースをロード
      SetMenu(hWnd, hMenu);               // ウィンドウにメニューリソース割り当て
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
    case WM_COMMAND:  // メニューが選択されたら
      switch ( LOWORD(wp)) {

        case 40003: DestroyWindow(hWnd); // 終了
                    break;
        case 40013: MessageBox(NULL, "ver1.00β", "バージョン情報", MB_OK);
                    break;
      }
      break;

    case WM_CLOSE:    // ウィンドウが閉じられるとき
      SetMenu(hWnd, NULL);  // ウィンドウからメニュー削除
      DestroyMenu(hMenu);   // メニューバーリソース破棄
      hMenu = NULL;
      break;                // ウィンドウを閉じる処理はDefWindowProc()に任せる

    case WM_DESTROY:  // ウィンドウが破棄されたら
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
  // 他のメッセージはデフォルトの処理を行う
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
