#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define CREATE 1			 //ע��
#define CREATEOK 2			 //ע��ɹ�
#define CREATENO 3			 //ע���û��Ѵ���
#define ONLINELOGIN 4		 //���ߵ�¼
#define HIDINLOGIN 5		 //�����¼
#define LOGINOK 6			 //��¼�ɹ�
#define LOGINNOA 7			 //��¼ʧ��(�������)
#define LOGINNOB 8			 //��¼ʧ��(�˻�������)
#define LOGINNOC 9			 //��¼ʧ��(�û��ѵ�¼)	 
#define ONLINE 11			 //����״̬����:����
#define HIDING 22			 //����״̬����:����
#define OFFLINE 33			 //����
#define FRIEND 44			 //�����б�
#define QUERY 55			 //��ѯ
#define ADDFRIEND 66		 //��Ӻ���
#define DELETENOW 77		 //ɾ������(����)
#define DELETEOFF 88		 //ɾ������(����)
#define OFFSAY 99			 //������Ϣ
#define PLEASENOW 111		 //��������(����)
#define PLEASEOFFLINE 222	 //��������(����)
#define FRIENDOK 333		 //��������ͬ��
#define FRIENDNO 444		 //��������ܾ�
#define FRIENDHEAD 555		
#define FRIENDEND 666	 
#define NOFRIEND 777
#define SAYSAY 888
#define BYE 999
#define QUERYNO 1000

struct PROTO
{
	int m_nEvent;
	int m_nSize;
	char m_szData[];
};

struct MESSAGE
{
	char m_szFriend [20];
	char m_szMessage[235];
};

struct RELO
{
	char m_szID[20];
	char m_szPwd[20];
};

struct FRIENDLIST
{
	char m_szID[20];
	int m_nState;
	FRIENDLIST *m_pLast;
	FRIENDLIST *m_pNext;
};

struct USERLIST
{
	char m_szID[20];
	int m_nState;
	SOCKET m_Socket;
	USERLIST *m_pNext;
	USERLIST *m_pLast;
	FRIENDLIST *m_pHead;
	FRIENDLIST *m_pEnd;
};

struct OFFDO
{
	char m_szNum[5];
	char m_szFriend[20];

	OFFDO *pNext;	
};

struct FRIENDOPERATE
{
	char m_szUser[20];
	char m_szFriend[20];
};

struct PLEASEMAN
{
	char m_szUser [20];
	char m_szFriend[20];

	PLEASEMAN *m_pLast;
	PLEASEMAN *m_pNext;
};

struct QUERYOK
{
	char m_szID [20];
	int m_nState;
};

#endif