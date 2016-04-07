#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "sockstruct.h"

#define PORT 9090
void message_loop(int sock){
	struct g_packet gp;
	memset(&gp,0x00,sizeof(struct g_packet));
	recv(sock, &gp,sizeof(struct g_packet), 0);

	printf("packtype : %d\nfrom: %d\n",gp.PACK_TYPE, gp.uid);
}
int main(){
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
