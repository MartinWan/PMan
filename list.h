#ifndef LIST_H
#define LIST_H
#include<sys/types.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

typedef struct node {
  pid_t pid;
  char* path;
  struct node* next;
} node;

node* node_init(void);


typedef struct list {
  node* head;
} list;

list* list_init(void);
void list_prepend(list* self, node* item);
void list_print(list* self);
void list_delete(list* self, pid_t val);
void list_end(list* self);

/*
 * Returns pointer to list item with pid_t = val
 * Returns NULL if not found.
 */
node* list_search(list* self, pid_t val);

#endif
