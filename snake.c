#include "snake.h"

#define BUF_SIZE 3

int rows, cols;
int ch;
int score;
bool game_end;
bool paused;
int input_buffer[BUF_SIZE];

int main() {
  srand(time(0));

  int i;
  for (i = 0; i < BUF_SIZE; ++i) {
    input_buffer[i] = 0;
  }

  initscr();
  getmaxyx(stdscr, rows, cols);
  refresh();

  WINDOW *play_area = newwin(rows-1, cols, 0, 0);
  box(play_area, 0, 0);
  touchwin(play_area);
  wrefresh(play_area);

  use_default_colors();
  start_color();
  init_pair(1, COLOR_GREEN, -1);
  init_pair(2, COLOR_RED, -1);

  char mesg[] = "Move with hjkl or arrows keys. (q)uit (p)ause";
  ch = 'l';
  game_end = false;
  paused = false;

  mvprintw(LINES-1, (cols-strlen(mesg)), "%s", mesg);

  noecho();
  cbreak();
  keypad(stdscr, true);
  curs_set(0);

  score = 0;
  char mesg_score[] = "Score:";
  mvprintw(rows-1, 0, "%s %d", mesg_score, score);
  refresh();

  gameloop(rows, cols);

  delwin(play_area);
  endwin();

  return 0;
}

void gameloop() {
  int x = cols/2, y = rows/2;

  struct Node* first = NULL;
  first = malloc(sizeof(struct Node));
  first->prev = NULL;
  first->next = NULL;
  first->x = x;
  first->y = y;

  struct List* snake = malloc(sizeof(struct List));
  snake->head = first;
  snake->tail = first;

  int food_x, food_y;
  spawn_food(snake, &food_x, &food_y);

  attron(A_BOLD | COLOR_PAIR(1));
  mvprintw(y, x, "O");
  attroff(A_BOLD | COLOR_PAIR(1));
  refresh();

  pthread_t keyboard_t;
  pthread_create(&keyboard_t, NULL, wait_keyboard, NULL);

  while (!game_end) {
    if (paused) {
      usleep(100*1000);
      continue;
    }
    use_input();
    move_snake(snake, ch, &food_x, &food_y);
    usleep(100*1000);
  }

  char mesg_end[] = "          GAME OVER. Press any button to quit";
  mvprintw(LINES-1, (cols-strlen(mesg_end)), "%s", mesg_end);
  refresh();

  pthread_join(keyboard_t, NULL);

  clear_snake(snake);
}

void spawn_food(struct List* snake, int* x, int* y) {
  struct Node* cur_node = snake->head;
  bool board[(rows-3)*(cols-2)];
  int i, j;
  for (i = 0; i < rows-3; ++i) {
    for (j = 0; j < cols-2; ++j) {
      board[i*(cols-2) + j] = true;
    }
  }

  while (cur_node) {
    board[(cur_node->y-1)*(cols-2) + (cur_node->x-1)] = false;
    cur_node = cur_node->next;
  }


  int loc = rand() % ((rows-3)*(cols-2) - (score + 1));
  int cur_idx = 0;
  while (loc >= 0) {
    if (board[cur_idx]) {
      --loc;
    }
    ++cur_idx;
  }

  int loc_x = cur_idx % (cols-2) + 1;
  int loc_y = cur_idx / (cols-2) + 1;
  attron(COLOR_PAIR(2));
  mvprintw(loc_y, loc_x, "X");
  attroff(COLOR_PAIR(2));
  refresh();

  *x = loc_x;
  *y = loc_y;
}

void move_snake(struct List* snake, int dir, int* food_x, int* food_y) {
  struct Node* new_head = NULL;
  new_head = malloc(sizeof(struct Node));
  snake->head->prev = new_head;
  new_head->next = snake->head;
  new_head->prev = NULL;
  snake->head = new_head;

  switch (dir) {
    case KEY_LEFT:
    case 'h':
      if (snake->head->next->x > 1) {
        snake->head->x = snake->head->next->x-1;
        snake->head->y = snake->head->next->y;
      } else {
        game_end = true;
      }
      break;
    case KEY_DOWN:
    case 'j':
      if (snake->head->next->y < rows-3) {
        snake->head->x = snake->head->next->x;
        snake->head->y = snake->head->next->y+1;
      } else {
        game_end = true;
      }
      break; 
    case KEY_UP:
    case 'k':
      if (snake->head->next->y > 1) {
        snake->head->x = snake->head->next->x;
        snake->head->y = snake->head->next->y-1;
      } else {
        game_end = true;
      }
      break;
    case KEY_RIGHT:
    case 'l':
      if (snake->head->next->x < cols-2) {
        snake->head->x = snake->head->next->x+1;
        snake->head->y = snake->head->next->y;
      } else {
        game_end = true;
      }
      break;
  }

  if (game_end) return;

  if (*food_x == snake->head->x && *food_y == snake->head->y) {
    spawn_food(snake, food_x, food_y);
    char mesg_score[] = "Score:";
    mvprintw(rows-1, 0, "%s %d", mesg_score, ++score);
  } else {
    struct Node* cur_tail = snake->tail;
    mvprintw(snake->tail->y, snake->tail->x, " ");
    cur_tail->prev->next = NULL;
    snake->tail = cur_tail->prev;
    free(cur_tail);
    cur_tail = NULL;
  }

  if (self_collision(snake)) {
    game_end = true;
  }

  attron(A_BOLD | COLOR_PAIR(1));
  mvprintw(snake->head->y, snake->head->x, "O");
  attroff(A_BOLD | COLOR_PAIR(1));
  refresh();
}

bool self_collision(struct List* snake) {
  struct Node* cur_node = snake->head->next;
  while (cur_node) {
    if (snake->head->x == cur_node->x && snake->head->y == cur_node->y) {
      return true;
    }
    cur_node = cur_node->next;
  }
  return false;
}

void *wait_keyboard() {
  while (true) {
    int key = getch();
    switch (key) {
      case 'h':
      case 'j':
      case 'k':
      case 'l':
      case KEY_LEFT:
      case KEY_DOWN:
      case KEY_UP:
      case KEY_RIGHT:
        add_input(key);
        break;
      case 'q':
        game_end = true;
        return NULL;
      case 'p':
        paused = !paused;
        break;
      default:
        break;
    }
    if (game_end) break;
  }
  return NULL;
}

void clear_snake(struct List* snake) {
  struct Node* cur_node = snake->head;
  struct Node* tmp;
  while (cur_node) {
    tmp = cur_node;
    cur_node = cur_node->next;
    free(tmp);
    tmp = NULL;
  }
  free(snake);
  snake = NULL;
}

void add_input(int key) {
  int i;
  for (i = 0; i < BUF_SIZE; ++i) {
    if (input_buffer[i] == 0) {
      input_buffer[i] = key;
      return;
    }
  }
  for (i = 0; i < BUF_SIZE-1; ++i) {
    input_buffer[i] = input_buffer[i+1];
  }
  input_buffer[BUF_SIZE-1] = key;
}

void use_input() {
  if (input_buffer[0] == 0) {
    return;
  }
  ch = input_buffer[0];
  int i;
  for (i = 0; i < BUF_SIZE-1; ++i) {
    input_buffer[i] = input_buffer[i+1];
  }
  input_buffer[BUF_SIZE-1] = 0;
}
