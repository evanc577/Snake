#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "list.h"

int main();

void gameloop();

void spawn_food(struct List* snake, int* x, int* y);

void move_snake(struct List* snake, int dir, int* food_x, int* food_y);

bool self_collision(struct List* snake);

void *wait_keyboard();

void clear_snake(struct List* snake);

void add_input(int key);

void use_input();
