#include "sockstruct.h"
#include "queue.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#define false 0
#define true 1
#define MEMKEY 1337
void init_queue(){
	int shm_id, shm_id2;
	void *shm_addr, *shm_addr2;
	int count;
	printf("init queue with size 1024\n");
	shm_id = shmget((key_t)MEMKEY, sizeof(struct queue), 0666|IPC_CREAT);
	if(shm_id == -1){
		perror("shm_id error");
	}
	shm_id2 = shmget((key_t)MEMKEY+1, (sizeof(struct g_packet) * 1024), 0666|IPC_CREAT);
	if(shm_id2 == -1){
		perror("shm_id2 error");
	}
	shm_addr = shmat(shm_id, (void *)0, 0);
	if(shm_addr == (void *)-1){
		perror("shm_addr error");
	}
	shm_addr2 = shmat(shm_id2,(void *)0, 0);
	if(shm_addr2 == (void *)-1){
		perror("shm_addr2 error");
	}
	msg_queue = (struct queue *)shm_addr;
	msg_queue->size =  1024;
	msg_queue->gp = (struct g_packet *)shm_addr2;
	msg_queue->tail = 0;
	msg_queue->head = 0;
}

int insert_queue(struct queue *q, struct g_packet data){
	if((q->tail + 1) % q->size == q->head){
		perror("insert_queue failed?!\n");
		return false;
	}
	memcpy(&q->gp[q->tail], &data, sizeof(struct g_packet));
	q->tail = (q->tail+1) % q->size;
	printf("inert queue success\n");
	return true;
}

struct g_packet *pop_queue(){
	if(msg_queue->head == msg_queue->tail){
		perror("WTF\n");
	}
	struct g_packet *gp;
	gp = &msg_queue->gp[msg_queue->head];
	msg_queue->head = (msg_queue->head+1) % msg_queue->size;
	return gp;
}
