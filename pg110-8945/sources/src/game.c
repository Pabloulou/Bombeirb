/*******************************************************************************
 * This file is part of Bombeirb.
 * Copyright (C) 2018 by Laurent Réveillère
 ******************************************************************************/
#include <assert.h>
#include <time.h>

#include <game.h>
#include <misc.h>
#include <window.h>
#include <sprite.h>
#include <map.h>


struct game* game_new(void) {
	sprite_load(); // load sprites into process memory

	FILE *file = fopen("creation_niv.txt", "r");
  	if (file == NULL)
		error("game_new : file opening failed \n");

	int value;
	fscanf(file, "%i", &value);


	struct game* game = malloc(sizeof(*game));
	game->maps = malloc(sizeof(struct game));
	game->levels = value;
	
	fscanf(file, "%i:", &value);

	game->level = value;
	game->player = player_init(3);
	
	int x,y;
	fscanf(file, "%i,%i", &x,&y);
	
	char *map_name = malloc(15 * sizeof(char));
	fscanf(file, "%s", map_name);
	
	for (int i=0; i<game->levels; i++){
		game->maps[i] = map_get_static(map_name, i);
	}

	//Set default location of the player
	player_set_position(game->player, x, y);

	fclose(file);
	return game;
}

void game_free(struct game* game) {
	assert(game);

	player_free(game->player);
	for (int i = 0; i < game->levels; i++)
		map_free(game->maps[i]);
}

struct map* game_get_current_map(struct game* game) {
	assert(game);
	return game->maps[game->level];
}


struct player* game_get_player(struct game* game) {
	assert(game);
	return game->player;
}

int game_get_level(struct game* game)
{
	assert(game);
	return game->level;
}

void game_set_level(struct game* game, int level) {
	assert(game);
	game->level = level;
}

void game_banner_display(struct game* game) {
	assert(game);

	struct map* map = game_get_current_map(game);

	int y = (map_get_height(map)) * SIZE_BLOC;
	// fill the banner cells 
	for (int i = 0; i < map_get_width(map); i++)
		window_display_image(sprite_get_banner_line(), i * SIZE_BLOC, y);

	int white_bloc = ((map_get_width(map) * SIZE_BLOC) - 6 * SIZE_BLOC) / 4;
	int x = white_bloc;
	y = (map_get_height(map) * SIZE_BLOC) + LINE_HEIGHT;
	window_display_image(sprite_get_banner_life(), x, y);

	x = white_bloc + SIZE_BLOC;
	window_display_image(sprite_get_number(player_get_nb_lives(game_get_player(game))), x, y);

	x = 2 * white_bloc + 2 * SIZE_BLOC;
	window_display_image(sprite_get_banner_bomb(), x, y);

	x = 2 * white_bloc + 3 * SIZE_BLOC;
	window_display_image(
			sprite_get_number(player_get_nb_bomb(game_get_player(game))), x, y);

	x = 3 * white_bloc + 4 * SIZE_BLOC;
	window_display_image(sprite_get_banner_range(), x, y);

	x = 3 * white_bloc + 5 * SIZE_BLOC;
	window_display_image(sprite_get_number(player_get_range(game_get_player(game))), x, y);
}

void game_display(struct game* game) {
	assert(game);

	window_clear();
	game_banner_display(game);
	map_display(game_get_current_map(game));
	player_display(game->player);

	window_refresh();
}
void game_bomb_explode(struct game* game,struct player* player){
		assert(game);
		assert(player); // ??

		struct map* map = game_get_current_map(game);
		int x=player_get_x(player);
		int y=player_get_y(player);
		int range=player->range;
		if(player->bombs > 0){
			player_dec_nb_bomb(player);
			map_set_cell_type(map, x, y, CELL_BOMB);
			// window_display_image(sprite_get_bomb_ttl(4), x,y);

			// while(count>0){
			// 	window_display_image(sprite_get_bomb_ttl(count), x*SIZE_BLOC, y*SIZE_BLOC);
			// 	count--;
			// }

			// map_set_cell_type(map, x, y, CELL_EMPTY);
			// while(range){
			// 	map_set_cell_type(map, x+range, y, CELL_EMPTY);
			// 	map_set_cell_type(map, x-range, y, CELL_EMPTY);
			// 	map_set_cell_type(map, x, y+range, CELL_EMPTY);
			// 	map_set_cell_type(map, x, y-range, CELL_EMPTY);
			// 	range--;
			// }
			// player_inc_nb_bomb(player);

			
		}




}
static short input_keyboard(struct game* game) {
	SDL_Event event;
	struct player* player = game_get_player(game);
	struct map* map = game_get_current_map(game);


	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			return 1;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				return 1;
			case SDLK_UP:
				player_set_current_way(player, NORTH);
				player_move(player, map, game);
				break;
			case SDLK_DOWN:
				player_set_current_way(player, SOUTH);
				player_move(player, map, game);
				break;
			case SDLK_RIGHT:
				player_set_current_way(player, EAST);
				player_move(player, map, game);
				break;
			case SDLK_LEFT:
				player_set_current_way(player, WEST);
				player_move(player, map, game);
				break;
			case SDLK_SPACE:
				game_bomb_explode(game,player);
				break;
			default:
				break;
			}

			break;
		}
	}
	return 0;
}

int game_update(struct game* game) {

	if (input_keyboard(game))
		return 1; // exit game

	return 0;
}
