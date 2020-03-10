/*******************************************************************************
 * This file is part of Bombeirb.
 * Copyright (C) 2018 by Laurent Réveillère
 ******************************************************************************/
#include <SDL/SDL_image.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <map.h>
#include <constant.h>
#include <misc.h>
#include <sprite.h>
#include <window.h>
#include <game.h>

struct map {
	int width;
	int height;
	unsigned char* grid;
};


#define CELL(i,j) ( (i) + (j) * map->width)

struct map* map_new(int width, int height)
{	
	assert(width > 0 && height > 0);
	struct map* map = malloc(sizeof (struct map)); //Create the map
	if (map == NULL )  			
		error("map_new : malloc map failed \n");

	map->width = width;
	map->height = height;

	map->grid = malloc(map->height * map->width * sizeof(char));
	if (map->grid == NULL) {
		error("map_new : malloc grid failed \n");
	}

	//Clean the map
	int i, j;
	for (i = 0; i < map->width; i++)
	  for (j = 0; j < map->height; j++)
	    map->grid[CELL(i,j)] = CELL_EMPTY;

	return map;
}

int map_is_inside(struct map* map, int x, int y)
{
	assert(map);
	if((x > map->width-1) ||(x < 0) || (y > map->height-1) || (y < 0)){
		return 0;
	}
	return 1;
}

void map_free(struct map *map)
{
	if (map == NULL )
		return;
	free(map->grid);
	free(map);
}

int map_get_width(struct map* map)
{
	assert(map != NULL);
	return map->width;
}

int map_get_height(struct map* map)
{
	assert(map);
	return map->height;
}

enum cell_type map_get_cell_type(struct map* map, int x, int y)
{
	assert(map && map_is_inside(map, x, y));
	return map->grid[CELL(x,y)] & 0xf0;
}

void map_set_cell_type(struct map* map, int x, int y, enum cell_type type)
{
	assert(map && map_is_inside(map, x, y));
	map->grid[CELL(x,y)] = type;
}

int door_is_open(struct map* map, int x, int y)
{
	assert(map && map_is_inside(map, x, y));
	if (map->grid[CELL(x,y)] && 0x01 == CELL_DOOR_CLOSE){
		return 0;
	}
	return 1;
}

int where_door_sends(struct map* map, int x,int y) 
{
	assert(map && map_is_inside(map, x, y));
	return map->grid[CELL(x,y)] & 0x0e;
}

void display_bonus(struct map* map, int x, int y, unsigned char type)
{
	// bonus is encoded with the 4 most significant bits
	switch (type & 0x0f) {
	case BONUS_BOMB_RANGE_INC:
		window_display_image(sprite_get_bonus(BONUS_BOMB_RANGE_INC), x, y);
		break;

	case BONUS_BOMB_RANGE_DEC:
		window_display_image(sprite_get_bonus(BONUS_BOMB_RANGE_DEC), x, y);
		break;

	case BONUS_BOMB_NB_DEC:
		window_display_image(sprite_get_bonus(BONUS_BOMB_RANGE_DEC), x, y);
		break;

	case BONUS_BOMB_NB_INC:
		window_display_image(sprite_get_bonus(BONUS_BOMB_NB_INC), x, y);
		break;
	}
}

void display_scenery(struct map* map, int x, int  y, unsigned char type)
{
	switch (type & 0x0f) { // sub-types are encoded with the 4 less significant bits
	case SCENERY_STONE:
		window_display_image(sprite_get_stone(), x, y);
		break;

	case SCENERY_TREE:
		window_display_image(sprite_get_tree(), x, y);
		break;
	}
}

void map_display(struct map* map)
{
	assert(map != NULL);
	assert(map->height > 0 && map->width > 0);

	int x, y;
	for (int i = 0; i < map->width; i++) {
	  for (int j = 0; j < map->height; j++) {
	    x = i * SIZE_BLOC;
	    y = j * SIZE_BLOC;

	    unsigned char type = map->grid[CELL(i,j)];
	    
	    switch (type & 0xf0) {
		case CELL_SCENERY:
		  display_scenery(map, x, y, type);
		  break;
	    case CELL_BOX:
	      window_display_image(sprite_get_box(), x, y);
	      break;
	    case CELL_BONUS:
	      display_bonus(map, x, y, type);
	      break;
	    case CELL_KEY:
	      window_display_image(sprite_get_key(), x, y);
	      break;
	    case CELL_DOOR:
	      switch (type & 0x0e) {
		  		case DOOR_OPEN:
			  	  window_display_image(sprite_get_door_opened(), x, y);
			  	  break;
		  		case DOOR_CLOSE:
			  	  window_display_image(sprite_get_door_closed(), x, y);
				  break;
		  	}
	      break;
	    }
	  }
	}
}

struct map* map_get_static(char *map_name, struct game *game) //struct game *game
{	
	assert(game);
	char current_map[30];
	sprintf(current_map, "./maps/%s%d.txt",map_name,game->level);//game->level

	FILE *file = fopen(current_map, "r");
  	if (file == NULL)
		error("map_new : file opening failed \n");

	int width;
	int height;

  	fscanf(file, "%i:%i", &width ,&height);

	struct map* map = map_new(width, height);

	int size = map->height * map->width;
	int valeur;

	for (int i = 0; i<size; i++){
		fscanf(file, "%i ", &valeur);
		int type = valeur & 0xf0;
		int special = valeur & 0x0f;

		switch (type)
		{
			case 0x00:
				map->grid[i]=CELL_EMPTY;
				break;
			
			case 0x10:
				switch (special) 
				{
					case 0x01:
						map->grid[i]=CELL_STONE;
						break;

					case 0x02:
						map->grid[i]=CELL_TREE;
						break;

					case 0x04:
						map->grid[i]=CELL_PRINCESS;
						break;
				}
				break;
			
			case 0x20:
				switch (special) 
				{
					case 0x00:
						map->grid[i]=CELL_BOX;
						break;

					case 0x01:
						map->grid[i]=CELL_BOX_RANGEINC;
						break;

					case 0x02:
						map->grid[i]=CELL_BOX_RANGEDEC;
						break;

					case 0x03:
						map->grid[i]=CELL_BOX_BOMBINC;
						break;
					case 0x04:
						map->grid[i]=CELL_BOX_BOMBDEC;
						break;
					case 0x05:
						map->grid[i]=CELL_BOX_MONSTER;
						break;
					case 0x06:
						map->grid[i]=CELL_BOX_LIFE;
						break;
					}
				break;

			case 0x30:
				map->grid[i]=valeur;
				break;
			
			case 0x40:
				map->grid[i]=CELL_KEY;
				break;
			
			case 0x50:
				map->grid[i]=CELL_BONUS;
				break;
			
			case 0x60:
				map->grid[i]=CELL_MONSTER;
				break;
			
			case 0x70:
				map->grid[i]=CELL_BOMB;
				break;
		}
	}

	fclose(file);
	return map;
}
