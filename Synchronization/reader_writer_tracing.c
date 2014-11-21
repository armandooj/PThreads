#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "reader_writer_tracing.h"



void tracing_init(tracing_t tracing, int nb_threads){
  int i; 
  tracing->nb_threads = nb_threads; 
  tracing->events = malloc(nb_threads * sizeof(event_s)); 
  tracing->events_last_idx = calloc(nb_threads, sizeof(int));
  tracing->ids = calloc(nb_threads, sizeof(pthread_t));
  tracing->events_names = calloc(MAX_EVENTS, sizeof(char*)); 
  for(i = 0; i < nb_threads; i++){
    tracing->events[i] = malloc(MAX_EVENTS_PER_THREADS * sizeof(event_s)); 
  }


  /* Store the initial time to be substracted to get relative time stamps */
  gettimeofday(&tracing->initial_time, NULL); 

  
}

void tracing_register_thread(tracing_t tracing, int id){
  if(tracing->ids[id] != 0){
    fprintf(stderr, "TRACING ERROR : Two threads are recorded with identical ids. Aborting.\n"); 
    abort();
  }
  if(id >= tracing->nb_threads){
    fprintf(stderr, "TRACING ERROR : Provided id is not within the valid range ([0, nb_threads[). Aborting.\n"); 
    abort();
  }

  tracing->ids[id] = pthread_self(); 
}

int tracing_get_thread_id(tracing_t tracing){
  pthread_t self = pthread_self(); 
  int i; 
  for(i = 0; i < tracing->nb_threads; i++){
    if(tracing->ids[i] == self)
      return i; 
  }


  fprintf(stderr, "TRACING ERROR : The function has been called from a non registered\
     thread, call tracing_register_thread(). Aborting.\n");
  abort();

}
void tracing_register_event(tracing_t tracing, int event_id, char *event_name){
  if(event_id < 0 || event_id >= MAX_EVENTS){
    fprintf(stderr, "TRACING ERROR : event ids must be within the range [0, MAX_EVENTS[. Aborting.\n");
    abort();
  }

  tracing->events_names[event_id] = malloc(strlen(event_name)+1); 
  strcpy(tracing->events_names[event_id], event_name); 
}

void tracing_record_event(tracing_t tracing, int event_type){
  int thread_id = tracing_get_thread_id(tracing);
  if(tracing->events_last_idx[thread_id] >= MAX_EVENTS_PER_THREADS-1){
  fprintf(stderr, "TRACING ERROR : Number of events have reached its maximum value per\
threads, change MAX_EVENTS_PER_THREADS. Aborting.\n");
  }
    
  event_t new_event = &tracing->events[thread_id][tracing->events_last_idx[thread_id]++];

  /* store event time */ 
  gettimeofday(&new_event->time, NULL); /* get current time */
  /* store event type */
  new_event->type = event_type; 
}

static char *tv_to_string(struct timeval *tv, char *buf, int buf_size){
  double ms = tv->tv_usec / 1000.; 
  snprintf(buf, buf_size, "%lds %.3fms", tv->tv_sec, ms); 
  return buf; 
}
 
static struct timeval tv_minus(struct timeval tv1, struct timeval tv2){
  struct timeval tv; 
  tv.tv_usec = tv1.tv_usec - tv2.tv_usec; /* long int */
  tv.tv_sec = 0; 
  while(tv.tv_usec < 0){ /* deal with carry */
    tv.tv_sec--; 
    tv.tv_usec+=1000000;
  }
  tv.tv_sec += tv1.tv_sec - tv2.tv_sec;
  return tv; 
}

/* returns 1 if tv1 < tv2 */
static int tv_lt(struct timeval tv1, struct timeval tv2){
  if(tv1.tv_sec < tv2.tv_sec)
    return 1; 
  if(tv1.tv_sec == tv2.tv_sec)
    if (tv1.tv_usec < tv2.tv_usec)
      return 1; 
  return 0; 
}

int tracing_compare_event_date(event_t e1, event_t e2){
  if(tv_lt(e1->time, e2->time)){
    return 1; 
  }
  else if (tv_lt(e2->time, e1->time))
    return -1; 
  return 0; 
}

char *tracing_event_to_string(tracing_t tracing, int event_id, char *buf, int buf_size){
  if(tracing->events_names[event_id]){
  snprintf(buf, buf_size, "%s ",tracing->events_names[event_id]); 

  }
  else{
    snprintf(buf, buf_size, "%d ", event_id); 
  }
  return buf; 
}

void print_all_events_per_thread(tracing_t tracing){
  int i;   
  for(i = 0; i < tracing->nb_threads; i++){
    int j; 
    
    for(j = 0; j < tracing->events_last_idx[i]; j++){
      char buf[64]; 
      /* computes time relatives to call to tracing_init() call */
      struct timeval tv = 
	tv_minus(tracing->events[i][j].time, tracing->initial_time); 

      printf("THREAD %d TIME: +%s, TYPE : ", 
	     i, tv_to_string(&tv, buf, 64));
      
      printf("%s\n", tracing_event_to_string(tracing, tracing->events[i][j].type, buf, 64));
    }
  }
}

void print_all_events_per_date(tracing_t tracing){
  int i;   
  
  int *current_idx = calloc(tracing->nb_threads, sizeof(int)); 

  struct timeval min; 
  int min_idx; 

  for(;;){
  min.tv_sec = LONG_MAX; min.tv_usec = 0;
  min_idx = -1; 
    for(i = 0; i < tracing->nb_threads; i++){
      if(current_idx[i] < tracing->events_last_idx[i]){
	struct timeval current = tracing->events[i][current_idx[i]].time; 
	if(tv_lt(current, min)){
	  min = current; 
	  min_idx = i; 
	}
      }
    }

    if(min_idx != -1){
      char buf[64]; 
      /* computes time relatives to call to tracing_init() call */
      struct timeval tv = 
	tv_minus(min, tracing->initial_time); 
    
      printf("THREAD %d TIME: +%s, TYPE : ", 
	     min_idx, tv_to_string(&tv, buf, 64));
      printf("%s\n", tracing_event_to_string(tracing, tracing->events[min_idx][current_idx[min_idx]].type, buf, 64));
      current_idx[min_idx]++; 
    }
    else{
      break; 
    }
  }
}

