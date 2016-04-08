
struct queue{
        int head;
        int tail;
	int size;
	struct g_packet *gp;
};
extern struct queue *msg_queue;
void init_queue();
struct g_packet *pop_queue();
int insert_queue(struct queue *q, struct g_packet data);
