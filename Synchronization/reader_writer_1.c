/**
* Naïve implementation using only mutexes
**/

#include "reader_writer.h"
#include "reader_writer_tracing.h"
 
extern tracing_t t; 

typedef struct reader_writer {
  pthread_mutex_t mutex;
} reader_writer_s;

reader_writer_t rw_init() {
  reader_writer_t rw = malloc(sizeof(reader_writer_s));

  pthread_mutex_init(&rw->mutex, NULL);
  
  return rw; 
}

void begin_read(reader_writer_t rw, int value){
  pthread_mutex_lock(&rw->mutex);
  tracing_record_event_with_value(t, BR_EVENT_ID, value); 
}

void end_read(reader_writer_t rw, int value){
  tracing_record_event_with_value(t, ER_EVENT_ID, value);  
  pthread_mutex_unlock(&rw->mutex);
}

void begin_write(reader_writer_t rw, int value){
  pthread_mutex_lock(&rw->mutex);
  tracing_record_event_with_value(t, BW_EVENT_ID, value);  
}

void end_write(reader_writer_t rw, int value){
  tracing_record_event_with_value(t, EW_EVENT_ID, value);
  pthread_mutex_unlock(&rw->mutex);
}

