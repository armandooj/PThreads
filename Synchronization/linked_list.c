#include <stdlib.h>
#include <stdio.h>
#include "linked_list.h"

void list_init(struct linked_list_head *list) {  
  list->head=NULL;
  list->sync = rw_init();
}


struct linked_list* list_remove(struct linked_list_head *list, int val) {
  struct linked_list **p, *ret=NULL;
  begin_write(list->sync);
  p=&list->head;
  while (*p) {
    if ((*p)->nb==val) {
      ret=*p;
      *p=(*p)->next;
      break;
    }
    p=&(*p)->next;
  }
  end_write(list->sync);
  return ret;
}

void list_insert(struct linked_list_head *list, int val){
  begin_write(list->sync);
  struct linked_list *new_cell = malloc(sizeof(struct linked_list)); 
  new_cell->next = list->head; 
  new_cell->nb = val;
  list->head = new_cell; 
  end_write(list->sync); 
}

int list_exists(struct linked_list_head *list, int val) {
  struct linked_list *p;
  begin_read(list->sync);
  p=list->head;
  while (p) {
    if (p->nb==val) {
      end_read(list->sync);
      return 1;
    }
    p=p->next;
  }
  end_read(list->sync);
  return 0;
}
