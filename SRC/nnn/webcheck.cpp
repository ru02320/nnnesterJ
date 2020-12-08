#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <string.h>
#include <stdio.h>

#include "resource.h"

#define ServerName "www.geocities.co.jp"
#define PortNumber 80
extern HINSTANCE g_main_instance;

void convert(char *str){
	int i,j;
	char tstr[8192];
	strcpy(tstr, str);
	for(j=0, i=0; tstr[i]; ++j, ++i){
		if(tstr[i]==0x0a)
			str[j++]=0x0d;
		str[j]=tstr[i];
	}
	str[j]='\0';
}



int webcheck(char *current, char *rpn, char *upstr){
	int	Status;
	WORD wsVersion;
	WSADATA wsaData;
	int soc;
	DWORD serveraddr;
	struct hostent *serverhostent;
	char sendbuf[2048];
	char recvbuf[8192];
	char webad[260];
	int buf_len;
	struct sockaddr_in serversockaddr;

	wsVersion = MAKEWORD(1,1);
	Status = WSAStartup(wsVersion, &wsaData);
	if(atexit((void (*)(void))(WSACleanup)))
		return 0;
	if(Status != 0)
		return 0;
	soc = socket(PF_INET, SOCK_STREAM, 0);
	if(soc == INVALID_SOCKET)
		return 0;

	{
		serverhostent = gethostbyname(ServerName);
		if(serverhostent == NULL){
			closesocket(soc);
			return 0;
		}
		else{
			serveraddr = *((unsigned long *)((serverhostent->h_addr_list)[0]));
		}
	}

	serversockaddr.sin_family       = AF_INET;
	serversockaddr.sin_addr.s_addr  = serveraddr;
	serversockaddr.sin_port         = htons((unsigned short)PortNumber);
	memset(serversockaddr.sin_zero,(int)0,sizeof(serversockaddr.sin_zero));
	if(connect(soc,(struct sockaddr *)&serversockaddr,sizeof(serversockaddr)) == SOCKET_ERROR){
		closesocket(soc);
		return 0;
	}

	LoadString(g_main_instance, IDS_STRING_DLGI_01 , webad, 260);

	sprintf(sendbuf,"GET %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: Mozura 0.0\r\n\r\n", webad, ServerName, PortNumber);
	if(send(soc,sendbuf,strlen(sendbuf),0) == SOCKET_ERROR){
		shutdown(soc,2);
		closesocket(soc);
		return 0;
	}

	{
		buf_len = recv(soc, recvbuf, 8192 - 1, 0);
		if(buf_len == SOCKET_ERROR || buf_len == 0){
			shutdown(soc,2);
			closesocket(soc);
			return 0;
		}
		recvbuf[buf_len] = '\0';
	}
	shutdown(soc,2);
	closesocket(soc);
	int i,j,flag=0;
	for(i=0;i<buf_len-14;++i){
		if(recvbuf[i]=='N' && !memcmp(&recvbuf[i], "NNNesterJ", 9)){
			memcpy(rpn, &recvbuf[i], 14);
			i+=14;
			for(j=14;;++j,++i){
				rpn[j]=recvbuf[i];
				if((unsigned char)rpn[j]<=0x20)
					break;
			}
			rpn[j]='\0';
			flag=1;
			break;
		}
	}
	if(flag){
		if(upstr){
			upstr[0]='\0';
			if(strcmp(current, rpn)){	//vup
				for(;i<buf_len-6;++i){
					if(recvbuf[i]=='[' && !memcmp(&recvbuf[i], "[VUP]", 5)){
						i+=5;
						for(j=0;i<buf_len ;++j,++i){
							if(recvbuf[i]=='[' && recvbuf[i-1]==0x0a)
								break;
							upstr[j]=recvbuf[i];
						}
						upstr[j]='\0';
						convert(upstr);
						return 1;
					}
				}
				return 1;
			}
			else{
				for(;i<buf_len-6;++i){
					if(recvbuf[i]=='[' && !memcmp(&recvbuf[i], "[WIP]", 5)){
						i+=5;
						for(j=0;i<buf_len ;++j,++i){
							if(recvbuf[i]=='[' && recvbuf[i-1]==0x0a)
								break;
							upstr[j]=recvbuf[i];
						}
						upstr[j]='\0';
						convert(upstr);
						return 2;
					}
				}
				return 2;
			}
		}
		else{
			if(strcmp(current, rpn))
				return 1;
			else
				return 2;
		}
	}
	return 0;
}
