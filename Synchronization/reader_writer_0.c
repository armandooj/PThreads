#include "reader_writer.h"
#include "reader_writer_tracing.h"


extern tracing_t t; 

typedef struct reader_writer{
  /* Complete with whater your need for synchronization */
} reader_writer_s; 

reader_writer_t rw_init(){
  reader_writer_t rw = malloc(sizeof(reader_writer_s)); 

  /* ... */
  
  return rw; 
}


void begin_read(reader_writer_t rw, int value){
  /* ... */
  tracing_record_event(t, BR_EVENT_ID);  

  /* ... */
}

void end_read(reader_writer_t rw, int value){
    /* ... */
  tracing_record_event(t, ER_EVENT_ID);  

    /* ... */
}

void begin_write(reader_writer_t rw, int value){
  /* ... */
  tracing_record_event_with_value(t, BW_EVENT_ID, value);  

  /* ... */
}

void end_write(reader_writer_t rw, int value){
  /* ... */
  tracing_record_event_with_value(t, EW_EVENT_ID, value);  

  /* ... */
}

