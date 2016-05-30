#include "gameroom.h"
int get_lbound(struct gameroom g_room[]){
	int i = 0;
	for(i = 0 ; i < GAMEROOM_MAX ; i++){
		if(g_room[i].occupied == false){
			return i;
		}		
	}
}

