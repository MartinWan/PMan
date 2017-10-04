#include "list.h"


node* node_init(void) {
  node* self = (node*) malloc(sizeof(node));
  self->next = NULL;
  return self;
}

void node_end(node* self) {
  free(self);
}

list* list_init(void) {
  list* self = (list*) malloc(sizeof(list));

  self->head = node_init(); // list impl always has dummy head

  return self;
}

void list_end(list* self) {
  node* p = self->head;

  // delete everythiing in the list
  while (p != NULL) {
    node* next = p->next;
    free(p);
    p = next;
  }

  free(self);
}

void list_prepend(list* self, node* item) {
  item->next = self->head->next;
  self->head->next = item;
}

void list_print(list* self) {
  node* p;
  int jobs = 0;
  for (p = self->head->next; p != NULL; p = p->next, jobs++) {
    printf("%d: %s\n", p->pid, p->path);
  }
  printf("Total background jobs: %d\n", jobs);
}

void list_delete(list* self, pid_t val) {
  node* p;
  node* prev = self->head;
  for (p = self->head->next; p != NULL; prev = p, p = p->next) {
    if (p->pid == val) {
      // delete node p
      prev->next = p->next;
      free(p);
      p = prev;
    }
  }
}

node* list_search(list* self, pid_t val) {
  node* p;
  for (p = self->head->next; p != NULL; p = p->next) {
    if (p->pid == val) {
      return p;
    }
  }
  return NULL;
}

