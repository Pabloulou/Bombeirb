/*******************************************************************************
 * This file is part of Bombeirb.
 * Copyright (C) 2018 by Laurent Réveillère
 ******************************************************************************/
#include <SDL/SDL_image.h>
#include <assert.h>

#include <player.h>
#include <sprite.h>
#include <window.h>
#include <misc.h>
#include <constant.h>
#include <game.h>
#include <map.h>


struct player* player_init(int bombs) {
	struct player* player = malloc(sizeof(*player));
	if (!player)
		error("Memory error");

	player->direction = SOUTH;
	player->bombs = bombs;
	player->lives=3;
	player->range=2;
	player->keys = 1;

	return player;
}


void player_set_position(struct player *player, int x, int y) {
	assert(player);
	player->x = x;
	player->y = y;
}


void player_free(struct player* player) {
	assert(player);
	free(player);
}

int player_get_x(struct player* player) {
	assert(player != NULL);
	return player->x;
}

int player_get_y(struct player* player) {
	assert(player != NULL);
	return player->y;
}

void player_set_current_way(struct player* player, enum direction way) {
	assert(player);
	player->direction = way;
}
// Set, Increase, Decrease the number of bomb that player can put
int player_get_nb_bomb(struct player* player) {
	assert(player);
	return player->bombs;
}

void player_inc_nb_bomb(struct player* player) {
	assert(player);
	player->bombs += 1;
}

void player_dec_nb_bomb(struct player* player) {
	assert(player);
	player->bombs -= 1;
}
// Set, Increase, Decrease the number of lives of the player
int player_get_nb_lives(struct player* player) {
	assert(player);
	return player->lives;
}

void player_inc_nb_lives(struct player* player) {
	assert(player);
	player->lives += 1;
}

void player_dec_nb_lives(struct player* player) {
	assert(player);
	player->lives -= 1;
}
// Set, Increase, Decrease the bombs' range
int  player_get_range(struct player * player){
	assert(player);
	return player->range;
}
void player_inc_range(struct player * player){
	assert(player);
	player->range += 1;
}
void player_dec_range(struct player * player){
	assert(player);
	player->range -= 1;
}





static int player_move_aux(struct player* player, struct map* map, int x, int y,game* game) { // game* game

	if (!map_is_inside(map, x, y))
		return 0;

	switch (map_get_cell_type(map, x, y)) {
	case CELL_SCENERY:
		return 0;
	case CELL_BOX:
		if(player->direction == NORTH ){
			if(player_move_aux(player,map,x,y-1,game) && (map_get_cell_type(map,x,y-1)!=CELL_BOX)){
	 			return 1;
			}
		}

		if(player->direction == SOUTH ){
			if(player_move_aux(player,map,x,y+1,game) && (map_get_cell_type(map,x,y+1)!=CELL_BOX)){
				return 1;				
			}
		}
		if(player->direction == EAST){
			if(player_move_aux(player,map,x+1,y,game) && (map_get_cell_type(map,x+1,y)!=CELL_BOX)){
				return 1;
			}
		}
		if(player->direction == WEST){
			if(player_move_aux(player,map,x-1,y,game) && (map_get_cell_type(map,x-1,y)!=CELL_BOX) ){
				return 1;
			}
		}		
		return 0;			

	case CELL_BONUS:
		switch (map_get_bonus_type(map, x, y))
		{
		case BONUS_BOMB_RANGE_DEC :
			player_dec_range(player);
			return 1;
			break;			
		case BONUS_BOMB_RANGE_INC :
			player_inc_range(player);
			return 1;
			break;
		case BONUS_BOMB_NB_DEC :
			player_dec_nb_bomb(player);
			return 1;
			break;			
		case BONUS_BOMB_NB_INC :
			player_inc_nb_bomb(player);
			return 1;
			break;

		case BONUS_LIFE:
			player_inc_nb_lives(player);
			return 1;
			break;
		
		default:
			break;

		}

	case CELL_MONSTER:
		break;
	
	case CELL_KEY:
		player->keys++;
		map_set_cell_type(map,x,y, CELL_EMPTY);
		return 1;


	case CELL_DOOR:
		if ((!door_is_open(map,x,y)) && (player->keys >0)) {
			open_door(map, x, y);
			(player->keys)--;
			return 0;
		}
		if ((!door_is_open(map,x,y)) && (player->keys <0)){
			return 0;
		}
		else {
			game_set_level(game,where_door_sends(map,x,y));
			return 1;
		}
		break;

	default:
		break;
	}

	// Player has moved
	return 1;
}

int player_move(struct player* player, struct map* map, game* game) {  //game* game
	int x = player->x;
	int y = player->y;
	int move = 0;

	switch (player->direction) {
	case NORTH:
		if(map_get_cell_type(map,x,y-1)==CELL_DOOR){
			if (!door_is_open(map,x,y-1)){
				player_move_aux(player, map, x, y - 1, game);
				break;
			}
		}
		if (player_move_aux(player, map, x, y - 1, game)) {
			if(map_get_cell_type(map,x,y-1)==CELL_BOX){
				map_set_cell_type(map, x, y-1, CELL_EMPTY);
				map_set_cell_type(map, x, y-2, CELL_BOX);
			}
			
			player->y--;

			move = 1;
		}
		break;

	case SOUTH:
		if(map_get_cell_type(map,x,y+1)==CELL_DOOR){
			if (!door_is_open(map,x,y+1)){
				player_move_aux(player, map, x, y + 1, game);
				break;
			}
		}
		if (player_move_aux(player, map, x, y + 1,game)) {
			if(map_get_cell_type(map,x,y+1)==CELL_BOX){
				map_set_cell_type(map, x, y+1, CELL_EMPTY);
				map_set_cell_type(map, x, y+2, CELL_BOX);
			}			
			player->y++;
			move = 1;
		}
		break;

	case WEST:
		if(map_get_cell_type(map,x-1,y)==CELL_DOOR){
			if (!door_is_open(map,x-1,y)){
				player_move_aux(player, map, x-1, y, game);
				break;
			}
		}
		if (player_move_aux(player, map, x - 1, y,game)) {
			if(map_get_cell_type(map,x-1,y)==CELL_BOX){
				map_set_cell_type(map, x-1, y, CELL_EMPTY);
				map_set_cell_type(map, x-2, y, CELL_BOX);
			}			
			
			player->x--;
			move = 1;
		}
		break;

	case EAST:
		if(map_get_cell_type(map,x+1,y)==CELL_DOOR){
			if (!door_is_open(map,x+1,y)){
				player_move_aux(player, map, x+1, y, game);
				break;
			}
		}
		if (player_move_aux(player, map, x + 1, y,game)) {
			if(map_get_cell_type(map,x+1,y)==CELL_BOX){
				map_set_cell_type(map, x+1, y, CELL_EMPTY);
				map_set_cell_type(map, x+2, y, CELL_BOX);
			}	
			player->x++;
			move = 1;
		}
		break;
	}

	if (move && map_get_cell_type(map, x, y) != CELL_BOMB) {
		
		 map_set_cell_type(map, x, y, CELL_EMPTY);
		
	}
	return move;
}

void player_display(struct player* player) {
	assert(player);
	window_display_image(sprite_get_player(player->direction),
			player->x * SIZE_BLOC, player->y * SIZE_BLOC);
}

