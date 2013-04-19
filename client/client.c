#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define ID_B1 1000             // �v�b�V���{�^����ID
#define ID_B2 (ID_B1+1)  // �v�b�V���{�^����ID
#define ID_B3 (ID_B2+1)  // �v�b�V���{�^����ID
#define ID_E1 2000
#define ID_E2 (ID_E1+1)
#define ID_E3 (ID_E2+1)
#define WM_SOCKET 10000
#define IDC_STATIC    3000
#define IDC_EDIT1    4000


HWND Create(HINSTANCE hInst);
HINSTANCE g_hInst, hInst1;    // �C���X�^���X�n���h��
HWND hEdit , hTl;
WNDPROC Org_Key; //�I���W�i���v���V�[�W���̃A�h���X
char strText1[32768] , error1[256],edit_str[256]; 
SOCKET sock;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK KeySubProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK MyDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp);
SOCKET InitializeWinsock(char *deststr);
void Winsock_Send();

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int showCmd)
{
  WNDCLASSEX wc;
  HWND hWnd;
  MSG msg;
  g_hInst = hInst;

  // �E�B���h�E�N���X�̏���ݒ�
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
  wc.lpszMenuName = NULL;                     // ���j���[��
  wc.lpszClassName = _T("Default Class Name");// �E�B���h�E�N���X��

  // �E�B���h�E�N���X��o�^����
  if( RegisterClassEx( &wc ) == 0 ){ return 1; }

  // �E�B���h�E���쐬����
  hWnd = CreateWindow(
      wc.lpszClassName,      // �E�B���h�E�N���X��
      _T("winsock_client"),  // �^�C�g���o�[�ɕ\�����镶����
      WS_OVERLAPPEDWINDOW,   // �E�B���h�E�̎��
      CW_USEDEFAULT,         // �E�B���h�E��\������ʒu�iX���W�j
      CW_USEDEFAULT,         // �E�B���h�E��\������ʒu�iY���W�j
      CW_USEDEFAULT,         // �E�B���h�E�̕�
      CW_USEDEFAULT,         // �E�B���h�E�̍���
      NULL,                  // �e�E�B���h�E�̃E�B���h�E�n���h��
      NULL,                  // ���j���[�n���h��
      hInst,                 // �C���X�^���X�n���h��
      NULL                   // ���̑��̍쐬�f�[�^
      );
  if( hWnd == NULL ){ return 1; }

  // �E�B���h�E��\������
  ShowWindow( hWnd, SW_SHOW );
  UpdateWindow( hWnd );

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

HWND CreatePushButton(HWND hWnd, int x, int y, int w, int h, LPCTSTR caption, int id)
{
  return CreateWindow(
      _T("BUTTON"),                            // �E�B���h�E�N���X��
      caption,                                 // �L���v�V����
      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,   // �X�^�C���w��
      x, y,                                    // ���W
      w, h,                                    // �T�C�Y
      hWnd,                                    // �e�E�B���h�E�̃n���h��
      (HMENU)(INT_PTR)id,                      // ���j���[�n���h���i������ID�w��ɗ��p�ł���j
      g_hInst,                                 // �C���X�^���X�n���h��
      NULL                                     // ���̑��̍쐬�f�[�^
      );
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
  RECT rc;
  static HWND hButton1;
  static HMENU hMenu;
  char buf[32768];
  int nLen, idDia, line = 0;
  static int hTlfor;

  switch( msg ){
    case WM_SOCKET:
      switch (WSAGETSELECTEVENT(lp)) {
        case FD_READ:
          nLen = recv(sock, buf, sizeof(buf), 0);
          if ( nLen < 0) {
            _stprintf(error1,_T("recv : %d\n"), WSAGetLastError());
            MessageBox(NULL, error1, NULL, MB_ICONWARNING);
            return 0;
          }
          strncat(strText1,buf,strlen(buf));
          if(hTlfor == 1)    //�t�H�[�J�X������Ƃ�
            line = SendMessage(hTl, EM_GETFIRSTVISIBLELINE, 0, 0);
          if(hTlfor == 0)    //�t�H�[�J�X���Ȃ��Ƃ�
            line = SendMessage(hTl, EM_GETLINECOUNT, 0, 0);
          SetWindowText(hTl,strText1);
          SendMessage(hTl, EM_LINESCROLL, 0, line);
          break;
        case FD_CLOSE:
          MessageBox(NULL, _T("�T�[�o�[�Ƃ̐ڑ����ؒf����܂����B"), _T("OK"), MB_OK);
          break;
      }
      return 0;

    case WM_CREATE:
      hButton1 = CreatePushButton( hWnd, rc.right-60,  rc.bottom-100, 60, 40, _T("���M"), ID_B1 );
      GetClientRect(hWnd, &rc);
      hEdit = CreateWindow(_T("EDIT"),NULL,WS_CHILD | WS_VISIBLE |
          WS_VSCROLL | ES_AUTOVSCROLL |
          ES_LEFT | ES_MULTILINE ,
          0 , rc.bottom-100 ,rc.right-60, 100 ,hWnd,(HMENU)ID_E1,g_hInst,NULL
          );
      //�E�B���h�E�̃T�u�N���X��
      Org_Key = (WNDPROC)GetWindowLong(hEdit, GWL_WNDPROC);
      SetWindowLong(hEdit, GWL_WNDPROC, (LONG)KeySubProc);

      hTl = CreateWindow(_T("EDIT") , NULL ,WS_CHILD | WS_VISIBLE |
          WS_VSCROLL | ES_AUTOVSCROLL |
          ES_LEFT | ES_MULTILINE |ES_READONLY ,
          0 , 0 , rc.right ,rc.bottom-100 , hWnd , (HMENU)ID_E2 ,g_hInst, NULL
          );
      hMenu = LoadMenu(NULL, "WIN_MENU"); // ���j���[���\�[�X�����[�h
      SetMenu(hWnd, hMenu);               // �E�B���h�E�Ƀ��j���[���\�[�X���蓖��
      return 0;
    case WM_SIZE: 
      GetClientRect(hWnd, &rc); 
      MoveWindow(hEdit, 0 , rc.bottom-100 ,rc.right-60, 100, TRUE);
      MoveWindow(hTl ,0 ,0 , rc.right ,rc.bottom-100 , TRUE); 
      MoveWindow(hButton1,rc.right-60, rc.bottom-100, 60, 40,TRUE);
      break;
    case WM_COMMAND:
      switch( LOWORD( wp ) ){
        case ID_E2:
          switch (HIWORD(wp)){
            case EN_SETFOCUS:
              hTlfor = 1;
              break;
            case EN_KILLFOCUS:
              hTlfor = 0;
              break;
          }
          break;
        case ID_B1:
          Winsock_Send();
          break;
        case 40001:
          if( MessageBox( hWnd, _T("�I�����Ă���낵���ł����H"), _T("�m�F"), MB_YESNO ) == IDYES ){
            SetMenu(hWnd, NULL);  // �E�B���h�E���烁�j���[�폜
            DestroyMenu(hMenu);   // ���j���[�o�[���\�[�X�j��
            hMenu = NULL;
            DestroyWindow(hWnd); // �I��
          }
          break;
        case 40002 : 	//�T�[�o�[�ɐڑ�
          idDia = DialogBox(hInst1, "OPTDLG", hWnd, (DLGPROC)MyDlgProc);
          if (idDia == IDOK) {
            sock = InitializeWinsock(edit_str);
            if (sock == INVALID_SOCKET){
              return -1;
            }
            WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
          }
          break;
        case 40004: MessageBox(NULL, _T("ver1.00��"), _T("�o�[�W�������"), MB_OK);
                    break;
        case 40003: MessageBox(NULL, _T("Shift�L�[+Enter�L�[�ő��M"), _T("�������"), MB_OK);
                    break;
      }
      break;
    case WM_CLOSE:    // �E�B���h�E�������悤�Ƃ����Ƃ��ɑ����Ă���
      if( MessageBox( hWnd, _T("�I�����Ă���낵���ł����H"), _T("�m�F"), MB_YESNO ) == IDYES ){
        SetMenu(hWnd, NULL);  // �E�B���h�E���烁�j���[�폜
        DestroyMenu(hMenu);   // ���j���[�o�[���\�[�X�j��
        hMenu = NULL;
      }
      break;                // �E�B���h�E����鏈����DefWindowProc()�ɔC����
    case WM_DESTROY:  // �E�B���h�E���j�����ꂽ��
      if (sock != INVALID_SOCKET) {
        shutdown(sock, SD_BOTH);
        closesocket(sock);
        WSACleanup();
      }
      PostQuitMessage(0);
      return 0;
  }

  // ���̃��b�Z�[�W�̓f�t�H���g�̏������s��
  return DefWindowProc(hWnd, msg, wp, lp);
}

LRESULT CALLBACK KeySubProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
  static char b_enter=0;

  switch (msg) { 
    case WM_KEYDOWN:
      if (wp == VK_RETURN){
        if (GetKeyState(VK_SHIFT) & 0x80){
          Winsock_Send();	
          b_enter = 1;
          return 0; //���b�Z�[�W�̏�����������O��Ԃ� 
        }

      }
      break;
    case WM_CHAR:
      if (wp == VK_RETURN && b_enter){
        b_enter = 0;
        return 0;
      }
      break;
    default:
      break;
  } 
  // �T�u�N���X�ŏ������Ȃ��������b�Z�[�W�́A�{���̃E�B���h�E�v���V�[�W���ɏ������Ă��炤
  return CallWindowProc(Org_Key, hWnd, msg, wp, lp);
}

LRESULT CALLBACK MyDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
  switch (msg) {
    case WM_INITDIALOG:
      SetDlgItemText(hDlgWnd, IDC_EDIT1, (LPCTSTR)edit_str);
      return TRUE;
    case WM_COMMAND:
      switch (LOWORD(wp)) {
        case IDOK:
          GetDlgItemText(hDlgWnd, IDC_EDIT1, (LPTSTR)edit_str, sizeof(edit_str));
          EndDialog(hDlgWnd,IDOK);
          break;
        case IDCANCEL:
          EndDialog(hDlgWnd, IDCANCEL);
          break;
        default:
          return FALSE;
      }
    default:
      return FALSE;
  }
  return TRUE;
}

SOCKET InitializeWinsock(char *deststr)
{
  WSADATA wsaData;
  struct sockaddr_in server;
  unsigned int **addrptr;

  if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
    _stprintf(error1,_T("WSAStartup failed\n"));
    MessageBox(NULL, error1, NULL, MB_ICONWARNING);
    WSACleanup();
    return INVALID_SOCKET;
  }

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    _stprintf(error1,_T("socket : %d\n"), WSAGetLastError());
    MessageBox(NULL, error1, NULL, MB_ICONWARNING);
    WSACleanup();
    return INVALID_SOCKET;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(12345);
  server.sin_addr.S_un.S_addr = inet_addr(deststr);

  if (server.sin_addr.S_un.S_addr == 0xffffffff) {
    struct hostent *host;
    host = gethostbyname(deststr);
    if (host == NULL) {
      if (WSAGetLastError() == WSAHOST_NOT_FOUND) {
        _stprintf(error1,_T("host not found : %s\n"), deststr);
        MessageBox(NULL, error1, NULL, MB_ICONWARNING);
      }
      WSACleanup();
      return INVALID_SOCKET;
    }

    addrptr = (unsigned int **)host->h_addr_list;

    while (*addrptr != NULL) {
      server.sin_addr.S_un.S_addr = *(*addrptr);
      if (connect(sock,(struct sockaddr *)&server,sizeof(server)) == 0) {
        break;
      }
      addrptr++;
    }
    if (*addrptr == NULL) {
      _stprintf(error1,_T("connect : %d\n"), WSAGetLastError());
      MessageBox(NULL, error1, NULL, MB_ICONWARNING);
      WSACleanup();
      return INVALID_SOCKET;
    }
  }
  else {
    if (connect(sock,(struct sockaddr *)&server,sizeof(server)) != 0) {
      _stprintf(error1,_T("connect : %d\n"), WSAGetLastError());
      MessageBox(NULL, error1, NULL, MB_ICONWARNING);
      closesocket(sock);
      WSACleanup();
      return INVALID_SOCKET;
    }
  }
  return sock;
}

void Winsock_Send()   //���M����
{					
  int nResult;
  char *usData;

  usData = calloc(32768,sizeof(char));
  GetWindowText(hEdit, usData, 32768);
  strcat(usData,"\r\n");
  nResult = send(sock,usData, strlen(usData)+1, 0);
  if (nResult < 1) {
    _stprintf(error1,_T("send : %d\n"), WSAGetLastError());
    MessageBox(NULL, error1, NULL, MB_ICONWARNING);
    free(usData);
    return;
  }
  SetWindowText(hEdit,NULL);
  free(usData);

}
