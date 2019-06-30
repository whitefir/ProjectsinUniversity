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

#define DEF_DNS_ADDRESS "10.3.9.4"  	//�ⲿDNS��������ַ
#define LOCAL_ADDRESS "127.0.0.1"		//����DNS��������ַ
#define DNS_PORT 53						//����DNS�����53�˿�
#define BUF_SIZE 512
#define LENGTH 65
#define AMOUNT 500
#define NOTFOUND -1

//DNS�����ײ�
typedef struct DNSHeader
{
    unsigned short ID;
    unsigned short Flags;
    unsigned short QdCount;
    unsigned short AnCount;
    unsigned short NsCount;
    unsigned short ArCount;
} DNSHDR, *pDNSHDR;

//DNS����������ṹ
typedef struct translate
{
	string IP;						//IP��ַ
	string domain;					//����
} Translate;

//IDת����ṹ
typedef struct IDChange
{
	unsigned short oldID;			//ԭ��ID
	SOCKADDR_IN client;				//�������׽��ֵ�ַ
} IDTransform;

Translate DNS_table[AMOUNT];		//DNS����������
IDTransform IDTransTable[AMOUNT];	//IDת����
int IDcount = 0;					//ת�����е���Ŀ����
char url[LENGTH];					//����
SYSTEMTIME sys;                     //ϵͳʱ��
int Day, Hour, Minute, Second;//����ϵͳʱ��ı���

int GetTable(char *tablePath);
void GetUrl(char *recvbuf, int recvnum);
int IsFind(char* url, int num);
unsigned short RegisterNewID (unsigned short oID, SOCKADDR_IN temp);
void DisplayInfo(unsigned short newID, int find);
void introduce(void);


int main(int argc, char** argv) 
{ 
    WSADATA wsaData; 
    SOCKET  socketServer, socketLocal;				//����DNS���ⲿDNS�����׽���
    SOCKADDR_IN serverName, clientName, localName;	//����DNS���ⲿDNS����������������׽��ֵ�ַ
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
	num = GetTable(tablePath);						//��ȡ����������

	//����ϵͳ��ʱ��
	GetLocalTime(&sys);
	Day          = sys.wDay;
    Hour         = sys.wHour;
	Minute       = sys.wMinute;
	Second       = sys.wSecond;

	for (int i = 0; i < AMOUNT; i++) 	//��ʼ��IDת����
       {			
		IDTransTable[i].oldID = 0;
		memset(&(IDTransTable[i].client), 0, sizeof(SOCKADDR_IN));
	   }

    WSAStartup(MAKEWORD(2,2), &wsaData);			//��ʼ��ws2_32.dll��̬���ӿ�

	//��������DNS���ⲿDNS�׽���
    socketServer = socket(AF_INET, SOCK_DGRAM, 0);
	socketLocal = socket(AF_INET, SOCK_DGRAM, 0);

	//���ñ���DNS���ⲿDNS�����׽���
	localName.sin_family = AF_INET;
	localName.sin_port = htons(DNS_PORT);
	localName.sin_addr.s_addr = inet_addr(LOCAL_ADDRESS);

	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(DNS_PORT);
	serverName.sin_addr.s_addr = inet_addr(outerDns);

	//�󶨱���DNS��������ַ
	if (bind(socketLocal, (SOCKADDR*)&localName, sizeof(localName))) 
       {
		cout << "Binding Port 53 failed." << endl;
		exit(1);
	   }
	else
	   {
        cout << "Binding Port 53 succeed." << endl;
       }

	//����DNS�м̷������ľ������
	while (1) 
       {
		iLen_cli = sizeof(clientName);
        memset(recvbuf, 0, BUF_SIZE);

		//����DNS����
		iRecv = recvfrom(socketLocal, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&clientName, &iLen_cli);

		if (iRecv == SOCKET_ERROR) 
           {
			cout << "Recvfrom Failed: " << WSAGetLastError() << endl;
			continue;
		   }
		else if (iRecv == 0) {break;} //???
		
		else 
           {	
			GetUrl(recvbuf, iRecv);				//��ȡ����
			int find = IsFind(url, num);		//�������������в���


			//��������������û���ҵ�,���ⲿ���ַ�����ֱ�Ӵ��ݱ��� 
			if (find == NOTFOUND) 
               {
				//IDת��
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));
				unsigned short nID = htons(RegisterNewID(ntohs(*pID), clientName));
				memcpy(recvbuf, &nID, sizeof(unsigned short));

				//��ӡ ʱ�� newID ���� ���� IP
				DisplayInfo(ntohs(nID), find);  //lower-upper =>up-low
				//�м̹�����ʾ���ʱ�򣬻�û�õ�IP�� 

				//��recvbufת����ָ�����ⲿDNS������
				iSend = sendto(socketServer, recvbuf, iRecv, 0, (SOCKADDR*)&serverName, sizeof(serverName));
				if (iSend == SOCKET_ERROR) 
                   {
					cout << "Sendto Failed: " << WSAGetLastError() << endl;
					continue;
				   }
				else if (iSend == 0){break;}

				free(pID);	//�ͷŶ�̬������ڴ�

				//���������ⲿDNS����������Ӧ����
				iRecv = recvfrom(socketServer, recvbuf, sizeof(recvbuf), 0, (SOCKADDR*)&clientName, &iLen_cli);

				//IDת��
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

				//��IDת�����л�ȡ����DNS�����ߵ���Ϣ
				clientName = IDTransTable[m].client;
				//��recvbufת���������ߴ�
				iSend = sendto(socketLocal, recvbuf, iRecv, 0, (SOCKADDR*)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR) 
                   {
					cout << "Sendto Failed: " << WSAGetLastError() << endl;
					continue;
				   }
				else if (iSend == 0){break;}

				free(pID);	//�ͷŶ�̬������ڴ�
   	           }

			//���������������ҵ��������Լ����챨���ṩ���� 
			else {	
				//��ȡ�����ĵ�ID
				unsigned short *pID = (unsigned short *)malloc(sizeof(unsigned short));
				memcpy(pID, recvbuf, sizeof(unsigned short));

				//ת��ID
				unsigned short nID = RegisterNewID(ntohs(*pID), clientName);

				//��ӡ ʱ�� newID ���� ���� IP
				DisplayInfo(nID, find);

				//������Ӧ���ķ���
				memcpy(sendbuf, recvbuf, iRecv);						//����������
				unsigned short a = htons(0x8180);         //QR=1,RD=1,RA=1
				memcpy(&sendbuf[2], &a, sizeof(unsigned short));		//�޸ı�־��

				//�޸Ļش�����
				if (strcmp(DNS_table[find].IP.c_str(), "0.0.0.0")==0)	
					{a = htons(0x0000);}	//���ι��ܣ�AnswerCount=0
				else
					{a = htons(0x0001);}	//���������ܣ�AnswerCount=1
				memcpy(&sendbuf[6], &a, sizeof(unsigned short));
				
				//����DNS��Ӧ����
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

				//�����ĺ���Ӧ���ֹ�ͬ���DNS��Ӧ���Ĵ���sendbuf
				memcpy(sendbuf+iRecv, answer, curLen);

				//����DNS��Ӧ����
				iSend = sendto(socketLocal, sendbuf, curLen, 0, (SOCKADDR*)&clientName, sizeof(clientName));
				if (iSend == SOCKET_ERROR) 
                   {
					cout << "Sendto Failed: " << WSAGetLastError() << endl;
					continue;
				   }
				else if (iSend == 0){break;}
			
				free(pID);		//�ͷŶ�̬������ڴ�
			}
		}
    }

    closesocket(socketServer);	//�ر��׽���
	closesocket(socketLocal);
    WSACleanup();				//�ͷ�ws2_32.dll��̬���ӿ��ʼ��ʱ�������Դ

    return 0;
}

//��������ȡ����������
int GetTable(char *tablePath)
{
	int i = 0, j, pos = 0;
	string table[AMOUNT];

	ifstream infile(tablePath, ios::in);	//�Զ��뷽ʽ���ı��ļ�

	if(! infile) 
       {
		cerr << "Open" << tablePath << "error!" <<endl;
		exit(1);
	   }

	//ÿ�δ��ļ��ж���һ�У�ֱ�������ļ�������Ϊֹ
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

	infile.close();		//�ر��ļ�
	cout << "Loading records succeed. " <<i<<" names."<< endl;

	return i;			//������������������Ŀ����, д����ʱ�ص���� 
}

//��������ȡDNS�����е�����
void GetUrl(char *recvbuf, int recvnum)
{
	char urlname[LENGTH];
	int i = 0, j, k = 0;

	memset(url, 0, LENGTH);
	memcpy(urlname, &(recvbuf[sizeof(DNSHDR)]), recvnum-16);	//��ȡ�������е�������ʾ
            //-16,except Header&Qtype,Qclass 
	int len = strlen(urlname);
	
	//����ת��
	while (i < len) 
       {
		if (urlname[i] > 0 && urlname[i] <= 63)  //����.֮�䲻�ܳ���63�������65 
		   {
            for (j = urlname[i], i++; j > 0 ; j--, i++, k++)
				{url[k] = urlname[i];} 
		   } 
		if (urlname[i] != 0) //û����β 
           {
			url[k] = '.';
		    k++;
		   }
	   }
      //����б�ҪӦ����������������������ȳ���65�Ĵ��󷵻� 

	url[k] = '\0';
}

//�������ж��Ƿ��ڱ����ҵ�DNS�����е��������ҵ������±�
int IsFind(char* url, int num)
{
	int find = NOTFOUND;
	char* domain;

	for (int i = 0; i < num; i++) 
       {
		domain = (char *)DNS_table[i].domain.c_str();
		if (strcmp(domain, url) == 0)   //�ҵ�
           {	
			find = i;
			//break;  //�޸�һ�£���flag�����滻��break,continue,etc. 
   	        i = num+1;
           }
	   }

	return find;
}

//������������IDת��Ϊ�µ�ID��������Ϣд��IDת������
unsigned short RegisterNewID (unsigned short oID, SOCKADDR_IN temp)
{
	srand(time(NULL));
	IDTransTable[IDcount].oldID = oID;
	IDTransTable[IDcount].client = temp;
	IDcount++;

	return (unsigned short)(IDcount-1);	//�Ա����±���Ϊ�µ�ID
}

//��������ӡ ʱ�� newID ���� ���� IP
void DisplayInfo(unsigned short newID, int find)
{
	//��ӡʱ��
	GetLocalTime( &sys );
	cout<<endl<<"================="<<endl;
    cout<<"TIME: ";
    cout<<setiosflags(ios::right)<<setw(7)<<setfill(' ') 
    <<(((sys.wDay - Day) * 24 + sys.wHour - Hour) * 60 + sys.wMinute - Minute) * 60 + sys.wSecond - Second;//���ÿ��Ϊ7��right���뷽ʽ
	cout<<endl;

	//��ӡת�����µ�ID
	cout.setf(ios::left);
	cout<<"ID: ";
	cout<<newID;
	cout<<endl;
	
	cout <<"FUNCTION: ";
	//�ڱ���û���ҵ�DNS�����е�����
	if (find == NOTFOUND) 	//�м̹���
	   {   
		cout <<"�м�"<<endl;
		cout <<"DOMAIN: "<<url<<endl;
	//	cout <<"IP: "<<"//��\\\\"<<endl;//ʩ���С����� 
	   }

	//�ڱ����ҵ�DNS�����е�����
	else 
       {
        if(DNS_table[find].IP == "0.0.0.0")  //���ι���
		   {
			cout <<"����"<<endl;
		    cout <<"DOMAIN: *"<<url<<endl;
		    cout <<"IP: "<<"0.0.0.0"<<endl;
  	       }

        else //����������
           {
            cout <<"������"<<endl;
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
