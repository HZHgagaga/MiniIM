#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define CREATE 1			 //注册
#define CREATEOK 2			 //注册成功
#define CREATENO 3			 //注册用户已存在
#define ONLINELOGIN 4		 //在线登录
#define HIDINLOGIN 5		 //隐身登录
#define LOGINOK 6			 //登录成功
#define LOGINNOA 7			 //登录失败(密码错误)
#define LOGINNOB 8			 //登录失败(账户不存在)
#define LOGINNOC 9			 //登录失败(用户已登录)	 
#define ONLINE 11			 //在线状态更改:在线
#define HIDING 22			 //在线状态更改:隐身
#define OFFLINE 33			 //离线
#define FRIEND 44			 //好友列表
#define QUERY 55			 //查询
#define ADDFRIEND 66		 //添加好友
#define DELETENOW 77		 //删除好友(在线)
#define DELETEOFF 88		 //删除好友(离线)
#define OFFSAY 99			 //离线消息
#define PLEASENOW 111		 //好友请求(在线)
#define PLEASEOFFLINE 222	 //好友请求(离线)
#define FRIENDOK 333		 //好友请求同意
#define FRIENDNO 444		 //好友请求拒绝
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