#ifndef __GAMESTRUCT_H__
#define __GAMESTRUCT_H__
#include <stdbool.h>
struct gameroom{
	unsigned int room_idx;
	bool occupied;
	unsigned char GAME_STATUS;
	unsigned int user_count;
	unsigned int uid[4];
	unsigned int playing_song;
};
struct gamedata{
	unsigned int uid;
	unsigned int score;
	unsigned int combo;
	
};
struct user{
	bool used;
	unsigned int uid;
	unsigned int token;
	char username[64];
	unsigned int wcount;
	unsigned int lcount;
};
#endif
