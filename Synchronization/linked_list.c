#include <stdlib.h>
#include <stdio.h>
#include "linked_list.h"

void list_init(struct linked_list_head *list) {
  list->head=NULL;
  list->sync = rw_init();
}


struct linked_list* list_remove(struct linked_list_head *list, int val) {
  struct linked_list **p, *ret=NULL;
  begin_write(list->sync, val);
  p=&list->head;
  while (*p) {
    if ((*p)->nb==val) {
      ret=*p;
      *p=(*p)->next;
      break;
    }
    p=&(*p)->next;
  }
  end_write(list->sync, val);
  return ret;
}

void list_insert(struct linked_list_head *list, int val){
  begin_write(list->sync, val);
  struct linked_list *new_cell = malloc(sizeof(struct linked_list)); 
  new_cell->next = list->head; 
  new_cell->nb = val;
  list->head = new_cell; 
  end_write(list->sync, val); 
}

int list_exists(struct linked_list_head *list, int val) {
  struct linked_list *p;
  begin_read(list->sync, val);
  p=list->head;
  while (p) {
    if (p->nb==val) {
      end_read(list->sync, val);
      return 1;
    }
    p=p->next;
  }
  end_read(list->sync, val);
  return 0;
}

void print_list_contents(struct linked_list_head *list) {
  struct linked_list *p;
  p = list->head;
  if (p != NULL) {
    while (p) {
      printf("%d ", p->nb);
      p = p->next;
    }
  }
}
