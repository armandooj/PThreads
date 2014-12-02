#include "reader_writer.h"
#include "reader_writer_tracing.h"
#include <pthread.h>
#include <stdio.h>

typedef struct cond_list {
  int count;
  pthread_cond_t condition;
  struct cond_list* next;
} cond_list_s, *cond_list_t; 

extern tracing_t t;

typedef enum {READER = 1, WRITER = -1} types;

typedef struct reader_writer {
  pthread_mutex_t mutex;
  cond_list_t head;
  cond_list_t tail;
} reader_writer_s;

reader_writer_t rw_init() {
  reader_writer_t rw = malloc(sizeof(reader_writer_s)); 
  pthread_mutex_init(&(rw->mutex), NULL);
  rw->head = NULL;
  rw->tail = NULL;
  return rw;
}

void insert(reader_writer_t rw, int type) {
  cond_list_t new_condition; 
  new_condition = malloc(sizeof(cond_list_s));
  pthread_cond_init(&(new_condition->condition), NULL);
  new_condition->count = type;
  new_condition->next = NULL;
  if ((rw->head) == NULL) {
    rw->head = new_condition; 
    rw->tail = new_condition;
  } else {
    rw->tail->next = new_condition;
    rw->tail = new_condition;
  }
}

void remove_head(reader_writer_t rw) {
  cond_list_t temp;
  temp = rw->head;
  // There was only one element
  if (rw->tail == rw->head) {
    rw->head = NULL;
    rw->tail = NULL;
  } else {
    rw->head = rw->head->next;
  } 
  pthread_cond_destroy(&(temp->condition));
  free(temp);
}

/*
Decides wether or not a thread should wait.
We handle two cases:
Readers: if only readers are reading, do not wait. But if there's someone (writer) waiting, make the thread it's next.
Writers: if nobody is in the list, write. Else add itself as the next in the list.
*/
pthread_cond_t *get_condition(reader_writer_t rw, int type) {
  pthread_cond_t *condition = NULL;

  // A writer came in
  if (type == WRITER) {
    // something was in the tail
    if (rw->tail != NULL) {
      insert(rw, WRITER);
      condition = &(rw->tail->condition);
    } else { 
      insert(rw, WRITER);      
    }
  }   
  // A reader came in
  else if (type == READER) {
    // First time, everything is null
    if (rw->tail == NULL) {
      insert(rw, READER);
    }
    // A reader got here first
    else if (rw->tail->count > 0) {
      // There was only 1 reader
      if (rw->tail == rw->head) {
        rw->head->count++;
      } else {
        rw->tail->count++;
        condition = &(rw->tail->condition);
      }
    }     
    // It there were writers
    else if (rw->tail->count < 0) {
      insert(rw, READER);
      condition = &(rw->tail->condition);
    }
  } 

  return condition;
}

/*
Deletes a condition, updating the tails an heads and letting know
the next waiting thread with a broadcast, since there may be one or more readers and 
we want to wake them all up.
*/
void delete_cond(reader_writer_t rw, int type){
  if (type == READER) {
    rw->head->count--;
    if (rw->head->count == 0) {
      remove_head(rw);
      if (rw->head != NULL) {
        pthread_cond_broadcast(&(rw->head->condition));
      }
    }
  } else if (type == WRITER) {
    remove_head(rw);
    if (rw->head != NULL)
      pthread_cond_broadcast(&(rw->head->condition));
  }
}

void begin_read(reader_writer_t rw, int value) {
  pthread_mutex_lock(&(rw->mutex));
  
  pthread_cond_t *condition;
  condition = get_condition(rw, READER);
  
  if (condition != NULL) {
    tracing_record_event_with_value(t, WR_EVENT_ID, value);
    pthread_cond_wait(condition, &(rw->mutex));
  } 

  tracing_record_event_with_value(t, BR_EVENT_ID, value);
  pthread_mutex_unlock(&(rw->mutex));
}

void end_read(reader_writer_t rw, int value) {
  pthread_mutex_lock(&(rw->mutex));
  
  delete_cond(rw, READER);
  
  tracing_record_event_with_value(t, ER_EVENT_ID, value);  
  pthread_mutex_unlock(&(rw->mutex));
}

void begin_write(reader_writer_t rw, int value) {
  pthread_mutex_lock(&(rw->mutex));
  
  pthread_cond_t *condition;
  condition = get_condition(rw, WRITER);
  
  if (condition != NULL) {
    tracing_record_event_with_value(t, WW_EVENT_ID, value);
    pthread_cond_wait(condition, &(rw->mutex)); 
  } 

  tracing_record_event_with_value(t, BW_EVENT_ID, value);  
  pthread_mutex_unlock(&(rw->mutex));
}

void end_write(reader_writer_t rw, int value) {
  pthread_mutex_lock(&(rw->mutex));
 
  delete_cond(rw, WRITER);
  
  tracing_record_event_with_value(t, EW_EVENT_ID, value);  
  pthread_mutex_unlock(&(rw->mutex));
}
