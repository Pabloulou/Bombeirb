/*******************************************************************************
 * This file is part of Bombeirb.
 * Copyright (C) 2018 by Laurent Réveillère
 ******************************************************************************/
#ifndef GAME_H_
#define GAME_H_

#include <player.h>
#include <map.h>

// Abstract data type




// Create a new game
struct game* game_new();

// Free a game
void game_free(struct game* game);

// Return the player of the current game
struct player* game_get_player(struct game* game);

// Return the current map
struct map* game_get_current_map(struct game* game);

// Display the game on the screen
void game_display(struct game* game);

// update
int game_update(struct game* game);

//==========================
// expolosion management 
void game_bomb_explode(struct game* game,struct player* player);
// Set current level 
void game_set_level(struct game* game, int level);

// Get the current level
int game_get_level(struct game* game);

#endif /* GAME_H_ */
