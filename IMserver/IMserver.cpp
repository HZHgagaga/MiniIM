// IMserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Protocol.h"
#pragma comment(lib, "ws2_32.lib")

DWORD WINAPI ThreadProc(LPVOID lParam);
void DoEvent(char *pleaseID, char *pleaseFriend, char *pData);
void CreateUSERLIST(RELO *rl, int nEvent, sockaddr_in *sin, USERLIST **pNew, SOCKET nSock);	//在线链表增添
void UpdateFriendList(USERLIST *pTemp, char *pleaseID);		//更新好友列表链表
void DeleteFriendList(USERLIST *pTemp);		//清空好友列表链表
void AddFriendList(USERLIST *pSelfUSE, char *pStr);		//好友列表增加
void CleanOffDoList();

sockaddr_in Clientaddr;
USERLIST *g_pUHead;			//在线用户链表头指针
USERLIST *g_pUEnd;			//在线用户链表尾指针
OFFDO *OffDoHead;
OFFDO *OffDoEnd;
bool b_AddFriend = false;	//好友请求更新标志
bool b_DelFriend = false;	//好友删除更新标志
bool b_SayOK = false;
bool b_DeleteOK = false;
bool b_AddFriendOK = false;
char szFindName[20] = "";
int g_npleaseEvent;
HANDLE  g_hMutex = NULL;

int main(int argc, char* argv[])
{
	WSAData wd;
	WSAStartup(0x0202, &wd);

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.S_un.S_addr = inet_addr("192.168.2.103");
	sin.sin_port = htons(5050);

	if (SOCKET_ERROR == bind(sListen, (SOCKADDR*)&sin, sizeof(sin)))
	{
		closesocket(sListen);
		return -1;
	}

	if (SOCKET_ERROR == listen(sListen, 3))
	{
		closesocket(sListen);
		return -1;
	}

	while (1)
	{
		int nLen = sizeof(Clientaddr);
		SOCKET sClient = accept(sListen, (SOCKADDR*)&Clientaddr, &nLen);
		HANDLE hThread = CreateThread(NULL, 0, ThreadProc, (LPVOID)sClient, 0, NULL);
		CloseHandle(hThread);
	}

	WSACleanup();
	CloseHandle(g_hMutex);

	return 0;
}

void DoEvent(char *pleaseID, char *pleaseFriend, char *pData)
{
	USERLIST *pTemp = g_pUHead;
	
	while (NULL != pTemp)
	{
		if(!strcmp(pleaseFriend, pTemp->m_szID) && SAYSAY == g_npleaseEvent)  //在线对话
		{
			Sleep(1);
			MESSAGE mg;
			strcpy(mg.m_szMessage, pData);
			strcpy(mg.m_szFriend, pleaseID);	
			PROTO *pt = (PROTO *)new char[sizeof(mg) + 8];
			pt->m_nEvent = SAYSAY;
			pt->m_nSize = sizeof(mg) + 8;
			memcpy(pt->m_szData, &mg, sizeof(mg));
			send(pTemp->m_Socket, (char *)pt, pt->m_nSize, 0);
			g_npleaseEvent = 0;
			b_SayOK = true;
		}

		if(!strcmp(pleaseFriend, pTemp->m_szID) && PLEASENOW == g_npleaseEvent)  //发送好友请求
		{
			FRIENDOPERATE foplease;
			strcpy(foplease.m_szUser, pleaseID);
			strcpy(foplease.m_szFriend, pleaseFriend);	
			PROTO *pt = (PROTO *)new char[sizeof(foplease) + 8];
			pt->m_nEvent = PLEASENOW;
			pt->m_nSize = sizeof(foplease) + 8;
			memcpy(pt->m_szData, &foplease, sizeof(foplease));
			send(pTemp->m_Socket, (char *)pt, pt->m_nSize, 0);
			g_npleaseEvent = 0;
		}
		
		if(!strcmp(pleaseFriend, pTemp->m_szID) && FRIENDOK == g_npleaseEvent)	//好友同意请求
		{
			AddFriendList(pTemp, pleaseID);
			UpdateFriendList(pTemp, "");
			g_npleaseEvent = 0;
			b_AddFriendOK = true;
		}
		
		if(!strcmp(pleaseFriend, pTemp->m_szID) && FRIENDNO == g_npleaseEvent)	//好友拒绝请求
		{
			FRIENDOPERATE foplease;
			strcpy(foplease.m_szUser, pleaseID);
			strcpy(foplease.m_szFriend, pleaseFriend);	
			PROTO *pt = (PROTO *)new char[sizeof(foplease) + 8];
			pt->m_nEvent = FRIENDNO;
			pt->m_nSize = sizeof(foplease) + 8;
			memcpy(pt->m_szData, &foplease, sizeof(foplease));
			send(pTemp->m_Socket, (char *)pt, pt->m_nSize, 0);
			g_npleaseEvent = 0;
		}
		
		if(!strcmp(pleaseFriend, pTemp->m_szID) && DELETENOW == g_npleaseEvent)	//好友删除
		{
			UpdateFriendList(pTemp, pleaseID);

			if (NULL == pTemp->m_pHead)
			{
				PROTO *pt = (PROTO *)new char[8];
				pt->m_nEvent = NOFRIEND;
				pt->m_nSize = 8;
				send(pTemp->m_Socket, (char *)pt, pt->m_nSize, 0);	
			}

			g_npleaseEvent = 0;
			b_DeleteOK = true;
		}

		USERLIST *pTemp1 = g_pUHead;
	
		while (NULL != pTemp1)
		{	
			if(NULL != pTemp1->m_pHead)      //发送好友列表
			{
				FRIENDLIST Friend = *(pTemp1->m_pHead);
				FRIENDLIST *pFriend = &Friend;
				
				while (NULL != pFriend)
				{
					Friend = *pFriend;
					PROTO *pt = (PROTO *)new char[sizeof(Friend) + 8];
					pt->m_nEvent = FRIEND;
					
					if (!strcmp(Friend.m_szID, pTemp1->m_pHead->m_szID))
					{
						pt->m_nEvent = FRIENDHEAD;
					}
					
					pt->m_nSize = sizeof(Friend) + 8;
					memcpy(pt->m_szData, &Friend, sizeof(Friend));
					send(pTemp1->m_Socket, (char *)pt, 355, 0);
					pFriend = pFriend->m_pNext;
				}
				
				DeleteFriendList(pTemp1);
			}
			
			pTemp1 = pTemp1->m_pNext;
		}

		pTemp = pTemp->m_pNext;
	}
}

DWORD WINAPI ThreadProc(LPVOID lParam)
{
	SOCKET sClient = (SOCKET)lParam;
	sockaddr_in sin = Clientaddr;
	USERLIST *pSelfUSE = new USERLIST;
	pSelfUSE->m_pHead = NULL;
	bool bFlage = false;
	bool binhere = false;
	char szMyID[20] = "";
	char szrID[20] = "";		//比较用户注册登录是否有问题
	char szrPwd[20] = "";		//比较用户注册登录是否有问题
	char szBuf[355] = "";		//接收客户端的数据包
	int nEvent = 0;
	int nSize = 0;
	char szpleaseID[20] = "";
	char szpleaseFriend[20] = "";

	while (1)
	{
		char *pBuf = szBuf;

		if (0 >= recv(sClient, szBuf, 355, 0))
		{
			break;
		}

		
		int nEvent = *(int *)pBuf;
		pBuf += 8;

		if (CREATE == nEvent)	//用户注册
		{
			g_hMutex = CreateMutex(NULL,TRUE,NULL);
			RELO rl = *(RELO *)pBuf;
			FILE *fp = fopen("User.txt", "a+");
			FILE *fp1 = fopen("User.txt", "r");
			
			while (0 == feof(fp1))
			{
				fscanf(fp1, "%s", szrID);
				fscanf(fp1, "%s", szrPwd);
				
				if (!strcmp(szrID, rl.m_szID))
				{ 
					PROTO *pt = (PROTO *)new char[8];
					pt->m_nEvent = CREATENO;
					pt->m_nSize = 8;
					send(sClient, (char *)pt, pt->m_nSize, 0);
					fclose(fp1);
					bFlage = true;
					break;
				}
			}
			
			if (!bFlage)
			{
				fprintf(fp, rl.m_szID);
				fprintf(fp, "\n");
				fprintf(fp, rl.m_szPwd);
				fprintf(fp, "\n");
				PROTO *pt = (PROTO *)new char[8];
				pt->m_nEvent = CREATEOK;
				pt->m_nSize = 8;
				send(sClient, (char *)pt, pt->m_nSize, 0);
			}
			
			bFlage = false;
			fclose(fp);
			ReleaseMutex(g_hMutex);
		}
		else if (ONLINELOGIN == nEvent || HIDINLOGIN == nEvent)		//用户登录
		{
			g_hMutex = CreateMutex(NULL,TRUE,NULL);
			RELO rl = *(RELO *)pBuf;
			FILE *fp = fopen("User.txt", "a+");
			USERLIST *pTemp = g_pUHead;
			binhere = false;

			while (NULL != pTemp)
			{
				if (!strcmp(pTemp->m_szID, rl.m_szID))
				{
					PROTO *pt = (PROTO *)new char[8];
					pt->m_nEvent = LOGINNOC;
					pt->m_nSize = 8;
					send(sClient, (char *)pt, pt->m_nSize, 0);
					binhere = true;
				}

				pTemp = pTemp->m_pNext;
			}

			if (binhere)
			{
				binhere = false;
				ReleaseMutex(g_hMutex);
				continue;
			}

			while (0 == feof(fp))
			{
				fscanf(fp, "%s", szrID);
				fscanf(fp, "%s", szrPwd);

				if (!strcmp(szrID, rl.m_szID) && !strcmp(szrPwd, rl.m_szPwd))
				{
					PROTO *pt = (PROTO *)new char[8];
					pt->m_nEvent = LOGINOK;
					pt->m_nSize = 8;
					send(sClient, (char *)pt, pt->m_nSize, 0);
					CreateUSERLIST(&rl, nEvent, &sin, &pSelfUSE, sClient);    //增入在线用户链表
					USERLIST *pTemp = g_pUHead;
					strcpy(szMyID, rl.m_szID);
					char szBuf1[50] = "./Say/";
					strcat(szBuf1, rl.m_szID);
					strcat(szBuf1, ".txt");
					FILE *fp1 = fopen(szBuf1, "a+");

					while (NULL != pTemp)
					{
						UpdateFriendList(pTemp, "");
						DoEvent("", "", "");
						pTemp = pTemp->m_pNext;
					}

					while (0 == feof(fp1))		//操作离线消息
					{
						char szfID[20] = "";
						char szMessage[255] = "";
						fscanf(fp1, "%s", szfID);
						fscanf(fp1, "%s", szMessage);

						if (!strcmp(szfID, ""))
						{
							break;
						}

						g_npleaseEvent = SAYSAY;
						DoEvent(szfID, szMyID, szMessage);
					}
	
					fclose(fp1);
					FILE *fp3 = fopen(szBuf1, "w+");
					fclose(fp3);
					char szBuf2[50] = "./FriendOperate/";
					strcat(szBuf2, rl.m_szID);
					strcat(szBuf2, ".txt");
					FILE *fp2 = fopen(szBuf2, "a+");

					while (0 == feof(fp2))
					{
						char szNum[2] = "";
						char szdeID[20] = "";
						fscanf(fp2, "%s", szNum);
						fscanf(fp2, "%s", szdeID);

						if ('0' == szNum[0])		//操作离线删除好友
						{
							g_npleaseEvent = DELETENOW;
							DoEvent(szdeID, szMyID, "");
						}

						if ('1' == szNum[0])
						{
							OFFDO *pNew = new OFFDO;
							strcpy(pNew->m_szNum, szNum);
							strcpy(pNew->m_szFriend, szdeID);
							pNew->pNext = NULL;

							if (NULL == OffDoHead)
							{
								OffDoHead = pNew;
								OffDoEnd = pNew;
							}
							else
							{
								OffDoEnd->pNext = pNew;
								OffDoEnd = pNew;
							}

							g_npleaseEvent = PLEASENOW;
							DoEvent(szdeID, szMyID, "");
						}
					}

					fclose(fp2);
					FILE *fp4 = fopen(szBuf2, "w+");
					OFFDO *pOffT = OffDoHead;

					while (NULL != pOffT)
					{
						fprintf(fp4, pOffT->m_szNum);
						fprintf(fp4, "\n");
						fprintf(fp4, pOffT->m_szFriend);
						fprintf(fp4, "\n");
						pOffT = pOffT->pNext;
					}

					fclose(fp4);
					CleanOffDoList();
					bFlage = true;
					pTemp = g_pUHead;
					break;
				}
				else if (!strcmp(szrID, rl.m_szID) && strcmp(szrPwd, rl.m_szPwd))
				{
					PROTO *pt = (PROTO *)new char[8];
					pt->m_nEvent = LOGINNOA;
					pt->m_nSize = 8;
					send(sClient, (char *)pt, pt->m_nSize, 0);
					bFlage = true;
					break;
				}
			}

			if (!bFlage)
			{
				PROTO *pt = (PROTO *)new char[8];
				pt->m_nEvent = LOGINNOB;
				pt->m_nSize = 8;
				send(sClient, (char *)pt, pt->m_nSize, 0);
			}

			bFlage = false;
			fclose(fp);
			ReleaseMutex(g_hMutex);
		}
		else if (QUERY == nEvent)	//查询用户
		{
			g_hMutex = CreateMutex(NULL,TRUE,NULL);
			char *pFindName = szFindName;
			pFindName = (char *)pBuf;
			USERLIST *pQuery = g_pUHead;
			QUERYOK pFindOK;
			bool bFind = false;     //判断是否查询到
			bool bFindHave = false;

			char szBuf2[50] = "./FriendOperate/";
			strcat(szBuf2, pFindName);
			strcat(szBuf2, ".txt");
			FILE *fp2 = fopen(szBuf2, "a+");

			while (0 == feof(fp2))
			{
				char szNum[2] = "";
				char szdeID[20] = "";
				fscanf(fp2, "%s", szNum);
				fscanf(fp2, "%s", szdeID);

				if ('1' == szNum[0])
				{
					if (!strcmp(szdeID, szMyID))
					{
						bFindHave = true;
						break;
					}
				}
			}

			fclose(fp2);

			while (NULL != pQuery)
			{
				if (!strcmp(pQuery->m_szID, pFindName))
				{
					strcpy(pFindOK.m_szID, pFindName);
					pFindOK.m_nState = pQuery->m_nState;	
					bFind = true;
				}

				pQuery = pQuery->m_pNext;
			}

			if (!bFind)
			{
				FILE *fp = fopen("User.txt", "a+");

				while (0 == feof(fp))
				{
					fscanf(fp, "%s", szrID);
					fscanf(fp, "%s", szrPwd);

					if (!strcmp(szrID, pFindName))
					{
						strcpy(pFindOK.m_szID, pFindName);
						pFindOK.m_nState = OFFLINE;
						bFind = true;
					}
				}

				if (!bFind)
				{
					strcpy(pFindOK.m_szID, "");
					pFindOK.m_nState = OFFLINE;
				}

				fclose(fp);
			}

			PROTO *pt = (PROTO *)new char[sizeof(pFindOK) + 8];
			pt->m_nEvent = QUERY;

			if (bFindHave)
			{
				pt->m_nEvent = QUERYNO;	
			}

			pt->m_nSize = sizeof(pFindOK) + 8;
			memcpy(pt->m_szData, &pFindOK, sizeof(pFindOK));
			send(sClient, (char *)pt, pt->m_nSize, 0);
			ReleaseMutex(g_hMutex);
		}
		else if (ONLINE == nEvent || HIDING == nEvent)		//在线状态更改
		{
			g_hMutex = CreateMutex(NULL,TRUE,NULL);
			USERLIST *pTemp = g_pUHead;

			while (NULL != pTemp)
			{
				if (!strcmp(szMyID, pTemp->m_szID))
				{
					if (ONLINE == nEvent)
					{
						pTemp->m_nState = ONLINE;
					}
					else
					{
						pTemp->m_nState = HIDING;
					}
				}

				pTemp = pTemp->m_pNext;
			}

			PROTO *pt = (PROTO *)new char[8];
			pt->m_nEvent = nEvent;
			pt->m_nSize = 8;
			send(sClient, (char *)pt, pt->m_nSize, 0);

			pTemp = g_pUHead;
			Sleep(10);

			while (NULL != pTemp)
			{
				UpdateFriendList(pTemp, "");

				DoEvent("", "", "");

				pTemp = pTemp->m_pNext;
			}
			ReleaseMutex(g_hMutex);
		}
		else if (PLEASENOW == nEvent)		//好友请求(在线)	
		{
			g_hMutex = CreateMutex(NULL,TRUE,NULL);
			FRIENDOPERATE foplease = *(FRIENDOPERATE *)pBuf;
			g_npleaseEvent = PLEASENOW;
			DoEvent(foplease.m_szFriend, foplease.m_szUser, "");
			char szBuf[60] = "./FriendOperate/";
			strcat(szBuf, foplease.m_szUser);
			strcat(szBuf, ".txt");
			FILE *fp = fopen(szBuf, "a+");
			fprintf(fp, "1");
			fprintf(fp, "\n");
			fprintf(fp, foplease.m_szFriend);
			fprintf(fp, "\n");
			fclose(fp);
			ReleaseMutex(g_hMutex);
		}
		else if (DELETENOW == nEvent)		//好友删除(在线)	
		{
			g_hMutex = CreateMutex(NULL,TRUE,NULL);
			b_DeleteOK = false;
			FRIENDOPERATE foplease = *(FRIENDOPERATE *)pBuf;
			g_npleaseEvent = DELETENOW;
			UpdateFriendList(pSelfUSE, foplease.m_szUser);

			if (NULL == pSelfUSE->m_pHead)
			{
				PROTO *pt = (PROTO *)new char[8];
				pt->m_nEvent = NOFRIEND;
				pt->m_nSize = 8;
				send(sClient, (char *)pt, pt->m_nSize, 0);	
			}

			DoEvent(foplease.m_szFriend, foplease.m_szUser, "");

			if (!b_DeleteOK)
			{
				char szBuf[50] = "./FriendOperate/";
				strcat(szBuf, foplease.m_szUser);
				strcat(szBuf, ".txt");
				FILE *fp = fopen(szBuf, "a+");
				fprintf(fp, "0");
				fprintf(fp, "\n");
				fprintf(fp, foplease.m_szFriend);
				fprintf(fp, "\n");
				fclose(fp);	
			}
			ReleaseMutex(g_hMutex);
		}
		else if (FRIENDOK == nEvent)		//好友请求同意	
		{
			g_hMutex = CreateMutex(NULL,TRUE,NULL);
			b_AddFriendOK = false;
			FRIENDOPERATE foplease = *(FRIENDOPERATE *)pBuf;
			g_npleaseEvent = FRIENDOK;
			AddFriendList(pSelfUSE, foplease.m_szFriend);
			UpdateFriendList(pSelfUSE, "");
			DoEvent(foplease.m_szUser, foplease.m_szFriend, "");
			char szBuf[60] = "./Friend/";
			strcat(szBuf, foplease.m_szFriend);
			strcat(szBuf, ".txt");
			
			if (!b_AddFriendOK)
			{
				FILE *fp = fopen(szBuf, "a+");
				fprintf(fp, foplease.m_szUser);
				fprintf(fp, "\n");
				fclose(fp);
			}

			char szBuf1[60] = "./FriendOperate/";
			strcat(szBuf1, foplease.m_szUser);
			strcat(szBuf1, ".txt");
			FILE *fp1 = fopen(szBuf1, "a+");

			while (0 == feof(fp1))
			{
				char szNum[5] = "";
				char szFriend[20] = "";

				fscanf(fp1, "%s", szNum);
				fscanf(fp1, "%s", szFriend);

				if (!strcmp(szNum, "") || !strcmp(szFriend, ""))
				{
					break;
				}

				if (!strcmp(szNum, "1") || !strcmp(szFriend, foplease.m_szFriend))
				{
					continue;
				}
				
				OFFDO *pNew = new OFFDO;
				strcpy(pNew->m_szNum, szNum);
				strcpy(pNew->m_szFriend, szFriend);
				pNew->pNext = NULL;
				
				if (NULL == OffDoHead)
				{
					OffDoHead = pNew;
					OffDoEnd = pNew;
				}
				else
				{
					OffDoEnd->pNext = pNew;
					OffDoEnd = pNew;
				}
			}

			fclose(fp1);
			OFFDO *pTemp = OffDoHead;
			FILE *fp2 = fopen(szBuf1, "w+");

			while (NULL != pTemp)
			{
				fprintf(fp2, pTemp->m_szNum);
				fprintf(fp2, "\n");
				fprintf(fp2, pTemp->m_szFriend);
				fprintf(fp2, "\n");
				pTemp = pTemp->pNext;
			}

			fclose(fp2);
			CleanOffDoList();
			ReleaseMutex(g_hMutex);
		}
		else if (FRIENDNO == nEvent)		//好友请求拒绝	
		{
			g_hMutex = CreateMutex(NULL,TRUE,NULL);
			FRIENDOPERATE foplease = *(FRIENDOPERATE *)pBuf;
			g_npleaseEvent = FRIENDNO;
			DoEvent(foplease.m_szUser, foplease.m_szFriend, "");
			char szBuf[60] = "./FriendOperate/";
			strcat(szBuf, foplease.m_szUser);
			strcat(szBuf, ".txt");
			FILE *fp1 = fopen(szBuf, "a+");

			while (0 == feof(fp1))
			{
				char szNum[5] = "";
				char szFriend[20] = "";

				fscanf(fp1, "%s", szNum);
				fscanf(fp1, "%s", szFriend);

				if (!strcmp(szNum, "") || !strcmp(szFriend, ""))
				{
					break;
				}

				if (!strcmp(szNum, "1") || !strcmp(szFriend, foplease.m_szFriend))
				{
					continue;
				}
				
				OFFDO *pNew = new OFFDO;
				strcpy(pNew->m_szNum, szNum);
				strcpy(pNew->m_szFriend, szFriend);
				pNew->pNext = NULL;
				
				if (NULL == OffDoHead)
				{
					OffDoHead = pNew;
					OffDoEnd = pNew;
				}
				else
				{
					OffDoEnd->pNext = pNew;
					OffDoEnd = pNew;
				}
			}

			fclose(fp1);
			OFFDO *pTemp = OffDoHead;
			FILE *fp2 = fopen(szBuf, "w+");

			while (NULL != pTemp)
			{
				fprintf(fp2, pTemp->m_szNum);
				fprintf(fp2, "\n");
				fprintf(fp2, pTemp->m_szFriend);
				fprintf(fp2, "\n");
				pTemp = pTemp->pNext;
			}

			fclose(fp2);
			CleanOffDoList();
			ReleaseMutex(g_hMutex);
		}
		else if (SAYSAY == nEvent)		//对话	
		{
			g_hMutex = CreateMutex(NULL,TRUE,NULL);
			b_SayOK = false;
			MESSAGE mg = *(MESSAGE *)pBuf;
			g_npleaseEvent = SAYSAY;
			DoEvent(szMyID, mg.m_szFriend, mg.m_szMessage);

			if (!b_SayOK)
			{
				char szsID[20] = "";
				char szsPwd[20] = "";
				FILE *fp = fopen("User.txt", "a+");
				FILE *fp1 = fopen("User.txt", "r");
				
				while (0 == feof(fp1))
				{
					fscanf(fp1, "%s", szsID);
					fscanf(fp1, "%s", szsPwd);
					
					if (!strcmp(szsID, mg.m_szFriend))
					{ 
						char szBuf[50] = "./Say/";
						strcat(szBuf, mg.m_szFriend);
						strcat(szBuf, ".txt");
						FILE *fp2 = fopen(szBuf, "a+");
						fprintf(fp2, szMyID);
						fprintf(fp2, "\n");
						fprintf(fp2, mg.m_szMessage);
						fprintf(fp2, "\n");
						fclose(fp2);
						fclose(fp1);
						break;
					}
				}

				fclose(fp);
			}
			ReleaseMutex(g_hMutex);
		}
		else if (BYE == nEvent)		//下线	
		{
			g_hMutex = CreateMutex(NULL,TRUE,NULL);
			if (pSelfUSE == g_pUHead)
			{
				if (pSelfUSE == g_pUEnd)
				{
					g_pUHead = NULL;
					g_pUEnd = NULL;
					delete pSelfUSE;
					USERLIST *pTemp = g_pUHead;

					while (NULL != pTemp)
					{
						UpdateFriendList(pTemp, "");
						pTemp = pTemp->m_pNext;
					}

					DoEvent("", "", "");
					ReleaseMutex(g_hMutex);
					return 0;
				}
				
				g_pUHead = pSelfUSE->m_pNext;
				pSelfUSE->m_pNext->m_pLast = NULL;
			}
			else
			{
				pSelfUSE->m_pLast->m_pNext = pSelfUSE->m_pNext;

				if (pSelfUSE == g_pUEnd)
				{
					g_pUEnd = pSelfUSE->m_pLast;
					delete pSelfUSE;
					USERLIST *pTemp = g_pUHead;

					while (NULL != pTemp)
					{
						UpdateFriendList(pTemp, "");
						pTemp = pTemp->m_pNext;
					}

					DoEvent("", "", "");
					ReleaseMutex(g_hMutex);
					return 0;
				}

				pSelfUSE->m_pNext->m_pLast = pSelfUSE->m_pLast;
			}
			
			delete pSelfUSE;
			USERLIST *pTemp = g_pUHead;

			while (NULL != pTemp)
			{
				UpdateFriendList(pTemp, "");
				pTemp = pTemp->m_pNext;
			}

			DoEvent("", "", "");
			ReleaseMutex(g_hMutex);
		}
		else if (PLEASEOFFLINE == nEvent)	//好友请求(离线)
		{
			g_hMutex = CreateMutex(NULL,TRUE,NULL);
			FRIENDOPERATE fo = *(FRIENDOPERATE *)pBuf;
			char szBuf[50] = "./FriendOperate/";
			strcat(szBuf, fo.m_szUser);
			strcat(szBuf, ".txt");
			FILE *fp = fopen(szBuf, "a+");
			fprintf(fp, "1");
			fprintf(fp, "\n");
			fprintf(fp, fo.m_szFriend);
			fprintf(fp, "\n");
			fclose(fp);
			ReleaseMutex(g_hMutex);
		}
	}

	return 0;
}

void AddFriendList(USERLIST *pTemp, char *pStr)		//好友列表增加
{
	if (NULL != pTemp && NULL != pStr)
	{
		char szBuf[25] = "./Friend/";
		strcat(szBuf, pTemp->m_szID);
		strcat(szBuf, ".txt");
		FILE *fp = fopen(szBuf, "a+");
		fprintf(fp, pStr);
		fprintf(fp, "\n");
		fclose(fp);
	}
}

void CreateUSERLIST(RELO *rl, int nEvent, sockaddr_in *sin, USERLIST **pNew, SOCKET nSock)	//在线链表
{
	if (NULL != rl)
	{
		if (NULL == g_pUHead)
		{
			strcpy((*pNew)->m_szID, rl->m_szID);
			(*pNew)->m_pHead = NULL;
			(*pNew)->m_pEnd = NULL;
			(*pNew)->m_pNext = NULL;
			(*pNew)->m_pLast = NULL;
			(*pNew)->m_Socket = nSock;
			g_pUHead = (*pNew);
			g_pUEnd = (*pNew);

			if (ONLINELOGIN == nEvent)
			{
				(*pNew)->m_nState = ONLINE;
			}
			else if (HIDINLOGIN == nEvent)
			{
				(*pNew)->m_nState = HIDING;
			}
		}
		else
		{
			strcpy((*pNew)->m_szID, rl->m_szID);
			(*pNew)->m_pNext = NULL;
			(*pNew)->m_pLast = g_pUEnd;
			g_pUEnd->m_pNext = (*pNew);
			g_pUEnd = (*pNew);
			(*pNew)->m_Socket = nSock;

			if (ONLINELOGIN == nEvent)
			{
				(*pNew)->m_nState = ONLINE;
			}
			else if (HIDINLOGIN == nEvent)
			{
				(*pNew)->m_nState = HIDING;
			}
		}
	}
}

void UpdateFriendList(USERLIST *pTemp, char *pleaseID)	
{
	if (NULL != pTemp)
	{
		char szBuf[25] = "./Friend/";
		strcat(szBuf, pTemp->m_szID);
		strcat(szBuf, ".txt");
		FILE *fp = fopen(szBuf, "a+");

		while (0 == feof(fp))
		{
			FRIENDLIST *pNew = new FRIENDLIST;
			strcpy(pNew->m_szID, "");
			fscanf(fp, "%s", pNew->m_szID);

			if (!strcmp(pNew->m_szID, ""))
			{
				break;
			}
			
			USERLIST *pFind = g_pUHead;

			if (DELETENOW == g_npleaseEvent && !strcmp(pNew->m_szID, pleaseID))
			{
				continue;
			}

			if (NULL == pTemp->m_pHead)
			{
				pNew->m_pLast = NULL;
				pNew->m_pNext = NULL;
				pTemp->m_pHead = pNew;
				pTemp->m_pEnd = pNew;
			}
			else
			{
				pNew->m_pNext = NULL;
				pNew->m_pLast = pTemp->m_pEnd;
				pTemp->m_pEnd->m_pNext = pNew;
				pTemp->m_pEnd = pNew;
			}

			while (NULL != pFind)
			{
				if (!strcmp(pFind->m_szID, pNew->m_szID))
				{
					pNew->m_nState = pFind->m_nState;
					break;
				}
				else
				{
					pNew->m_nState = OFFLINE;
				}
				
				pFind = pFind->m_pNext;
			}
		}

		fclose(fp);

		FILE *fp1 = fopen(szBuf, "w+");
		FRIENDLIST *pSave = pTemp->m_pHead;

		while (NULL != pSave)
		{
			fprintf(fp1, pSave->m_szID);
			fprintf(fp, "\n");
			pSave = pSave->m_pNext;
		}

		fclose(fp1);
	}
}

void DeleteFriendList(USERLIST *pSelfUSE)	//清空好友列表链表
{
	if (NULL != pSelfUSE->m_pHead)
	{
		FRIENDLIST *pTemp = pSelfUSE->m_pHead;
		FRIENDLIST *pTempNext = NULL;

		while (NULL != pTemp)
		{
			pTempNext = pTemp->m_pNext;
			delete pTemp;
			pTemp = pTempNext;
		}

		pSelfUSE->m_pHead = NULL;
		pSelfUSE->m_pEnd = NULL;
	}
}

void CleanOffDoList()
{
	if (NULL != OffDoHead)
	{
		OFFDO *pTemp = OffDoHead;
		OFFDO *pTempNext = NULL;

		while (NULL != pTemp)
		{
			pTempNext = pTemp->pNext;
			delete pTemp;
			pTemp = pTempNext;
		}

		OffDoHead = NULL;
		OffDoEnd = NULL;
	}
}
