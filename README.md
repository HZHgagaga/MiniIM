        于2018年8月份时用纯C编写的即时通讯系统，以示纪念。
        使用win32api实现了客户端GUI和socket功能，暂时没有群聊功能。当时知识水平有限，为了尽快实现功能，只使用了简单的socket通信，每一个在线用户分配一个单独的线程，并且当时没有顾及代码风格，使用了大量的全局变量以及判断语句，也没有进行适量的注释，导致后来我自己都有点阅读困难。。。
        有好友管理和查询用户以及对话记录载入功能，都是用文件来实现。注册用户的账号密码都存放在服务端的文本文件中，服务端也用每个用户名命名一个文件用来存放好友数据，聊天记录通过服务端以及本地文本文件共同实现。当初实现方法大部分是自己瞎想，可能有很多不合理之处。
        以下是当初写的设计思路，因为写的时候发现根本没法按当初瞎想的方法实现，所以实际实现的时候其实有些许不同。。。

宏常量：

#define CREATE 1			 //注册

#define CREATEOK 2		 //注册成功

#define CREATENO 3		 //注册用户已存在

#define ONLINELOGIN 4		 //在线登录

#define HIDINLOGIN 5		 //隐身登录

#define LOGINOK 6			 //登录成功

#define LOGINNOA 7		 //登录失败(密码错误)

#define LOGINNOB 8		 //登录失败(账户不存在)

#define ONLINE 11			 //在线状态更改:在线

#define HIDING 22			 //在线状态更改:隐身

#define OFFLINE 33			 //离线

#define FRIEND 44			 //好友列表

#define QUERY 55			 //查询

#define ADDFRIEND 66		 //添加好友

#define DELETE 77			 //删除好友(在线)

#define DELETEOFF 88		 //删除好友(离线)

#define SAY 99				 //离线消息

#define PLEASENOW 111	 //好友请求(在线)

#define PLEASEOFFLINE 222	 //好友请求(离线)

#define FRIENDOK 333		 //好友请求同意

#define FRIENDNO 444		 //好友请求拒绝



结构体设计：

Stuct  PROTO(每次发送的数据包，所有的事件)

{

	int m_Event;

	int m_Size;

	char m_szData[];

};


Struct MESSAGE（离线消息用结构体）

{

	char m_szID [20];

	char m_szFriend [20];

	char m_szMessage[235];

};


Stuct  RELO（注册登录所用结构体）
{

	char m_szID[20];

	char m_szPAW[20];

};


Struct  USERLIST（用于创建链表，存放所有的服务器在线和隐私的用户）

{

	char m_szID[20];

	int m_nState;

	char m_szIP[16];

	unsignaed int m_uiPort;

	USERLIST *m_pNext;

	USERLIST *m_pLast;

	FRIENDLIST *m_pHead;

	FRIENDLIST *m_pEnd;

};


struct  FRIENDLIST(用于创建链表，存放好友列表，每一个用户都拥有一个此链表)

{

	char m_szID[20];

	int m_nState;

	char m_szIP[16];

	unsignaed int m_uiPort;

	FRIENDLIST *pLast;

	FRIENDLIST *pNext;

};


Struct FRIENDOPERATE（好友操作用结构体，用于好友增删）

{

	char m_szUser[20];

	char m_szFriend[20];

};


Struct QUERYOK（查询结果）

{

	char m_szID [20];

	int m_nState;

};



服务端：
与客户端以TCP的方式连接，在各个客户端打开后，与各个客户端保持连接，当客户端退出后，服务端与该客户端断开连接。每当登入成功一个用户，就会为该用户创建一个线程函数直到该用户登出，每当服务端读取数据包PROTO中的事件类型后，各个线程将会对其判断并且进行操作。

注册模块：在客户端进行注册操作后，服务端会收到用户注册的标志信息（存储在RELO结构体中），服务端会将注册后的账号密码存储到存储着所有用户账号密码的文件中，并且会判断是否存在重复账户名，并会反馈给客户端。

登录模块：在客户端进行登入操作后，服务端会收到用户登录的标志信息（存储在RELO结构体中），服务端将读取存储着所有注册用户账号密码的文件，并且会判断是否存在该账户以及账户密码是否正确，并会反馈给客户端。

在线状态模块：服务端将保存目前所登入的在线的与隐身的客户端列表，采用链表（结点为USERLIST结构体）的方式存储，每个节点将含有该客户端的账户名、在线状态、IP、端口号，每当有新用户登入其账户名和在线状态就会存储入该链表中，若客户端采用隐身登录，其在线状态会变成隐身，也可以在登录后更改在线状态。

好友列表模块（读取）：新的客户端在进行登入操作后或者每当服务端中的在线的与隐身的用户列表有用户的在线状态改变时。各个用户线程将会读取自己对应的存在文件中的好友列表存入各自的好友链表（结点为FRIENDLIST结构体）中（每个用户都有一个自己的好友列表文件），或者有用户线程进行好友增删操作时，会读取自己对应的存在文件中的好友列表存入各自的好友链表中。好友列表文件含有所添加的好友的账户名，链表节点成员为好友的账户名、在线状态、IP、端口号，读取该文件时，链表节点除了账户名外暂时不给初始值。

好友列表模块（发送）：各个用户线程从文件中读入时会将所有读取的好友列表与目前在线的与隐身进行比较，将所有好友列表链表（结点为FRIENDLIST结构体）中目前在线的与隐身的用户的节点的在线状态、IP、端口号更新，若是离线好友，不对IP和端口号进行赋值，然后将该链表各个节点信息发送给对应的账户客户端，发送完成后，各个用户线程的该链表将会删除。当用户进行登入或者每当服务端中的在线的与隐身的客户端列表有客户的在线状态改变时，服务端都重复以上行为，以实现动态更新。

查询用户模块：当服务端收到用户查询用户的标志信息与的需要查询的账户名信息后，先会在目前在线的链表中查询是否有此用户，若有，则将此用户信息存入QUERYOK结构体发送给客户端，若无，则会在存储着所有用户账号密码的文件中查询是否有此用户，若有，则此用户信息存入QUERYOK结构体发送给客户端，若无，则发送一个ID值为空的QUERYOK结构体。

添加好友模块(离线)：。当服务端收到用户添加好友的标志信息与的需要添加的账户名信息后（内容将存储在一个成员为发出请求用户名、被请求用户名结构体中FRIENDOPERATE），判断该账户是否在线，若离线，为收到该消息的线程的用户创建一个专属的离线好友操作文件，将需要添加的账户名以追加的方式存入文件，且每个账户名前一行标识数字1，每当有新用户登录时，各线程会读取该用户的离线好友操作文件，判断是否有该用户，若有，则向该账户发送好友请求事件和从离线好友操作文件中读取的前一行为数字1的用户名（内容将存储在一个成员为发出请求用户名、被请求用户名结构体中FRIENDOPERATE），各线程会判断该用户是否为自己，若是，则向客户端发送消息，离线好友操作文件中相应的信息也会删除。当客户端返回同意消息时（同意事件消息和发出请求的用户名），各线程判断发出请求用户名是否是自己，再将该用户添加入自己的好友列表文件中，不同意时，返回给发出请求的用户好友请求拒绝和拒绝好友的用户名消息。

添加好友模块(在线)：当服务端收到用户添加好友的标志信息与的需要添加的账户名信息后（内容将存储在一个成员为发出请求用户名、被请求用户名结构体中FRIENDOPERATE），判断需要添加的账户是否在线，若在线，各线程会判断被请求用户是否为自己，若是，则向该线程的客户端发送消息。当客户端返回同意消息时（同意事件消息和发出请求的用户名），各线程判断发出请求用户名是否是自己，再将该用户添加入自己的好友列表文件中，不同意时，返回给发出请求的用户好友请求拒绝和拒绝好友的用户名消息。

删除好友模块(离线)：当服务端收到用户删除好友的标志信息与的需要删除的好友账户名信息后（内容将存储在一个成员为发出删除请求用户名、被删除用户名结构体中FRIENDOPERATE），收到该消息的线程会将自身存储着该用户的好友文件中删除该好友信息。然后判断需要删除的好友是否在线，若离线，将需要添加的账户名以追加的方式存入文件，且每个账户名前一行标识数字0，每当有新用户登录时，各线程会读取该用户的离线好友操作文件，判断是否有该用户，若有，则向该账户发送删除好友事件和从离线好友操作文件中读取的前一行为数字0的用户名（内容将存储在一个成员为发出请求用户名、被请求用户名结构体中FRIENDOPERATE），各线程会判断该用户是否为自己，若是，离线好友操作文件中相应的信息也会删除。

删除好友模块(在线)：当服务端收到用户删除好友的标志信息与的需要删除的好友账户名信息后（内容将存储在一个成员为发出删除请求用户名、被删除用户名结构体中FRIENDOPERATE），收到该消息的线程会将自身存储着该用户的好友文件中删除该好友信息。然后判断需要删除的好友是否在线，若在线，所有用户线程判断被删除用户是否为自己，线程会在存储着该用户的好友文件中删除该好友信息。

一对一聊天模块（离线）：当服务端收到离线消息的标志信息与的需要发送的目标账户名信息后（存储在MESSAGE结构体中），存入一个成员为用户名、信息内容的结构体中，为该用户创建一个专属的离线信息文件，将结构体内容以追加的方式存入文件。当有用户登入时，会读取该用户的离线信息文件，判断是否有信息，若有，则用send()函数发送，发送完后，文件内容将清空。




客户端：
以TCP连接的方式，在进行登入操作后，与服务器保持连接，直至进行登出操作后，与服务端断开连接。当希望与好友通信时，会以UDP的方式与好友进行点对点通信，通过一个长度为255的char型字符串来传输通信的内容。客户端的接受TCP消息或者UDP消息将使用单独的线程进行操作。

注册模块：用户填入希望注册的用户密码（存储在RELO结构体中），将会把注册事件信息和注册内容发送给服务端，对服务端的反馈出现相应的提示。

登录模块：用户填入希望登录的用户密码（存储在RELO结构体中），可以进行隐身登入或者在线登入，将会把登录事件信息和登录内容发送给服务端，对服务端的反馈出现相应的提示，成功登入后，登入注册界面将会消失，出现显示好友列表及其其他操作的窗口。

在线状态模块：用户登入成功后可以选择更改在线状态，将会把更改在线状态事件信息和相应的内容发送给服务端。

好友列表模块：每个好友信息将存储在一套链表中（结点为FRIENDLIST结构体），链表结点含有的成员有：好友信息、IP、端口号。单独的TCP消息接收线程将不断接收服务端的消息，若是判断服务端发送来的消息是好友列表，将会删除原有链表，将接收到的信息添加到新的链表，并且会将链表中的好友用户名逐一显示在listbox控件中，窗口中的listbox控件中会显示所有的好友以及状态，若是有好友发送消息过来，其是否有新消息也是在listbox控件中显示，隐身好友和离线好友都将会显示离线状态。

删除好友模块：从listbox控件中单击选中好友后，可以点击删除按钮，提示是否删除后，从listbox控件中删除对应项（信息存储在FRIENDOPERATE结构体中），并且会发送删除好友事件以及删除好友的账户名给服务端。

查询用户模块：在显示好友列表的窗口中有一个按钮叫做查询，点击后，弹出一个对话框，输入希望查询的用户名后，点击查询，将会把查询事件的信息和用户名的信息发送给服务端，服务端查询到后，返回的内容将存入QUERYOK结构体中，该用户的用户名和在线状态和是否是已有的好友都会显示出来，若返回的ID值为空，则显示无此用户。

好友请求模块：当接收到服务端的好友请求信息后（信息存储在FRIENDOPERATE结构体中），会有窗口弹出，上面会显示用户名请求添加好友，可以选择同意或拒绝，然后将结果消息和请求的用户名发送给服务端。

添加好友模块：在查询到用户后，会在自身的好友链表中查询该用户名是否已经存在，若用户已是好友，则添加按钮不可点击，若不是好友，可以点击添加按钮（信息存储在FRIENDOPERATE结构体中），则发送添加好友事件的信息和用户名的信息发送给服务端。当服务端返回最终结果消息后，会有窗口弹出，上面会显示某用户名是否同意添加好友。

一对一聊天模块（在线）：每一个好友都将拥有独自的消息文件，若是有某个好友及其对应的IP和端口号接收到消息，在查看之前，listbox控件中对应项将增加新消息字样的标识。双击好友列表中的用户名后，将会弹出聊天窗口，此时将会读取对应的消息文件，将其显示在聊天窗口中，输入希望发送的内容，点击发送，发送的内容会以追加的方式保存到消息文件中。

一对一聊天模块（离线）：若对离线好友发送消息，则将发送的消息以离线消息事件发送给服务器（存储在MESSAGE结构体中）。当登陆后，TCP消息接受线程若是收到离校信息消息标志，则会将发送来的消息以追加的方式存入到消息文件中，listbox控件中对应好友项将增加新消息字样的标识。双击好友列表中的用户名后，将会弹出聊天窗口，此时将会读取对应的消息文件，将其显示在聊天窗口中。
