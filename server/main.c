#include <fcntl.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <openssl/md5.h>
#ifndef __SOCKSTRUCT_H__
#include "sockstruct.h"
#endif
#ifndef __GAMESTRUCT_H__
#include "gamestruct.h"
#endif
#ifndef __GAMEROOM_H__
#include "gameroom.h"
#endif
#include "queue.h"

#define PORT 9090
#define true 1
#define false 0
#ifndef MEMKEY
#define MEMKEY 1337
#endif
#define MEMSIZE 10240
struct gameroom g_room[GAMEROOM_MAX];
struct user user_info[USER_MAX];
struct queue *msg_queue;
sqlite3 *db;
char *encode(unsigned char hashed[]){
	char *tmp = malloc(33);
	bzero(tmp,sizeof(tmp));
	int i = 0;
	for(i = 0 ; i < MD5_DIGEST_LENGTH ; i++){
		sprintf(tmp,"%s%02x",tmp,hashed[i]);
	}
	return tmp;
}

void do_queue(){
	struct g_packet *gp;
	int rc;
	char *err = NULL;
	int i = 0;
	sqlite3_stmt *res;
	char *init = "create table if not exists user(idx INTEGER AUTO INCREMENT,"
	"id varchar(64),"
	"pw varchar(32))";
	char *login_query = "select * from user where id = ? and pw = ?";
	char *id_query = "select * from user where id = ?";
	char *insert_user_query = "insert into user(id,pw) values(?, ?)";
	rc = sqlite3_exec(db,init,0,0,&err);	
	if(rc != SQLITE_OK){
		printf("error! %s\n",err);
		sqlite3_free(err);
		return;
	}
	while(true){
		if(msg_queue->head != msg_queue->tail){
			gp = pop_queue();
			printf("popped : %d %d %s\n",gp->PACK_TYPE,gp->uid, gp->payload);
			switch(gp->PACK_TYPE){
				case 1:
				{
					unsigned char hashed[MD5_DIGEST_LENGTH+1];
					struct g_login *p = (struct g_login *)gp->payload;
					char *encoded;
					printf("[DEBUG]id : %s, pw : %s\n",p->id,p->pw);
					MD5(p->pw,strlen(p->pw),hashed);
					encoded = encode(hashed);
					rc = sqlite3_prepare_v2(db,login_query,-1,&res,0);
					if(rc == SQLITE_OK){
						sqlite3_bind_text(res,1,p->id,strlen(p->id),0);
						sqlite3_bind_text(res,2,encoded,32,0);
					}
					else{
						printf("Failed to execute : %s\n",sqlite3_errmsg(db));
						free(encoded);
						return;
					}
					int step = sqlite3_step(res);
					if(step == SQLITE_ROW){
						printf("%s, %s, %s\n",sqlite3_column_text(res,0), sqlite3_column_text(res,1),
						sqlite3_column_text(res,2));
						send(gp->uid,"\x00",0,0);
					}
					else{
						printf("[DEBUG]cannot find user with id: %s, pw: %s\n",p->id,encoded);
						send(gp->uid,"\x01",1,0);
					}
					sqlite3_finalize(res);
					free(encoded);
					break;
				}
				case 0x02:
				{
					unsigned char hashed[MD5_DIGEST_LENGTH+1];
					struct g_login *p = (struct g_login *)gp->payload;
					char *encoded;
					printf("[DEBUG]regid: %s, pw %s\n", p->id, p->pw);
					MD5(p->pw, strlen(p->pw), hashed);
					encoded = encode(hashed);
					rc = sqlite3_prepare_v2(db,id_query,-1,&res,0);
					if(rc == SQLITE_OK){
						sqlite3_bind_text(res,1,p->id,strlen(p->id),0);
					} 
					else{
						printf("Failed to execute : %s\n", sqlite3_errmsg(db));
						free(encoded);
						return;
					}
					int step = sqlite3_step(res);
					if(step == SQLITE_ROW){
						printf("[DEBUG] already exists user with id %s\n",p->id);
						send(gp->uid,"\x00",1,0);
					}
					else{
						sqlite3_finalize(res);
						rc = sqlite3_prepare_v2(db,insert_user_query,-1,&res,0);
						if(rc == SQLITE_OK){
							sqlite3_bind_text(res,1,p->id,strlen(p->id), 0);
							sqlite3_bind_text(res,2,encoded,32,0);
						}
						else{
							printf("Failed to execute : %s\n",sqlite3_errmsg(db));
							return;
						}
						int step = sqlite3_step(res);
						sqlite3_finalize(res);
					}
					free(encoded);
					break;
				}
				case 0x03:
				{
					
					break;
				}

			}
		}
	}

}
void message_loop(int sock){
	while(true){
		struct g_packet gp;
		memset(&gp,0x00,sizeof(struct g_packet));
		if(recv(sock, &gp,sizeof(struct g_packet), 0) > 0){
			gp.uid=sock;
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
/*
struct gameroom{
        unsigned int room_idx;
        bool occupied;
        unsigned char GAME_STATUS;
        unsigned int user_count;
        unsigned int uid[4];
        unsigned int playing_song;
};

*/
void gameroom_init(){
	int i = 0;
	int j = 0;
	printf("[DEBUG] Gameroom initializing..\n");
	for(i = 0 ; i < GAMEROOM_MAX ; i++){
		g_room[i].room_idx = 0;
		g_room[i].occupied = false;
		g_room[i].user_count = 0;
		g_room[i].GAME_STATUS = 0;
		for(j = 0 ; j < GAMEROOM_MAX ; j++){
			g_room[i].uid[j] = 0;
		}
		g_room[i].playing_song = 0;
	}
}
void user_init(){
	int i = 0;
	printf("[DEBUG] User Info initializing..\n");
	for(i = 0 ; i < USER_MAX ; i++){
		user_info[i].used = false;
		user_info[i].uid = 0;
		user_info[i].token = 0;
		memset(&user_info[i].username, 0x00, 64);
		user_info[i].wcount = 0;
		user_info[i].lcount = 0;
	}
}
int main(){
	int sockfd, newsockfd, portno, clilen, pid;
	struct sockaddr_in serv_addr, cli_addr;
	int optval = 1;
	unsigned int randval;
	int rc;
	signal(SIGCHLD, SIG_IGN);
	init_queue(msg_queue);
	
	gameroom_init();
	user_init();
	rc = sqlite3_open("data.db",&db);
	if(rc != SQLITE_OK){
		printf("error on sqlite: %s\n",sqlite3_errmsg(db));
		sqlite3_close(db);
		return 0;
	}
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
			int randfd;
			unsigned int randval;
			close(sockfd);
			char *str;
			str = inet_ntoa(cli_addr.sin_addr);
			randfd = open("/dev/urandom",O_RDONLY);
			read(randfd,&randval,4,0);
			printf("sending uid : %x\n",randval);
			send(newsockfd,&randval,4,0);
			close(randfd);
			printf("[DEBUG]we got a new connection on %s\n",str);
			message_loop(newsockfd);
			exit(0);
		}
		else{
			close(newsockfd);
		}
	}
	sqlite3_close(db);
	exit(0);
}
