/*white fir*/
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h> 
#include <windows.h> 
#include <time.h> 
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

#define DEF_DNS_ADDRESS "10.3.9.4"  	//外部DNS服务器地址
#define LOCAL_ADDRESS "127.0.0.1"		//本地DNS服务器地址
#define DNS_PORT 53						//进行DNS服务的53端口
#define BUF_SIZE 512
#define LENGTH 65
#define AMOUNT 500
#define NOTFOUND -1

//DNS报文首部
typedef struct DNSHeader
{
    unsigned short ID;
    unsigned short Flags;
    unsigned short QdCount;
    unsigned short AnCount;
    unsigned short NsCount;
    unsigned short ArCount;
} DNSHDR, *pDNSHDR;

//DNS域名解析表结构
typedef struct translate
{
	string IP;						//IP地址
	string domain;					//域名
} Translate;

//ID转换表结构
typedef struct IDChange
{
	unsigned short oldID;			//原有ID
	SOCKADDR_IN client;				//请求者套接字地址
} IDTransform;

Translate DNS_table[AMOUNT];		//DNS域名解析表
IDTransform IDTransTable[AMOUNT];	//ID转换表
int IDcount = 0;					//转换表中的条目个数
char url[LENGTH];					//域名
SYSTEMTIME sys;                     //系统时间
int Day, Hour, Minute, Second;//保存系统时间的变量

int GetTable(char *tablePath);
void GetUrl(char *recvbuf, int recvnum);
int IsFind(char* url, int num);
unsigned short RegisterNewID (unsigned short oID, SOCKADDR_IN temp);
void DisplayInfo(unsigned short newID, int find);
void introduce(void);


int main(int argc, char** argv) 
{ 
    WSADATA wsaData; 
    SOCKET  socketServer, socketLocal;				//本地DNS和外部DNS两个套接字
    SOCKADDR_IN serverName, clientName, localName;	//本地DNS、外部DNS和请求端三个网络套接字地址
    char sendbuf[BUF_SIZE];
    char recvbuf[BUF_SIZE]; 
    char tablePath[100];
    char outerDns[16];
    int iLen_cli, iSend, iRecv;
    int num;

	if (argc == 1) 
       {
		strcpy(outerDns, DEF_DNS_ADDRESS);
		strcpy(tablePath, "dnsrelay.txt");
       }

	else if (argc == 3) //-dd
       {
		strcpy(outerDns, argv[2]);
		strcpy(tablePath, "dnsrelay.txt");
	   }

	else if (argc == 4) //-d
       {
		strcpy(outerDns, argv[2]);
		strcpy(tablePath, argv[3]);
	   }
	
    introduce();
    
    cout << "Name server. " <<outerDns<<" ..."<< endl;
    cout << "Try to load table. " <<tablePath<<" ..."<< endl;
	num = GetTable(tablePath);						//获取域名解析表

	//保存系统的时间
	GetLocalTime(&sys);
	Day          = sys.wDay;
    Hour         = sys.wHour;
	Minute       = sys.wMinute;
	Second       = sys.wSecond;

	for (int i = 0; i < AMOUNT; i++) 	//初始化ID转换表
       {			
		IDTransTable[i].oldID = 0;
		memset(&(IDTransTable[i].client), 0, sizeof(SOCKADDR_IN));
	   }

    WSAStartup(MAKEWORD(2,2), &wsaData);			//初始化ws2_32.dll动态链接库

	//创建本地DNS和外部DNS套接字
    socketServer = socket(AF_INET, SOCK_DGRAM, 0);
	socketLocal = socket(AF_INET, SOCK_DGRAM, 0);

	//设置本地DNS和外部DNS两个套接字
	localName.sin_family = AF_INET;
	localName.sin_port = htons(DNS_PORT);
	localName.sin_addr.s_addr = inet_addr(LOCAL_ADDRESS);

	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(DNS_PORT);
	serverName.sin_addr.s_addr = inet_addr(outerDns);

	//绑定本地DNS服务器地址
	if (bind(socketLocal, (SOCKADDR*)&localName, sizeof(localName))) 
       {
		cout << "Binding Port 53 failed." << endl;
		exit(1);
	   }
	else
	   {
        cout << "Binding Port 53 succeed." << endl;
       }

	//本地DNS中继服务器的具体操作
	while (1) 
       {
		iLen_cli = sizeof(clientName);
        memset(recvbuf, 0, BUF_SIZE);

		//接收DNS请求
		iRecv = recvfrom(socketLocal, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&clientName, &iLen_cli);

		if (iRecv == SOCKET_ERROR) 
           {
			cout << "Recvfrom Failed: " << WSAGetLastError() << endl;
			continue;
		   }
		else if (iRecv == 0) {break;} //???
		
		else 
           {	
			GetUrl(recvbuf, iRecv);				//获取域名
			int find = IsFind(url, num);		//在域名解析表中查找


			//在域名解析表中没有找到,从外部名字服务器直接传递报文 
			if (find == NOTFOUND) 
               {
				//ID转换
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));
				unsigned short nID = htons(RegisterNewID(ntohs(*pID), clientName));
				memcpy(recvbuf, &nID, sizeof(unsigned short));

				//打印 时间 newID 功能 域名 IP
				DisplayInfo(ntohs(nID), find);  //lower-upper =>up-low
				//中继功能显示输出时候，还没得到IP呢 

				//把recvbuf转发至指定的外部DNS服务器
				iSend = sendto(socketServer, recvbuf, iRecv, 0, (SOCKADDR*)&serverName, sizeof(serverName));
				if (iSend == SOCKET_ERROR) 
                   {
					cout << "Sendto Failed: " << WSAGetLastError() << endl;
					continue;
				   }
				else if (iSend == 0){break;}

				free(pID);	//释放动态分配的内存

				//接收来自外部DNS服务器的响应报文
				iRecv = recvfrom(socketServer, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&clientName, &iLen_cli);

				//ID转换
				pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));
				int m = ntohs(*pID);
				unsigned short oID = htons(IDTransTable[m].oldID);
				memcpy(recvbuf, &oID, sizeof(unsigned short));
	/*			
                char r_buf[BUF_SIZE]={0}; 
                char buf[10]={0};
                unsigned short ip[4];
				strcpy(r_buf,&(recvbuf[iSend]));
                for(int t=0;t<strlen(r_buf);t++)
                   {
                    if(r_buf[t]==4)
                       {
                        ip[0]=r_buf[t+1];
                        ip[1]=r_buf[t+2];
                        ip[2]=r_buf[t+3];
                        ip[3]=r_buf[t+4];
                cout<<ip[0]<<'.'<<ip[1]<<'.'<<ip[2]<<'.'<<ip[3]<<endl<<"!!!!"<<endl;
                       }    
                   }
                   
                   
	           ofstream outfile("D:\\dnsrelay_new.txt", ios::app,0);

	           if(! outfile) 
                  {
		           cerr << "Open" << tablePath << "error!" <<endl;
		           exit(1);
	              }
               outfile.write(ip,4);
               outfile.close();
*/

				//从ID转换表中获取发出DNS请求者的信息
				clientName = IDTransTable[m].client;
				//把recvbuf转发至请求者处
				iSend = sendto(socketLocal, recvbuf, iRecv, 0, (SOCKADDR*)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR) 
                   {
					cout << "Sendto Failed: " << WSAGetLastError() << endl;
					continue;
				   }
				else if (iSend == 0){break;}

				free(pID);	//释放动态分配的内存
   	           }

			//在域名解析表中找到，可以自己构造报文提供服务 
			else {	
				//获取请求报文的ID
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));

				//转换ID
				unsigned short nID = RegisterNewID(ntohs(*pID), clientName);

				//打印 时间 newID 功能 域名 IP
				DisplayInfo(nID, find);

				//构造响应报文返回
				memcpy(sendbuf, recvbuf, iRecv);						//拷贝请求报文
				unsigned short a = htons(0x8180);         //QR=1,RD=1,RA=1
				memcpy(&sendbuf[2], &a, sizeof(unsigned short));		//修改标志域

				//修改回答数域
				if (strcmp(DNS_table[find].IP.c_str(), "0.0.0.0")==0)	
					{a = htons(0x0000);}	//屏蔽功能：AnswerCount=0
				else
					{a = htons(0x0001);}	//服务器功能：AnswerCount=1
				memcpy(&sendbuf[6], &a, sizeof(unsigned short));
				
				//构造DNS响应部分
				int curLen = 0;
				char answer[16];
				unsigned short Name = htons(0xc00c); 
				memcpy(answer, &Name, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned short TypeA = htons(0x0001);
				memcpy(answer+curLen, &TypeA, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned short ClassA = htons(0x0001);
				memcpy(answer+curLen, &ClassA, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned long timeLive = htonl(0x7b);
				memcpy(answer+curLen, &timeLive, sizeof(unsigned long));//ttl 32bit 
				curLen += sizeof(unsigned long);

				unsigned short IPLen = htons(0x0004);
				memcpy(answer+curLen, &IPLen, sizeof(unsigned short));
				curLen += sizeof(unsigned short);

				unsigned long IP = (unsigned long) inet_addr(DNS_table[find].IP.c_str());
				memcpy(answer+curLen, &IP, sizeof(unsigned long));
				curLen += sizeof(unsigned long);
				curLen += iRecv;

				//请求报文和响应部分共同组成DNS响应报文存入sendbuf
				memcpy(sendbuf+iRecv, answer, curLen);

				//发送DNS响应报文
				iSend = sendto(socketLocal, sendbuf, curLen, 0, (SOCKADDR*)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR) 
                   {
					cout << "Sendto Failed: " << WSAGetLastError() << endl;
					continue;
				   }
				else if (iSend == 0){break;}
			
				free(pID);		//释放动态分配的内存
			}
		}
    }

    closesocket(socketServer);	//关闭套接字
	closesocket(socketLocal);
    WSACleanup();				//释放ws2_32.dll动态链接库初始化时分配的资源

    return 0;
}

//函数：获取域名解析表
int GetTable(char *tablePath)
{
	int i = 0, j, pos = 0;
	string table[AMOUNT];

	ifstream infile(tablePath, ios::in);	//以读入方式打开文本文件

	if(! infile) 
       {
		cerr << "Open" << tablePath << "error!" <<endl;
		exit(1);
	   }

	//每次从文件中读入一行，直至读到文件结束符为止
	while (getline(infile, table[i]) && i < AMOUNT)
		{i++;} 

	if (i == AMOUNT-1)
	   {
        cout << "The DNS table memory is full. " << endl;
       } 

	for (j = 0; j < i; j++) 
       {
		pos = table[j].find(' ');
		if (pos > table[j].size())
			{
             cout << "The record is not in a correct format. " << endl;
            } 
		else{
			 DNS_table[j].IP = table[j].substr(0, pos);
			 DNS_table[j].domain = table[j].substr(pos+1);
		    }
	   }

	infile.close();		//关闭文件
	cout << "Loading records succeed. " <<i<<" names."<< endl;

	return i;			//返回域名解析表中条目个数, 写报告时重点错误 
}

//函数：获取DNS请求中的域名
void GetUrl(char *recvbuf, int recvnum)
{
	char urlname[LENGTH];
	int i = 0, j, k = 0;

	memset(url, 0, LENGTH);
	memcpy(urlname, &(recvbuf[sizeof(DNSHDR)]), recvnum-16);	//获取请求报文中的域名表示
            //-16,except Header&Qtype,Qclass 
	int len = strlen(urlname);
	
	//域名转换
	while (i < len) 
       {
		if (urlname[i] > 0 && urlname[i] <= 63)  //两个.之间不能超过63，域名最长65 
		   {
            for (j = urlname[i], i++; j > 0 ; j--, i++, k++)
				{url[k] = urlname[i];} 
		   } 
		if (urlname[i] != 0) //没到结尾 
           {
			url[k] = '.';
		    k++;
		   }
	   }
      //如果有必要应该在这里加上输入域名长度超过65的错误返回 

	url[k] = '\0';
}

//函数：判断是否在表中找到DNS请求中的域名，找到返回下标
int IsFind(char* url, int num)
{
	int find = NOTFOUND;
	char* domain;

	for (int i = 0; i < num; i++) 
       {
		domain = (char *)DNS_table[i].domain.c_str();
		if (strcmp(domain, url) == 0)   //找到
           {	
			find = i;
			//break;  //修改一下，用flag方法替换掉break,continue,etc. 
   	        i = num+1;
           }
	   }

	return find;
}

//函数：将请求ID转换为新的ID，并将信息写入ID转换表中
unsigned short RegisterNewID (unsigned short oID, SOCKADDR_IN temp)
{
	srand(time(NULL));
	IDTransTable[IDcount].oldID = oID;
	IDTransTable[IDcount].client = temp;
	IDcount++;

	return (unsigned short)(IDcount-1);	//以表中下标作为新的ID
}

//函数：打印 时间 newID 功能 域名 IP
void DisplayInfo(unsigned short newID, int find)
{
	//打印时间
	GetLocalTime( &sys );
	cout<<endl<<"================="<<endl;
    cout<<"TIME: ";
    cout<<setiosflags(ios::right)<<setw(7)<<setfill(' ') 
    <<(((sys.wDay - Day) * 24 + sys.wHour - Hour) * 60 + sys.wMinute - Minute) * 60 + sys.wSecond - Second;//设置宽度为7，right对齐方式
	cout<<endl;

	//打印转换后新的ID
	cout.setf(ios::left);
	cout<<"ID: ";
	cout<<newID;
	cout<<endl;
	
	cout <<"FUNCTION: ";
	//在表中没有找到DNS请求中的域名
	if (find == NOTFOUND) 	//中继功能
	   {   
		cout <<"中继"<<endl;
		cout <<"DOMAIN: "<<url<<endl;
	//	cout <<"IP: "<<"//。\\\\"<<endl;//施工中。。。 
	   }

	//在表中找到DNS请求中的域名
	else 
       {
        if(DNS_table[find].IP == "0.0.0.0")  //屏蔽功能
		   {
			cout <<"屏蔽"<<endl;
		    cout <<"DOMAIN: *"<<url<<endl;
		    cout <<"IP: "<<"0.0.0.0"<<endl;
  	       }

        else //服务器功能
           {
            cout <<"服务器"<<endl;
		    cout <<"DOMAIN: "<<url<<endl;
		    cout <<"IP: "<<DNS_table[find].IP<<endl;
		   }
	   }
}

void introduce(void)
{
    cout<<"DNSRELAY LJS-GBY-CHB"<<endl<<"Good Luck, Have Fun~"<<endl;
    cout<<"dnsrelay[-d|-dd][dns-server-ipaddr][filename]"<<endl<<endl; 
}
