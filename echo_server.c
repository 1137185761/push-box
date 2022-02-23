#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <error.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>

//#include "commonFunc.h"
#define IP "127.0.0.1"

#define debug 1

//处理http请求
int doHttpRequest(const char* srcBuf,char* dstBuf,int count,char *urlout,char *mannerout);
//处理出错响应
int do_error_response(int sock);
//处理http响应(使用硬编码方式)
int doHttpResponse(int sock,const char* url);
//处理http响应(发送文件的方式)
int doHttpResponseFile(int sock,const char* file);
//发送http头部
int sendHeader(int sock,FILE *file);
//发送html内容
int sendHtmlContent(int sock,FILE *file);
//将处理http请求和响应封装到一个函数
void* doRequestAndResponse(void* sock);

//从http请求中解析出请求行
/*
*参数
*srcBuf:浏览器发过来的请求字符串
*dstBuf:解析请求行的目的地址
*count:存放解析请求行的最大字节数
*返回值:返回0表示读取完毕,返回1表示读取成功,返回-1表示读取失败
*/
int getLine(const char* srcBuf,char* dstBuf,int count);

int main(void)
{
	/********************************/
	/*1.AF_INET:代表协议家族,表示使用ipv4协议
	/*2.SOCK_STREAM:表示面向字节流(TCP协议)
	/*3.protocal 一般指定为0
	/********************************/
	int sock = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in sockAddr;
	bzero(&sockAddr,sizeof(sockAddr));
	
	//指定协议家族(ipv4)
	sockAddr.sin_family = AF_INET;
	//ip地址(监听指定ip地址)
	//inet_pton(AF_INET,IP,sockAddr.sin_addr.s_addr);
	//ip地址(监听本机所有ip)
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockAddr.sin_port = htons(80);
	
	//将要监听的ip和端口与sock绑定
	socklen_t sockAddrLen = sizeof(sockAddr);
	int bindval = bind(sock,(struct sockaddr*)&sockAddr,sockAddrLen);
	if(bindval == 0)
	{
		printf("bind success!\n");
	}
	else
	{
		printf("bind failed,reason:%s\n",strerror(errno));
	}
	int lisVal = listen(sock,128);
	if(lisVal == 0)
        {
                printf("listen success!\n");
        }
        else
        {
                printf("listen failed,reason:%s\n",strerror(errno));
        }
	printf("wait for connect\n");
	while(1)
	{
		int clientSock;
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		bzero(&clientAddr,sizeof(clientAddr));
		
		clientSock = accept(sock,(struct sockaddr*)&clientAddr,&clientAddrLen);
		
		//处理请求并响应
		int* sockPtr = (int*)malloc(sizeof(int));
		doRequestAndResponse((void*)sockPtr);
		
		pthread_t threadID;
		pthread_create(&threadID,NULL,doRequestAndResponse,(void*)sockPtr);
		//close(clientSock);
	}
	return 0;
}

int doHttpRequest(const char* srcBuf,char* dstBuf,int count,char *urlout,char *mannerout)
{
	int len = -1;
	len = getLine(srcBuf,dstBuf,count);
	
	//如果读取成功,解析请求行
	char manner[128];
	char url[128];
	if(len == 1)
	{
		int i = 0;
		while(!isspace(dstBuf[i]) && i < sizeof(manner)-1)
		{
			manner[i] = dstBuf[i];
			i++;
		}
		
		strcpy(mannerout,manner);
		printf("the manner is:%s\n",manner);
		
		while(isspace(dstBuf[i++]));
		
		int j = 0;
		while(!isspace(dstBuf[i]) && j < sizeof(url)-1 )
		{
			url[j++] = dstBuf[i++];
		}
		strcpy(urlout,url);
		printf("the url is:%s\n",url);
	}
}

int getLine(const char* srcBuf,char* dstBuf,int count)
{
	if(!srcBuf)
	{
		if(debug)
		{
			printf("srcBuf is nullptr\n");
		}
		return -1;
	}
	
	if(!dstBuf)
	{
		if(debug)
		{
			printf("dstBuf is nullptr\n");
		}
		return -1;
	}
	int size = 0;
	//当dstBuf空间未满且请求行尚未读取完毕继续进行while循环
	while(size < count - 2 && srcBuf[size] != '\n')
	{
		if(size == 0 && srcBuf[size] == '\r')
		{
			return 0;
		}
		dstBuf[size] = srcBuf[size];
		size++;
	}
	
	//如果dstBuf空间不够
	if(size == count - 2 && dstBuf[count-2] != '\n' )
	{
		if(debug)
		{
			printf("dstBuf size is too small!\n");
			return -1;
		}
	}
	dstBuf[size] = '\0';
	printf("read success! the request is : %s\n",dstBuf);
	return 1;
}


int doHttpResponse(int sock,const char* url)
{
	const char *main_header = "HTTP/1.0 200 OK\r\nServer: Martin Server\r\nContent-Type: text/html\r\nConnection: Close\r\n";
	const char * welcome_content = "\
	<html lang=\"zh-CN\">\n\
	<head>\n\
	<meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">\n\
	<title>This is a test</title>\n\
	</head>\n\
	<body>\n\
	<div align=center height=\"500px\" >\n\
	<br/><br/><br/>\n\
	<h2>大家好，欢迎来到奇牛学院VIP 试听课！</h2><br/><br/>\n\
	<form action=\"commit\" method=\"post\">\n\
	尊姓大名: <input type=\"text\" name=\"name\" />\n\
	<br/>芳龄几何: <input type=\"password\" name=\"age\" />\n\
	<br/><br/><br/><input type=\"submit\" value=\"提交\" />\n\
	<input type=\"reset\" value=\"重置\" />\n\
	</form>\n\
	</div>\n\
	</body>\n\
	</html>";
	
	char contentSize[64];
	int len = snprintf(contentSize,64,"Content-Length:%d\r\n\r\n",strlen(welcome_content));
	
	write(sock,main_header,strlen(main_header));
	write(sock,contentSize,len);
	write(sock,welcome_content,strlen(welcome_content));
	//sprintf(responseHead,"HTTP1 ");
}

int do_error_response(int sock)
{
	const char* errorContent = "HTTP/1.0 404 NOT FOUND\r\n\
Content-Type: text/html\r\n\
\r\n\
<HTML lang=\"zh-CN\">\r\n\
<meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">\r\n\
<HEAD>\r\n\
<TITLE>NOT FOUND</TITLE>\r\n\
</HEAD>\r\n\
<BODY>\r\n\
	<P>文件不存在！\r\n\
    <P>The server could not fulfill your request because the resource specified is unavailable or nonexistent.\r\n\
</BODY>\r\n\
</HTML>";

	int len = write(sock,errorContent,strlen(errorContent));
	
	if(len == -1)
	{
		printf("send error response failed,reason:%s",strerror(errno));
	}
}

int doHttpResponseFile(int sock,const char* file)
{
	FILE *filePtr = fopen(file,"r");
	
	//发送http头部
	sendHeader(sock,filePtr);
	
	//发送html内容
	sendHtmlContent(sock,filePtr);
	return 1;
}

int sendHeader(int sock,FILE *file)
{
	char headBuf[1024];
	strcpy(headBuf,"HTTP/1.0 200 OK\r\n");
	strcat(headBuf,"Server:LiuTing Server\r\n");
	strcat(headBuf,"Content-Type:text/html\r\n");
	strcat(headBuf,"Connection:close\r\n");
	
	int fd = fileno(file);
	struct stat st;
	if(fstat(fd,&st) == -1)
	{
		fprintf(stderr,"fstat error,reason:%s\n",strerror(errno));
		do_error_response(sock);
	}
	
	char sizeContent[64];
	snprintf(sizeContent,64,"Content-Length:%ld\r\n\r\n",st.st_size);
	strcat(headBuf,sizeContent);
	
	if(send(sock,headBuf,strlen(headBuf),0)<0)
	{
		fprintf(stderr,"send falied,reason:%s\n",strerror(errno));
		return -1;
	}
	
	fprintf(stdout,"send success,content:%s\n",headBuf);
	return 1;
}

int sendHtmlContent(int sock,FILE *file)
{
	if(!file)
	{
		fprintf(stdout,"the file ptr is null\n");
		return -1;
	}
	
	//定义一个4k的缓冲区
	char sendBuf[4096];
	while(!feof(file))
	{
		if(!fgets(sendBuf,sizeof(sendBuf),file))
		{
			fprintf(stderr,"fgets error,reason:%s\n",strerror(errno));
			return -1;
		}
		
		fprintf(stdout,"write pre,content:\n%s\n",sendBuf);
		
		int len = write(sock,sendBuf,strlen(sendBuf));
		if(len < 0)
		{
			fprintf(stderr,"write error,reason:%s\n",strerror(errno));
			return -1;
		}
		fprintf(stdout,"write success,writed len :%d,content:\n%s\n",strlen(sendBuf),sendBuf);
	}
	
	return 0;
}

void* doRequestAndResponse(void* sock)
{
	char buf[4096];
	int clientSock = *(int*)sock;
	fprintf(stdout,"in doRequestAndResponse success!,client sock:%d\n",*(int*)sock);
	if(clientSock != -1)
	{
		int len = read(clientSock,buf,sizeof(buf));
		if(len != -1)
		{
			//printf("%s\n",buf);
		}
		char dstBuf[128];
		char urlout[128];
		char mannerout[128];
		doHttpRequest(buf,dstBuf,128,urlout,mannerout);
		
		//将url映射成path
		char path[128];
		sprintf(path,"./htmldocs/%s",urlout);
		
		struct stat st;
		if(stat(path,&st) == -1)
		{
			printf("point path error,reason:%s\n",strerror(errno));
			//回复出错页面
			do_error_response(clientSock);
		}
		else
		{
			//doHttpResponse(clientSock,NULL);
			fprintf(stdout,"in doRequestAndResponse-->doHttpResponseFile success!\n");
			doHttpResponseFile(clientSock,path);
		}
		
		//char writeBuf[] = "HELLO CLIENT";
		//write(clientSock,writeBuf,sizeof(writeBuf));
	}
	else
	{
		printf("accept failed,reason:%s\n",strerror(errno));
	}
	
	//释放资源
	close(*(int*)sock);
	if(sock)
		free(sock);
}