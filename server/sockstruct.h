#define P_UNDEFINED 0
#define P_LOGIN 1

struct g_gameroom{
	unsigned int p_uid[4];

};
struct g_packet{
	unsigned int PACK_TYPE;
	unsigned int uid;
	char payload[256];
};
