/*********************************************************************************
 *      Copyright:  (C) 2024 lingyun<lingyun>
 *                  All rights reserved.
 *
 *       Filename:  socket_server_thread.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(17/02/24)
 *         Author:  lingyun <lingyun>
 *      ChangeLog:  1, Release initial version on "17/02/24 18:51:08"
 *                 
 ********************************************************************************/

#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<getopt.h>
#include<pthread.h>
#include<ctype.h>

typedef void *(THREAD_BODY)(void *thread_arg);
void *thread_worker(void *ctx);
int thread_start(pthread_t * thread_id,THREAD_BODY * thread_workbody,void * thread_arg);

void print_usage(char *progname)
{
	printf("%s usage:\n",progname);
	printf("-p(--port): sepcify server listen port.\n");
	printf("-h(--help): print this help information.\n");
	return ;
}

int main(int argc,char **argv)
{
	int						sockfd=-1;
	int						rv=-1;
	struct sockaddr_in		servaddr;
	struct sockaddr_in		cliaddr;
	socklen_t				len=sizeof(cliaddr);
	int						port=0;
	int						clifd;
	int						ch;
	int						on=1;
	int						tid;

	struct option			opts[]={
	
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};

	while((ch=getopt_long(argc,argv,"p:h",opts,NULL))!=-1)
	{
		switch(ch)
		{
			case 'p':
					port=atoi(optarg);
					break;
			case 'h':
					print_usage(argv[0]);
					return 0;
		}
	}

	if(!port)
	{
		print_usage(argv[0]);
		return 0;
	}

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		printf("create socket failure:%s\n",strerror(errno));
		return -1;
	}
	printf("create socket[%d] successfully\n",sockfd);

	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(port);
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	rv=bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	
	if(rv<0)
	{
		printf("socket[%d] bind on port[%] failure:%s\n",sockfd,port,strerror(errno));
		return -2;
	}

	listen(sockfd,13);
	printf("start to listen on port[%d]\n",port);

	while(1)
	{
		printf("start accept new client incoming...\n");
		clifd=accept(sockfd,(struct sockaddr *)&cliaddr,&len);
		if(clifd<0)
		{
			printf("accept new client failure:%s\n",strerror(errno));
			continue;
		}

		printf("accept new client[%s:%d] successfully\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

		thread_start(&tid,thread_worker,(void *)clifd);
	}

	close(sockfd);
	return 0;
}

int thread_start(pthread_t * thread_id,THREAD_BODY * thread_workbody,void * thread_arg)
{
	int				rv=-1;
	pthread_attr_t  thread_attr;

	if(pthread_attr_init(&thread_attr))
	{
		printf("pthread_attr_init() failure:%s\n",strerror(errno));
		goto CleanUp;
	}

	if(pthread_attr_setstacksize(&thread_attr,120*1024))
	{
		printf("pthread_attr_setstacksize() failure:%s\n",strerror(errno));
		goto CleanUp;
	}

	if(pthread_attr_setdetachstate(&thread_attr,PTHREAD_CREATE_DETACHED))
	{
		printf("pthread_attr_setdetachstate() failure:%s\n",strerror(errno));
		goto CleanUp;
	}

	if(pthread_create(thread_id,&thread_attr,thread_workbody,thread_arg))
	{
		printf("create thread failure:%s\n",strerror(errno));
		goto CleanUp;
	}

	rv=0;
CleanUp:
	pthread_attr_destroy(&thread_attr);
	return rv;
}

void *thread_worker(void *ctx)
{
	int 		clifd;
	int 		rv;
	char 		buf[1024];
	int 		i;

	if(!ctx)
	{
		printf("Invalid input argumets in %s()\n",__FUNCTION__);
		pthread_exit(NULL);
	}

	clifd=(int)ctx;

	printf("cjild thread start to communicate with sock client...\n");

	while(1)
	{
		memset(buf,0,sizeof(buf));
		rv=read(clifd,buf,sizeof(buf));
		if(rv<0)
		{
			printf("read data from client sockfd[%d] failure:%s and thread will exit\n",clifd,strerror(errno));
			close(clifd);
			pthread_exit(NULL);
		}
		else if(rv==0)
		{
			printf("socket[%d] get disconnected and thread will exit\n",clifd,strerror(errno));
			close(clifd);
			pthread_exit(NULL);
		}
		else if(rv>0)
		{
			printf("read %d bytes data from server:%s\n",rv,buf);
		}
		
		for(i=0;i<rv;i++)
		{
			buf[i]=toupper(buf[i]);
		}

		rv=write(clifd,buf,rv);
		if(rv<0)
		{
			printf("write to client by sockfd[%d] failure:%s\n",clifd,strerror(errno));
			close(clifd);
			pthread_exit(NULL);
		}
		
	}
}
