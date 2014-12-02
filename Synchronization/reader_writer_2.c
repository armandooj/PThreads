#include "reader_writer.h"
#include "reader_writer_tracing.h"
 
extern tracing_t t; 

typedef struct reader_writer {
  pthread_mutex_t mutex;
  pthread_cond_t cond_reader;
  pthread_cond_t cond_writer;
  int readers;
} reader_writer_s;

reader_writer_t rw_init() {
  reader_writer_t rw = malloc(sizeof(reader_writer_s));

  pthread_mutex_init(&rw->mutex, NULL);
  
  pthread_cond_init(&rw->cond_writer, NULL);
  pthread_cond_init(&rw->cond_reader, NULL);

  return rw; 
}

void begin_read(reader_writer_t rw, int value){
  pthread_mutex_lock(&rw->mutex);
  rw->readers++;
  tracing_record_event_with_value(t, BR_EVENT_ID, value); 
  pthread_mutex_unlock(&rw->mutex);  
}

void end_read(reader_writer_t rw, int value){
  pthread_mutex_lock(&rw->mutex);
  rw->readers--;    

  if (rw->readers == 0) {
    pthread_cond_signal(&rw->cond_writer);
  }
    
  tracing_record_event_with_value(t, ER_EVENT_ID, value); 
  pthread_mutex_unlock(&rw->mutex);   
}

void begin_write(reader_writer_t rw, int value){
  pthread_mutex_lock(&rw->mutex);

  while (rw->readers > 0) {
    pthread_cond_wait(&rw->cond_writer, &rw->mutex);
  }

  tracing_record_event_with_value(t, BW_EVENT_ID, value);   
}

void end_write(reader_writer_t rw, int value){
  tracing_record_event_with_value(t, EW_EVENT_ID, value);
  pthread_mutex_unlock(&rw->mutex);  
}

