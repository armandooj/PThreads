#ifndef   	_LINKED_LIST_H_
#define   	_LINKED_LIST_H_
#include "reader_writer.h"


struct linked_list {
  int nb;
  struct linked_list* next;
};	

struct linked_list_head{
  reader_writer_t sync;
  struct linked_list *head;
};

void list_init(struct linked_list_head *list);

struct linked_list* list_remove(struct linked_list_head *list, int val) ;

void list_insert(struct linked_list_head *list, int val);

int list_exists(struct linked_list_head *list, int val);

#endif 	    /* !_LINKED_LIST_H_ */
