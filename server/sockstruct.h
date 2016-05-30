#ifndef __SOCKSTRUCT_H__
#define __SOCKSTRUCT_H__
#define P_UNDEFINED 0
#define P_LOGIN 1

struct g_login{
	char id[64];
	char pw[64];
};
struct g_gameroom{
	unsigned int p_uid[4];

};
struct g_packet{
	unsigned int PACK_TYPE;
	unsigned int uid;
	char payload[256];
};
#endif
