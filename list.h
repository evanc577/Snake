struct Node {
  int x, y;
  struct Node *prev, *next;
};

struct List {
  struct Node *head, *tail;
};
