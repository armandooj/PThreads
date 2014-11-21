/* 
Create n thread inserting random values in a global list 
Whenever a thread found its id in the list, it returns. 
If it doesn't find its id after 10 insertions
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "linked_list.h"
#include "reader_writer.h"
#include "reader_writer_tracing.h"


struct linked_list_head list; 
tracing_t t; 

void *thread_func(void *a){
  int id = (long int)a; 
  /* mandatory ! */
  tracing_register_thread(t, id); 
  int i ;
  for(i = 0; i < 10; i++){
    list_insert(&list, rand()%10); 
    if(list_exists(&list, id)){
      printf("Thread nmbr %d, won.\n", id); 
      return NULL; 
    }
  }
  return NULL;
}

int main(int argc, char **argv){
  
  if(argc != 2){
    printf("usage : %s nb_threads\n", argv[0]); 
    exit(1);
  }

  int nb_threads = atoi(argv[1]); 
  
  srand(time(NULL)*getpid()); 

  /*** Linked list ***/
  list_init(&list); 



  /*** Tracing ... ***/
  tracing_s tracing; 
  t = &tracing; 

  /* set a timeout to kill the process in case of deadlock */
  alarm(5); 
  tracing_init(t, nb_threads); 

  /* Give a human readable name to events */
  tracing_register_event(t, BR_EVENT_ID, "BEGIN READ");
  tracing_register_event(t, ER_EVENT_ID, "END READ");
  tracing_register_event(t, BW_EVENT_ID, "BEGIN WRITE");
  tracing_register_event(t, EW_EVENT_ID, "END WRITE");

  /*** Thread creation ***/
  pthread_t *tids = malloc(sizeof(pthread_t) * nb_threads -1); 
  long int i; 

  for(i = 0; i < nb_threads-1; i++){
    pthread_create(tids+i, NULL, thread_func, (void*)(i+1)); 
  }

  thread_func((void*)0); 

  for(i = 0; i < nb_threads-1; i++){
    pthread_join(tids[i], NULL); 
  }

  print_all_events_per_date(t); 

  exit(EXIT_SUCCESS); 
}
