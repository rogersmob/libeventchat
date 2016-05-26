#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<event.h>
#include<pthread.h>
#include<json/json.h>

#define PORT        25341
#define BACKLOG     5
#define MEM_SIZE    1024
struct event_base* base;
struct sock_ev{
   struct event* read_ev;
   struct event* write_ev;
   char* buffer;
};

void release_sock_event(struct sock_ev* ev){
	event_del(ev->read_ev);
	free(ev->read_ev);
	free(ev->write_ev);
	free(ev->buffer);
	free(ev);
}

void on_read(int sock,short event,void* arg){
	struct sock_ev* ev =(struct sock_ev*)arg;
   char buf[1024] = {0};
   int size = recv(sock,buf,1024,0);
   if(size == 0){
      release_sock_event(ev);
      printf("read error\n");
	  return ;
   }
   printf("recv:%s\n",buf);
}

void* eventfun(void* arg){
   int sockfd = *((int*)arg);
    base = event_base_new();
	struct sock_ev* ev = (struct sock_ev*)malloc(sizeof(struct sock_ev));
	ev->read_ev = (struct event*)malloc(sizeof(struct event));
	ev->write_ev = (struct event*)malloc(sizeof(struct event));
	event_set(ev->read_ev, sockfd, EV_READ|EV_PERSIST, on_read, ev);
	event_base_set(base, ev->read_ev);
	event_add(ev->read_ev, NULL);
	event_base_dispatch(base);
	return NULL;
}

int main(int argc,char **argv)
{
	if(argc < 3){
		fprintf(stderr, "usage: %s ip port\n",
				argv[0]);
		exit(-1);
	}


	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		fprintf(stderr, "socket error\n");
		exit(-1);
	}
	struct sockaddr_in  sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &sockaddr.sin_addr.s_addr);

	if(connect(sockfd, (struct sockaddr*)&sockaddr,sizeof(sockaddr)) < 0){
		fprintf(stderr, "connect error\n");
		exit(-1);
	}
	pthread_t th;
	pthread_create(&th,0,eventfun,(void*)(&sockfd));
	char buffer[1024] = {'\0'};
	ssize_t n;
	while(1)
	{
	  n = read(STDIN_FILENO,buffer,1024);
      if(write(sockfd,buffer,n)<0)
	  {
	    fprintf(stderr,"write error\n");
		continue;
	  }
	  else
	  {
	   // write(STDOUT_FILENO,buffer,n);
	  }
	}
	close(sockfd);
	return 0;
}
