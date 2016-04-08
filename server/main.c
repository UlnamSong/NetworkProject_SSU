#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "sockstruct.h"
#include "queue.h"

#define PORT 9090
#define true 1
#ifndef MEMKEY
#define MEMKEY 1337
#endif
#define MEMSIZE 10240
struct queue *msg_queue;
void do_queue(){
	struct g_packet *gp;
	while(true){
		if(msg_queue->head != msg_queue->tail){
			gp = pop_queue();
			printf("popped : %d %d %s\n",gp->PACK_TYPE,gp->uid, gp->payload);
		}
	}

}
void message_loop(int sock){
	while(true){
		struct g_packet gp;
		memset(&gp,0x00,sizeof(struct g_packet));
		if(recv(sock, &gp,sizeof(struct g_packet), 0) > 0){
			send(sock, "\x00",1,0);
			if(insert_queue(msg_queue, gp)){
				printf("Success on push\n");
			}
			else{
				perror("Cannot push message.\n");
			}
		}
	}
}
int main(){
	init_queue(msg_queue);
	int sockfd, newsockfd, portno, clilen, pid;
	struct sockaddr_in serv_addr, cli_addr;
	int optval = 1;
	signal(SIGCHLD, SIG_IGN);

	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("Error opening socket.");
		exit(1);
	}
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	bzero((char *)&serv_addr, sizeof(serv_addr));

	portno = 9090;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		perror("Error binding socket.");
		exit(1);
	}
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	pid = fork();
	if(pid < 0){
		perror("Error on creating message loop\n");
		exit(1);
	}
	if(pid == 0){
		do_queue();	
	}
	while(1){
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd < 0){
			perror("Error on accept.");
			exit(1);
		}
		pid = fork();
		if(pid < 0){
			perror("Error on fork.");
			exit(1);
		}
		if(pid == 0){
			close(sockfd);
			message_loop(newsockfd);
			exit(0);
		}
		else{
			close(newsockfd);
		}
	}
	exit(0);
}
