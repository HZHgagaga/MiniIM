// IMclient.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../resource.h"
#include "../Protocol.h"
#pragma comment(lib, "ws2_32.lib")


BOOL CALLBACK DigLoginProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DigCreateProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DigMenuProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DigFindProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DigSayProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DigPleaseProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ServerThreadProc(LPVOID lParam);
void CleanFriendList();

SOCKET sClient;
sockaddr_in sin;
BOOL g_bSever = TRUE;
BOOL g_bSay = TRUE;
BOOL bLogin = FALSE;
BOOL b_Please = FALSE;
BOOL g_bOpenSay = FALSE;
BOOL bChecked = FALSE;
BOOL bCanShow = FALSE;
HWND hLoginHwnd;
HWND hMenuHwnd;
HWND hCreateHwnd;
HWND hFindHwnd;
HWND hSayHwnd;
FRIENDLIST *g_pHead;
FRIENDLIST *g_pEnd;
PLEASEMAN *g_pleaseHead;
PLEASEMAN *g_pleaseEnd;
HANDLE hThread;
char g_SelfID[20] = "";
char g_FriendID[20] = "";
int g_nFState = 0;
char g_szSayName[20] = "";


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	WSAData wd;
	WSAStartup(0x0202, &wd);

	sClient = socket(AF_INET, SOCK_STREAM, NULL);

	sin.sin_family = AF_INET;
	sin.sin_addr.S_un.S_addr = inet_addr("192.168.2.103");
	sin.sin_port = htons(5050);

	if (SOCKET_ERROR == connect(sClient, (SOCKADDR*)&sin, sizeof(sin)))
	{
		closesocket(sClient);
		return -1;
	}

	hThread = CreateThread(NULL, 0, ServerThreadProc, NULL, NULL, NULL);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOGLOGIN), NULL, DigLoginProc);

	if (bLogin)
	{
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_IMMENUE), NULL, DigMenuProc);
	}

	CloseHandle(hThread);
	WSACleanup();

	return 0;
}

BOOL CALLBACK DigLoginProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	hLoginHwnd = hWnd;

	if (bLogin)
	{
		EndDialog(hWnd, WM_CLOSE);
	}

	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				HWND hEditID = GetDlgItem(hWnd, IDC_EDITLOINGID);
				HWND hEditPWD = GetDlgItem(hWnd, IDC_EDITLOINGPWD);
				SendMessage(hEditID, EM_LIMITTEXT,(WPARAM)20, 0);
				SendMessage(hEditPWD, EM_LIMITTEXT,(WPARAM)20, 0);
			}
			break;
		case WM_COMMAND:
			{
				if (BN_CLICKED == HIWORD(wParam))
				{
					if (IDC_LOGIN == LOWORD(wParam))
					{
						BOOL ShouldBreak = FALSE;
						RELO rl;
						GetDlgItemText(hWnd, IDC_EDITLOINGID, rl.m_szID, 20);
						GetDlgItemText(hWnd, IDC_EDITLOINGPWD, rl.m_szPwd, 20);

						if (!strcmp(rl.m_szID, "") || !strcmp(rl.m_szPwd, ""))
						{
							MessageBox(hWnd, "请输入有效的字符", "啊哈", MB_OK);
							break;
						}

						for (int i = 0; i < 20; i++)
						{
							if (' ' == rl.m_szID[i] || ' ' == rl.m_szPwd[i])
							{
								MessageBox(hWnd, "ID和密码不能含有空格", "啊哈", MB_OK);
								ShouldBreak = TRUE;
								break;	
							}
						}

						if (ShouldBreak)
						{
							break;
						}

						strcpy(g_SelfID, rl.m_szID);

						PROTO *pt = (PROTO *)new char[sizeof(rl) + 8];

						if (bChecked)
						{
							pt->m_nEvent = HIDINLOGIN;
						}
						else
						{
							pt->m_nEvent = ONLINELOGIN;
						}

						pt->m_nSize = sizeof(rl) + 8;
						memcpy(pt->m_szData, &rl, sizeof(rl));
						send(sClient, (char *)pt, pt->m_nSize, 0);

						SetDlgItemText(hWnd, IDC_EDITLOINGPWD, "");
					}

					if (IDC_CREATE == LOWORD(wParam))
					{
						DialogBox((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGCREATE), hWnd, DigCreateProc);	
					}

					if (IDC_DARK == LOWORD(wParam))
					{
						bChecked = !bChecked;

						if (!bChecked)
						{
							HWND hLogin = GetDlgItem(hWnd, IDC_LOGIN);
							SetWindowText(hLogin, "在线登录");
						}

						if (bChecked)
						{
							HWND hLogin = GetDlgItem(hWnd, IDC_LOGIN);
							SetWindowText(hLogin, "隐身登录");
						}
					}
				}
			}
			break;
		case WM_CLOSE:
			EndDialog(hWnd, WM_CLOSE);
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK DigCreateProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	hCreateHwnd = hWnd;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				HWND hEditID = GetDlgItem(hWnd, IDC_EDIT1);
				HWND hEditPWD = GetDlgItem(hWnd, IDC_EDIT2);
				SendMessage(hEditID, EM_LIMITTEXT,(WPARAM)20, 0);
				SendMessage(hEditPWD, EM_LIMITTEXT,(WPARAM)20, 0);
			}
			break;
		case WM_COMMAND:
			{
				if (BN_CLICKED == HIWORD(wParam))
				{
					if (IDC_CREATECREATE == LOWORD(wParam))
					{
						BOOL ShouldBreak = FALSE;
						RELO rl;
						GetDlgItemText(hWnd, IDC_EDIT1, rl.m_szID, 20);
						GetDlgItemText(hWnd, IDC_EDIT2, rl.m_szPwd, 20);

						if (!strcmp(rl.m_szID, "") || !strcmp(rl.m_szPwd, ""))
						{
							MessageBox(hWnd, "请输入有效的字符", "啊哈", MB_OK);
							break;
						}

						for (int i = 0; i < 20; i++)
						{
							if (' ' == rl.m_szID[i] || ' ' == rl.m_szPwd[i])
							{
								MessageBox(hWnd, "ID和密码不能含有空格", "啊哈", MB_OK);
								ShouldBreak = TRUE;
								break;	
							}
						}

						if (ShouldBreak)
						{
							break;
						}

						PROTO *pt = (PROTO *)new char[sizeof(rl) + 8];
						pt->m_nEvent = CREATE;
						pt->m_nSize = sizeof(rl) + 8;
						memcpy(pt->m_szData, &rl, sizeof(rl));
						send(sClient, (char *)pt, pt->m_nSize, 0);

						SetDlgItemText(hWnd, IDC_EDIT1, "");
						SetDlgItemText(hWnd, IDC_EDIT2, "");
					}
				}
			}
			break;
		case WM_CLOSE:
			EndDialog(hWnd, WM_CLOSE);
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK DigMenuProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (NULL == g_pleaseHead)
	{
		HWND hPlist = GetDlgItem(hMenuHwnd, IDC_BUTTONPLEASE);
		EnableWindow(hPlist, FALSE);
	}

	hMenuHwnd = hWnd;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				SuspendThread(hThread);
				char DiaName[20] = "";
				SetDlgItemText(hWnd, IDC_MYSTYLE, "在线");
				strcpy(DiaName, "HZH-IM  ID:");
				strcat(DiaName, g_SelfID);
				SetWindowText(hWnd, DiaName);

				if (bChecked)
				{
					SetDlgItemText(hWnd, IDC_MYSTYLE, "隐身");	
				}

				HWND hf1 = GetDlgItem(hWnd, IDC_LIST);
				HWND hf2 = GetDlgItem(hWnd, IDC_LIST2);
				SendMessage(hf1, LB_RESETCONTENT, 0, 0);
				SendMessage(hf2, LB_RESETCONTENT, 0, 0);
				FRIENDLIST *pTemp1 = g_pHead;
				
				while (NULL != pTemp1)
				{
					char szBuf[60] = "";
					char NewFlage[5] = "";
					strcpy(szBuf, "./FriendSay/");
					strcat(szBuf, g_SelfID);
					strcat(szBuf, pTemp1->m_szID);
					strcat(szBuf, "-");
					strcat(szBuf, ".txt");
					FILE *fp1 = fopen(szBuf, "a+");
					fscanf(fp1, "%s", NewFlage);
					fclose(fp1);

					if (!strcmp("我", NewFlage) && ONLINE == pTemp1->m_nState)
					{
						char szSayFlage[30] = "(新消息)";
						strcat(szSayFlage, pTemp1->m_szID);
						SendDlgItemMessage(hMenuHwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)szSayFlage);
					}
					else if (ONLINE == pTemp1->m_nState)
					{
						SendDlgItemMessage(hMenuHwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)pTemp1->m_szID);
					}

					if (!strcmp("我", NewFlage) && ONLINE != pTemp1->m_nState)
					{
						char szSayFlage[30] = "(新消息)";
						strcat(szSayFlage, pTemp1->m_szID);
						SendDlgItemMessage(hMenuHwnd, IDC_LIST2, LB_ADDSTRING, 0, (LPARAM)szSayFlage);
					}
					else if (ONLINE != pTemp1->m_nState)
					{
						SendDlgItemMessage(hMenuHwnd, IDC_LIST2, LB_ADDSTRING, 0, (LPARAM)pTemp1->m_szID);	
					}
					
					pTemp1 = pTemp1->m_pNext;
				}

				bCanShow = true;
				ResumeThread(hThread);
			}
			break;
		case WM_COMMAND:
			{
				if (BN_CLICKED == HIWORD(wParam))
				{
					if (IDC_FIND == LOWORD(wParam))
					{
						DialogBox((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGFIND), hWnd, DigFindProc);		
					}

					if (IDC_BUTTONPLEASE == LOWORD(wParam))
					{
						DialogBox((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGPLEASE), hWnd, DigPleaseProc);
					}

					if (IDC_NOWDARK == LOWORD(wParam))
					{
						PROTO *pt = (PROTO *)new char[8];
						pt->m_nEvent = HIDING;
						pt->m_nSize = 8;
						send(sClient, (char *)pt, pt->m_nSize, 0);
					}

					if (IDC_NOW == LOWORD(wParam))
					{
						PROTO *pt = (PROTO *)new char[8];
						pt->m_nEvent = ONLINE;
						pt->m_nSize = 8;
						send(sClient, (char *)pt, pt->m_nSize, 0);	
					}
				}

				if (LBN_DBLCLK == HIWORD(wParam))
				{
					char szName[20] = "";
					char szMess[5] = "";
					char *pName = szName;

					if (IDC_LIST == LOWORD(wParam))
					{
						HWND List = GetDlgItem(hWnd, IDC_LIST); 
						int nList = (int)SendMessage(List, LB_GETCURSEL, 0, 0); 
						SendMessage(List, LB_GETTEXT, nList, (LPARAM)szName);
						strcpy(szMess, pName);

						if (!strcmp(szMess, "(新消息)"))
						{
							pName += 5;
						}
						
						strcpy(g_szSayName, pName);

						if (strcmp(szName, ""))
						{
							char szBuf[60] = "";
							strcpy(szBuf, "./FriendSay/");
							strcat(szBuf, g_SelfID);
							strcat(szBuf, g_szSayName);
							strcat(szBuf, "-");
							strcat(szBuf, ".txt");
							FILE *fp1 = fopen(szBuf, "w+");
							fclose(fp1);
							SendDlgItemMessage(hWnd, IDC_LIST, LB_DELETESTRING, nList, 0);
							SendDlgItemMessage(hWnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)pName);
							DialogBox((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGSAY), hWnd, DigSayProc);
							break;
						}
					}
					if (IDC_LIST2 == LOWORD(wParam))
					{
						HWND List2 = GetDlgItem(hWnd, IDC_LIST2); 
						int nList2 = (int)SendMessage(List2, LB_GETCURSEL, 0, 0); 
						SendMessage(List2, LB_GETTEXT, nList2, (LPARAM)szName);
						strcpy(szMess, pName);

						if (!strcmp(szMess, "(新消息)"))
						{
							pName += 5;
						}

						strcpy(g_szSayName, pName);

						if (strcmp(szName, ""))
						{
							char szBuf[60] = "";
							strcpy(szBuf, "./FriendSay/");
							strcat(szBuf, g_SelfID);
							strcat(szBuf, g_szSayName);
							strcat(szBuf, "-");
							strcat(szBuf, ".txt");
							FILE *fp1 = fopen(szBuf, "w+");
							fclose(fp1);
							SendDlgItemMessage(hWnd, IDC_LIST2, LB_DELETESTRING, nList2, 0);
							SendDlgItemMessage(hWnd, IDC_LIST2, LB_ADDSTRING, 0, (LPARAM)pName);
							DialogBox((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGSAY), hWnd, DigSayProc);
							break;
						}		
					}
				}
			}
			break;
		case WM_CLOSE:
			{
				PROTO *pt = (PROTO *)new char[8];
				pt->m_nEvent = BYE;
				pt->m_nSize = 8;
				send(sClient, (char *)pt, pt->m_nSize, 0);
				EndDialog(hWnd, WM_CLOSE);
			}
			break;
		default:
			return FALSE;
	}

	return TRUE;	
}

BOOL CALLBACK DigFindProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	hFindHwnd = hWnd;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				HWND hADD = GetDlgItem(hWnd, IDC_FINDADD);
				EnableWindow(hADD, FALSE);
				HWND hEditF = GetDlgItem(hWnd, IDC_EDITFIND);
				SendMessage(hEditF, EM_LIMITTEXT,(WPARAM)20, 0);
			}
			break;
		case WM_COMMAND:
			{
				if (BN_CLICKED == HIWORD(wParam))
				{
					if (IDC_FINDYES == LOWORD(wParam))
					{
						SendDlgItemMessage(hWnd, IDC_FINDSHOW, LB_DELETESTRING, 0, 0);
						SendDlgItemMessage(hWnd, IDC_FINDSHOW, LB_DELETESTRING, 0, 0);
						SendDlgItemMessage(hWnd, IDC_FINDSHOW, LB_DELETESTRING, 0, 0);
						SendDlgItemMessage(hWnd, IDC_FINDSHOW, LB_DELETESTRING, 0, 0);
						SendDlgItemMessage(hWnd, IDC_FINDSHOW, LB_DELETESTRING, 0, 0);
						SendDlgItemMessage(hWnd, IDC_FINDSHOW, LB_DELETESTRING, 0, 0);
						QUERYOK qr;
						GetDlgItemText(hWnd, IDC_EDITFIND, qr.m_szID, 20);

						PROTO *pt = (PROTO *)new char[sizeof(qr) + 8];
						pt->m_nEvent = QUERY;
						pt->m_nSize = sizeof(qr) + 8;
						memcpy(pt->m_szData, &qr, sizeof(qr));
						send(sClient, (char *)pt, pt->m_nSize, 0);

						strcpy(g_FriendID, qr.m_szID);
						SetDlgItemText(hWnd, IDC_EDITFIND, "");
					}

					if (IDC_FINDADD == LOWORD(wParam))
					{
						BOOL bCanBack = FALSE;
						HWND List = GetDlgItem(hWnd, IDC_FINDSHOW); 
						FRIENDOPERATE fo;
						strcpy(fo.m_szUser, g_FriendID);
						strcpy(fo.m_szFriend, g_SelfID);
						PLEASEMAN *ppTemp = g_pleaseHead;
				
						while (NULL != ppTemp)
						{
							if (!strcmp(ppTemp->m_szUser, g_FriendID))
							{
								MessageBox(hWnd, "已收到对方好友申请", "啊哈", MB_OK);
								bCanBack = TRUE;
								break;
							}

							ppTemp = ppTemp->m_pNext;
						}

						if (bCanBack)
						{
							break;
						}

						PROTO *pt = (PROTO *)new char[sizeof(fo) + 8];
						pt->m_nEvent = PLEASENOW;

						if (OFFLINE == g_nFState)
						{
							pt->m_nEvent = PLEASEOFFLINE;
						}

						pt->m_nSize = sizeof(fo) + 8;
						memcpy(pt->m_szData, &fo, sizeof(fo));
						send(sClient, (char *)pt, pt->m_nSize, 0);
						MessageBox(hWnd, "已发出好友申请", "啊哈", MB_OK);
						HWND hAdd = GetDlgItem(hWnd, IDC_FINDADD);
						EndDialog(hWnd, WM_CLOSE);
					}
				}
			}
			break;
		case WM_CLOSE:
			EndDialog(hWnd, WM_CLOSE);
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK DigSayProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	hSayHwnd = hWnd;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				SetFocus(GetDlgItem(hSayHwnd, IDC_EDITSAYINPUT));
				char WinName[30] = "";
				strcpy(WinName, g_szSayName);
				FRIENDLIST *pTemp = g_pHead;

				while (NULL != pTemp)
				{
					if (!strcmp(pTemp->m_szID, g_szSayName))
					{
						if (ONLINE == pTemp->m_nState)
						{
							strcat(WinName, "  (在线)");
							SetWindowText(hWnd, WinName);
							break;
						}
						else
						{
							strcat(WinName, "  (离线)");
							SetWindowText(hWnd, WinName);
							break;
						}
					}

					pTemp = pTemp->m_pNext;
				}

				HWND hEditPut = GetDlgItem(hWnd, IDC_EDITLOINGPWD);
				SendMessage(hEditPut, IDC_EDITSAYINPUT,(WPARAM)76, 0);
				g_bOpenSay = TRUE;
				char szName[20] = "";
				char szBuf[60] = "";
				GetWindowText(hWnd, szName, 20);
				strcpy(szBuf, "./FriendSay/");
				strcat(szBuf, g_SelfID);
				strcat(szBuf, g_szSayName);
				strcat(szBuf, ".txt");
				FILE *fp = fopen(szBuf, "a+");
				
				while (0 == feof(fp))
				{
					char SayName[20] = "";
					char SayData[255] = "";
					fscanf(fp, "%s", SayName);
					fscanf(fp, "%s", SayData);

					if (!strcmp(SayName, ""))
					{
						break;
					}

					strcat(SayName, ":");
					HWND hEdit = GetDlgItem(hWnd, IDC_EDITSAYSHOW);
					SendMessageA(hEdit, EM_SETSEL, -2, -1);
					SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)SayName);
					SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)"\r\n");
					SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)SayData);
					SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)"\r\n\r\n");
					SendMessageA(hEdit, WM_VSCROLL, SB_BOTTOM, 0);
				}
				fclose(fp);
				SetFocus(GetDlgItem(hSayHwnd, IDC_EDITSAYINPUT));
			}
			break;
		case WM_COMMAND:
			{
				if (BN_CLICKED == HIWORD(wParam))
				{
					if (IDC_DELETE == LOWORD(wParam)) 
					{	
						if (IDYES == MessageBox(hWnd, "是否删除", "删除好友", MB_YESNO))
						{
							char szName[20] = "";
							FRIENDOPERATE fo;
							strcpy(fo.m_szUser, g_szSayName);
							strcpy(fo.m_szFriend, g_SelfID);
							PROTO *pt = (PROTO *)new char[sizeof(fo) + 8];
							pt->m_nEvent = DELETENOW;
							pt->m_nSize = sizeof(fo) + 8;
							memcpy(pt->m_szData, &fo, sizeof(fo));
							send(sClient, (char *)pt, pt->m_nSize, 0);
							g_bOpenSay = FALSE;
							EndDialog(hWnd, WM_CLOSE);
						}
					}
					
					if (IDC_SAYSEND == LOWORD(wParam))
					{
						MESSAGE mg;
						char szName[20] = "";
						strcpy(mg.m_szFriend, g_szSayName);
						GetDlgItemText(hWnd, IDC_EDITSAYINPUT, mg.m_szMessage, 235);

						if (!strcmp(mg.m_szMessage, ""))
						{
							MessageBox(hWnd, "请输入有效的字符", "啊哈", MB_OK);
							SetFocus(GetDlgItem(hWnd, IDC_EDITSAYINPUT));
							break;
						}

						PROTO *pt = (PROTO *)new char[sizeof(mg) + 8];
						pt->m_nEvent = SAYSAY;
						pt->m_nSize = sizeof(mg) + 8;
						memcpy(pt->m_szData, &mg, sizeof(mg));
						send(sClient, (char *)pt, pt->m_nSize, 0);
						char szBuf[60] = "";
						strcpy(szBuf, "./FriendSay/");
						strcat(szBuf, g_SelfID);
						strcat(szBuf, g_szSayName);
						strcat(szBuf, ".txt");
						FILE *fp = fopen(szBuf, "a+");
						fprintf(fp, "我");
						fprintf(fp, "\n");
						fprintf(fp, mg.m_szMessage);
						fprintf(fp, "\n");
						fclose(fp);
						strcpy(szBuf, "./FriendSay/");
						strcat(szBuf, g_szSayName);
						strcat(szBuf, g_SelfID);
						strcat(szBuf, "-");
						strcat(szBuf, ".txt");
						FILE *fp1 = fopen(szBuf, "w+");
						fprintf(fp1, "我");
						fprintf(fp1, "\n");
						fclose(fp1);
						HWND hEdit = GetDlgItem(hWnd, IDC_EDITSAYSHOW);
						SendMessageA(hEdit, EM_SETSEL, -2, -1);
						SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)"我:");
						SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)"\r\n");
						SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)mg.m_szMessage);
						SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)"\r\n\r\n");
						SendMessageA(hEdit, WM_VSCROLL, SB_BOTTOM, 0);	
						SetDlgItemText(hWnd, IDC_EDITSAYINPUT, "");
						SetFocus(GetDlgItem(hWnd, IDC_EDITSAYINPUT));
					}
				}
			}
			break;
		case WM_CLOSE:
			{
				char szBuf[60] = "";
				strcpy(szBuf, "./FriendSay/");
				strcat(szBuf, g_SelfID);
				strcat(szBuf, g_szSayName);
				strcat(szBuf, "-");
				strcat(szBuf, ".txt");
				FILE *fp1 = fopen(szBuf, "w+");
				fclose(fp1);
				g_bOpenSay = FALSE;
				EndDialog(hWnd, WM_CLOSE);
			}
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK DigPleaseProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				if (NULL != g_pleaseHead)
				{
					PLEASEMAN *pTemp = g_pleaseHead;

					while (NULL != pTemp)
					{
						SendDlgItemMessage(hWnd, IDC_PLEASELIST, LB_DELETESTRING, 0, 0);	
						pTemp = pTemp->m_pNext;
					}

					pTemp = g_pleaseHead;

					while (NULL != pTemp)
					{
						SendDlgItemMessage(hWnd, IDC_PLEASELIST, LB_ADDSTRING, 0, (LPARAM)pTemp->m_szUser);	
						pTemp = pTemp->m_pNext;
					}
				}
			}
			break;
		case WM_COMMAND:
			{
				if (BN_CLICKED == HIWORD(wParam))
				{
					HWND List = GetDlgItem(hWnd, IDC_PLEASELIST); 
					int nList = (int)SendMessage(List, LB_GETCURSEL, 0, 0); 
					char szName[20];
					SendMessage(List, LB_GETTEXT, nList, (LPARAM)szName);
					FRIENDOPERATE fo;
					strcpy(fo.m_szUser, g_SelfID);
					strcpy(fo.m_szFriend, szName);
					PROTO *pt = (PROTO *)new char[sizeof(fo) + 8];
					pt->m_nSize = sizeof(fo) + 8;
					memcpy(pt->m_szData, &fo, sizeof(fo));

					if (IDC_PLEASELISTOK == LOWORD(wParam))
					{	
						pt->m_nEvent = FRIENDOK;
						send(sClient, (char *)pt, pt->m_nSize, 0);
					}

					if (IDC_PLEASELISTNO == LOWORD(wParam))
					{
						pt->m_nEvent = FRIENDNO;
						send(sClient, (char *)pt, pt->m_nSize, 0);
					}

					if (IDC_PLEASELISTNO == LOWORD(wParam) || IDC_PLEASELISTOK == LOWORD(wParam))
					{
						PLEASEMAN *pTemp = g_pleaseHead;

						while (NULL != pTemp)
						{
							if (!strcmp(pTemp->m_szUser, szName))
							{
								break;
							}

							pTemp = pTemp->m_pNext;
						}

						if (pTemp == g_pleaseHead)
						{
							if (pTemp == g_pleaseEnd)
							{
								g_pleaseHead = NULL;	
								g_pleaseEnd = NULL;
								SendDlgItemMessage(hWnd, IDC_PLEASELIST, LB_DELETESTRING, nList, 0);
								delete pTemp;
								break;
							}

							g_pleaseHead = pTemp->m_pNext;
							pTemp->m_pNext->m_pLast = NULL;
						}
						else
						{
							pTemp->m_pLast->m_pNext = pTemp->m_pNext;
							
							if (pTemp == g_pleaseEnd)
							{
								g_pleaseEnd = pTemp->m_pLast;
								SendDlgItemMessage(hWnd, IDC_PLEASELIST, LB_DELETESTRING, nList, 0);
								delete pTemp;
								break;
							}

							pTemp->m_pNext->m_pLast = pTemp->m_pLast;
						}

						delete pTemp;
						if (NULL != g_pleaseHead)
						{
							PLEASEMAN *pTemp = g_pleaseHead;

							while (NULL != pTemp)
							{
								SendDlgItemMessage(hWnd, IDC_PLEASELIST, LB_DELETESTRING, 0, 0);	
								pTemp = pTemp->m_pNext;
							}

							pTemp = g_pleaseHead;

							while (NULL != pTemp)
							{
								SendDlgItemMessage(hWnd, IDC_PLEASELIST, LB_ADDSTRING, 0, (LPARAM)pTemp->m_szUser);	
								pTemp = pTemp->m_pNext;
							}
						}
					}

					SendDlgItemMessage(hWnd, IDC_PLEASELIST, LB_DELETESTRING, nList, 0);
				}
			}
			break;
		case WM_CLOSE:
			EndDialog(hWnd, WM_CLOSE);
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

DWORD WINAPI ServerThreadProc(LPVOID lParam)
{
	while (g_bSever)
	{
		char szBuf[355] = "";

		if (0 >= recv(sClient, szBuf, 355, 0))
		{
			continue;
		}

		char *pBuf = szBuf;
		int nEvent = *(int *)pBuf;
		pBuf += 8;

		if (CREATEOK == nEvent)
		{
			MessageBox(hCreateHwnd, "注册成功", "啊哈", MB_OK);
		}
		else if (CREATENO == nEvent)
		{
			MessageBox(hCreateHwnd, "注册用户已存在", "啊哈", MB_OK);
		}
		else if (LOGINNOA == nEvent)
		{
			MessageBox(hLoginHwnd, "密码错误", "啊哈", MB_OK);
		}
		else if (LOGINNOB == nEvent)
		{
			MessageBox(hLoginHwnd, "用户不存在", "啊哈", MB_OK);
		}
		else if (LOGINNOC == nEvent)
		{
			MessageBox(hLoginHwnd, "用户已登录", "啊哈", MB_OK);
		}
		else if (LOGINOK == nEvent)
		{
			bLogin = TRUE;
		}
		else if (ONLINE == nEvent)
		{
			SetDlgItemText(hMenuHwnd, IDC_MYSTYLE, "在线");
		}
		else if (HIDING == nEvent)
		{
			SetDlgItemText(hMenuHwnd, IDC_MYSTYLE, "隐身");
		}
		else if (PLEASENOW == nEvent)
		{
			FRIENDOPERATE fo = *(FRIENDOPERATE*)pBuf;
			PLEASEMAN *pNew = new PLEASEMAN;
			strcpy(pNew->m_szUser, fo.m_szUser);
			strcpy(pNew->m_szFriend, fo.m_szFriend);
			pNew->m_pLast = NULL;
			pNew->m_pNext = NULL;

			if (NULL == g_pleaseHead)
			{
				g_pleaseHead = pNew;
				g_pleaseEnd = pNew;
			}	
			else
			{
				pNew->m_pLast = g_pleaseEnd;
				g_pleaseEnd->m_pNext = pNew;
				g_pleaseEnd = pNew;
			}

			HWND hPlist = GetDlgItem(hMenuHwnd, IDC_BUTTONPLEASE);
			EnableWindow(hPlist, TRUE);
		}
		else if (NOFRIEND == nEvent)
		{
			if (NOFRIEND == nEvent)
			{
				CleanFriendList();	
			}
			HWND hf1 = GetDlgItem(hMenuHwnd, IDC_LIST);
			HWND hf2 = GetDlgItem(hMenuHwnd, IDC_LIST2);
			SendMessage(hf1, LB_RESETCONTENT, 0, 0);
			SendMessage(hf2, LB_RESETCONTENT, 0, 0);	
		}
		else if (FRIEND == nEvent || FRIENDHEAD == nEvent)
		{
			if (FRIENDHEAD == nEvent)
			{
				CleanFriendList();	
			}
			
			FRIENDLIST fl = *(FRIENDLIST*)pBuf;
			FRIENDLIST *pNew = new FRIENDLIST;

			pNew->m_nState = fl.m_nState;
			pNew->m_pLast = NULL;
			pNew->m_pNext = NULL;
			strcpy(pNew->m_szID, fl.m_szID);

			if (NULL == g_pHead)
			{
				g_pHead = pNew;
				g_pEnd = pNew;
			}
			else
			{
				g_pEnd->m_pNext = pNew;
				pNew->m_pLast = g_pEnd;
				g_pEnd = pNew;
			}

			if (bCanShow)
			{
				FRIENDLIST *pTemp1 = g_pHead;
				HWND hf1 = GetDlgItem(hMenuHwnd, IDC_LIST);
				HWND hf2 = GetDlgItem(hMenuHwnd, IDC_LIST2);
				SendMessage(hf1, LB_RESETCONTENT, 0, 0);
				SendMessage(hf2, LB_RESETCONTENT, 0, 0);
				
				while (NULL != pTemp1)
				{
					char szBuf[60] = "";
					char NewFlage[5] = "";
					strcpy(szBuf, "./FriendSay/");
					strcat(szBuf, g_SelfID);
					strcat(szBuf, pTemp1->m_szID);
					strcat(szBuf, "-");
					strcat(szBuf, ".txt");
					FILE *fp1 = fopen(szBuf, "a+");
					fscanf(fp1, "%s", NewFlage);
					fclose(fp1);

					if (!strcmp("我", NewFlage) && ONLINE == pTemp1->m_nState)
					{
						char szSayFlage[30] = "(新消息)";
						strcat(szSayFlage, pTemp1->m_szID);
						SendDlgItemMessage(hMenuHwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)szSayFlage);
					}
					else if (ONLINE == pTemp1->m_nState)
					{
						SendDlgItemMessage(hMenuHwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)pTemp1->m_szID);
					}

					if (!strcmp("我", NewFlage) && ONLINE != pTemp1->m_nState)
					{
						char szSayFlage[30] = "(新消息)";
						strcat(szSayFlage, pTemp1->m_szID);
						SendDlgItemMessage(hMenuHwnd, IDC_LIST2, LB_ADDSTRING, 0, (LPARAM)szSayFlage);
					}
					else if (ONLINE != pTemp1->m_nState)
					{
						SendDlgItemMessage(hMenuHwnd, IDC_LIST2, LB_ADDSTRING, 0, (LPARAM)pTemp1->m_szID);	
					}
					
					pTemp1 = pTemp1->m_pNext;
				}
			}

			char WinName[30] = "";
			strcpy(WinName, g_szSayName);
			FRIENDLIST *pTemp = g_pHead;

			while (NULL != pTemp)
			{
				if (!strcmp(pTemp->m_szID, g_szSayName))
				{
					if (ONLINE == pTemp->m_nState)
					{
						strcat(WinName, "  (在线)");
						SetWindowText(hSayHwnd, WinName);
						break;
					}
					else
					{
						strcat(WinName, "  (离线)");
						SetWindowText(hSayHwnd, WinName);
						break;
					}
				}

				pTemp = pTemp->m_pNext;
			}
		}
		else if (QUERY == nEvent || QUERYNO == nEvent)
		{
			QUERYOK qr = *(QUERYOK*)pBuf;

			if (!strcmp(qr.m_szID, ""))
			{
				SendDlgItemMessage(hFindHwnd, IDC_FINDSHOW, LB_ADDSTRING, 0, (LPARAM)"无此用户");
			}
			else
			{
				g_nFState = qr.m_nState;
				SendDlgItemMessage(hFindHwnd, IDC_FINDSHOW, LB_ADDSTRING, 0, (LPARAM)"用户名:");
				SendDlgItemMessage(hFindHwnd, IDC_FINDSHOW, LB_ADDSTRING, 0, (LPARAM)qr.m_szID);
				SendDlgItemMessage(hFindHwnd, IDC_FINDSHOW, LB_ADDSTRING, 0, (LPARAM)"是否在线:");

				if (ONLINE == qr.m_nState)
				{
					SendDlgItemMessage(hFindHwnd, IDC_FINDSHOW, LB_ADDSTRING, 0, (LPARAM)"在线");
				}
				else
				{
					SendDlgItemMessage(hFindHwnd, IDC_FINDSHOW, LB_ADDSTRING, 0, (LPARAM)"离线");	
				}
				
				SendDlgItemMessage(hFindHwnd, IDC_FINDSHOW, LB_ADDSTRING, 0, (LPARAM)"是否为好友:");
				FRIENDLIST *pTemp = g_pHead;
				BOOL bFriend = FALSE;

				while (NULL != pTemp)
				{
					if (!strcmp(qr.m_szID, pTemp->m_szID))
					{
						bFriend = TRUE;
					}

					pTemp = pTemp->m_pNext;
				}
					
				if (bFriend)
				{
					SendDlgItemMessage(hFindHwnd, IDC_FINDSHOW, LB_ADDSTRING, 0, (LPARAM)"是");
				}
				else if(!strcmp(qr.m_szID, g_SelfID))
				{
					SendDlgItemMessage(hFindHwnd, IDC_FINDSHOW, LB_ADDSTRING, 0, (LPARAM)"这是你自己");	
				}
				else
				{
					SendDlgItemMessage(hFindHwnd, IDC_FINDSHOW, LB_ADDSTRING, 0, (LPARAM)"不是");
					HWND hADD = GetDlgItem(hFindHwnd, IDC_FINDADD);
					EnableWindow(hADD, TRUE);
				}

				if (QUERYNO == nEvent)
				{
					HWND hAdd = GetDlgItem(hFindHwnd, IDC_FINDADD);
					SetDlgItemText(hFindHwnd, IDC_FINDADD, "已申请");
					EnableWindow(hAdd, FALSE);
				}

				PLEASEMAN *ppTemp = g_pleaseHead;
				
				while (NULL != ppTemp)
				{
					if (!strcmp(ppTemp->m_szUser, qr.m_szID))
					{
						HWND hAdd = GetDlgItem(hFindHwnd, IDC_FINDADD);
						SetDlgItemText(hFindHwnd, IDC_FINDADD, "已有申请");
						EnableWindow(hAdd, FALSE);
						break;
					}

					ppTemp = ppTemp->m_pNext;
				}
			}
		}
		else if (HIDING == nEvent)
		{
			SetDlgItemText(hMenuHwnd, IDC_MYSTYLE, "隐身");
		}
		else if (SAYSAY == nEvent)
		{
			MESSAGE mg = *(MESSAGE*)pBuf;
			FRIENDLIST *pTemp = g_pHead;

			while (NULL != pTemp)
			{
				if (!strcmp(pTemp->m_szID, mg.m_szFriend))
				{
					char szBuf[60] = "";
					strcpy(szBuf, "./FriendSay/");
					strcat(szBuf, g_SelfID);
					strcat(szBuf, pTemp->m_szID);
					strcat(szBuf, ".txt");
					FILE *fp = fopen(szBuf, "a+");
					fprintf(fp, pTemp->m_szID);
					fprintf(fp, "\n");
					fprintf(fp, mg.m_szMessage);
					fprintf(fp, "\n");
					fclose(fp);

					if (g_bOpenSay && !strcmp(g_szSayName, mg.m_szFriend))
					{
						char szSay[255] = "";
						HWND hEdit = GetDlgItem(hSayHwnd, IDC_EDITSAYSHOW);
						SendMessageA(hEdit, EM_SETSEL, -2, -1);
						SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)pTemp->m_szID);
						SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)":");
						SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)"\r\n");
						SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)mg.m_szMessage);
						SendMessageA(hEdit, EM_REPLACESEL, TRUE, (long)"\r\n\r\n");
						SendMessageA(hEdit, WM_VSCROLL, SB_BOTTOM, 0);	
					}

					if (!g_bOpenSay || (g_bOpenSay && strcmp(g_szSayName, mg.m_szFriend)))
					{	
						FRIENDLIST *pTemp1 = g_pHead;
						int nList = 0;
						char szListOne[20] = "";
						HWND List = GetDlgItem(hMenuHwnd, IDC_LIST);
						HWND List2 = GetDlgItem(hMenuHwnd, IDC_LIST2);

						while (NULL != pTemp1)
						{
							SendMessage(List, LB_GETTEXT, nList, (LPARAM)szListOne);

							if (!strcmp(szListOne, mg.m_szFriend))
							{
								SendDlgItemMessage(hMenuHwnd, IDC_LIST, LB_DELETESTRING, nList, 0);
								char szSayFlage[30] = "(新消息)";
								strcat(szSayFlage, mg.m_szFriend);
								SendDlgItemMessage(hMenuHwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)szSayFlage);
								break;
							}

							SendMessage(List2, LB_GETTEXT, nList, (LPARAM)szListOne);

							if (!strcmp(szListOne, mg.m_szFriend))
							{
								SendDlgItemMessage(hMenuHwnd, IDC_LIST2, LB_DELETESTRING, nList, 0);
								char szSayFlage[30] = "(新消息)";
								strcat(szSayFlage, mg.m_szFriend);
								SendDlgItemMessage(hMenuHwnd, IDC_LIST2, LB_ADDSTRING, 0, (LPARAM)szSayFlage);
								break;
							}

							nList++;
							pTemp1 = pTemp1->m_pNext;
						}	
					}
				}

				pTemp = pTemp->m_pNext;
			}
		}

		nEvent = 0;
	}
	
	return 0;
}

void CleanFriendList()
{
	if (NULL != g_pHead)
	{
		FRIENDLIST *pTemp = g_pHead;
		FRIENDLIST *pTempNext = NULL;

		while (NULL != pTemp)
		{
			pTempNext = pTemp->m_pNext;
			delete pTemp;
			pTemp = pTempNext;
		}

		g_pHead = NULL;
		g_pEnd = NULL;
	}
}