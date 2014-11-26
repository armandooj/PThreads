#ifndef   	_READER_WRITER_TRACING_H_
#define   	_READER_WRITER_TRACING_H_

#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_EVENTS_PER_THREADS 65536
#define MAX_EVENTS 16


typedef struct{
  struct timeval time; /* time stamp (see man gettimeofday)*/
  int type;
  int value;
}event_s, *event_t; 

typedef struct{
  int nb_threads; 
  event_s **events; /* per thread array of events */
  int *events_last_idx; /* indexes of the last event for each thread event array */
  pthread_t *ids; 
  struct timeval initial_time; 
  char **events_names;
}tracing_s, *tracing_t; 

/* Initialize the thread event tracing system. */
void tracing_init(tracing_t tracing, int nb_threads); 

/* Register the current thread as thread nmbr \id.  Threads must be
   registered in order to use other tracing functions. 

   Check linked_list_simple_test.c for usage info.
   
   \warning The function will call abort() if \id is not within the
   range [0, nb_threads[

   \warning The function will call abort() if two threads are recorded
   with the same id */
void tracing_register_thread(tracing_t tracing, int id); 

/* Returns the the thread id 

   \warning This function will call abort() if the function is called
   from an unregistred thread. See tracing_register_thread() for more
   information */
int tracing_get_thread_id(tracing_t tracing); 


/* Register a new event type, this is essentially to associate 
   event ids with event names.

   \warning \id must be within the range [0,MAX_EVENTS[  */
void tracing_register_event(tracing_t tracing, int event_id, char *event_name); 


/* Records a new event in the event list.
   
   \warning This function will call abort() if the number of events
   recorded for the current thread reaches MAX_EVENTS_PER_THREADS
 */
void tracing_record_event(tracing_t tracing, int event_type); 

/* Records a new event in the event list.
   
   \warning This function will call abort() if the number of events
   recorded for the current thread reaches MAX_EVENTS_PER_THREADS
 */
void tracing_record_event_with_value(tracing_t tracing, int event_type, int value);

/* return 1, if e1 precedes e2, 0 if they occures simultaneously -1 if
   e2 precedes e1 */
int tracing_compare_event_date(event_t e1, event_t e2);

/* Prints all recorded events sorted by occuring time */ 
void print_all_events_per_date(tracing_t tracing); 

/* Prints all recorded events sorted by thread id */ 
void print_all_events_per_thread(tracing_t tracing); 
#endif	    /* _READER_WRITER_TRACING_H_ */
