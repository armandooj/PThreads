#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "queue.h"

#define N_PHILOSOPHERS 5
#define N_BARMANS 3
#define N_BOTTLES 3
#define DRINKING_TIME 3

typedef struct {
	int id;
	int waiting_times;
	int status;
	int barman_id;
} Philosopher;

typedef struct {
	int id;
	int waiting_times;
	int status;
	int philosopher_id;
	int drink;
} Barman;

typedef enum {
	// General
	USED, FREE,
	// Barman
	WORKING,
	NONE = -1
} status;

Philosopher *philosophers[N_PHILOSOPHERS];
Barman *barmans[N_BARMANS];

// Waiting line for philosophers
Queue p_queue;

// Conditions
pthread_mutex_t p_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t p_cond[N_PHILOSOPHERS];
pthread_cond_t b_cond[N_BARMANS];

void prepare_drink(Barman *barman) {
	sleep((rand() % 5) + 2);

	printf("Barman: %d prepared drink for %d\n", barman->id + 1, barman->philosopher_id + 1);
	// Let know the philosopher the drink is done
	pthread_cond_signal(&b_cond[barman->id]);

	barman->status = FREE;
	barman->drink = NONE;
	
	// Inform the first philosopher in the queue (if any)
	int first_philosopher = pop(&p_queue);
	if (first_philosopher != NONE) {
		pthread_cond_signal(&p_cond[first_philosopher]);
		printf(" and informed %d\n", first_philosopher + 1);
	}
}

void drink(Philosopher *philosopher) {
	printf("P: %d drinking\n", philosopher->id + 1);
	sleep(DRINKING_TIME);
	philosopher->waiting_times = 0;
	philosopher->barman_id = NONE;
}

void request_drink(Barman *barman, int drink) {
	pthread_mutex_lock(&p_mutex); /// ?????? 
	barman->drink = drink;
	printf("P: %d requesting drink\n", barman->philosopher_id + 1);
	// And wait until it's ready
	pthread_cond_wait(&b_cond[barman->id], &p_mutex);
	pthread_mutex_unlock(&p_mutex);
}

// Request a barman or wait for one in a queue
void get_barman(Philosopher *philosopher) {
	pthread_mutex_lock(&p_mutex);
	int barman_id = NONE;

	while (barman_id == NONE) {		
		int i;
		for (i = 0; i < N_BARMANS; i++) {
			// A barman is available
			if (barmans[i]->status == FREE) {
				philosopher->barman_id = barmans[i]->id;
				barmans[i]->status = WORKING;
				barmans[i]->philosopher_id = i;
				barman_id = i;
				i = N_BARMANS;
			}
		}

		if (barman_id == NONE) {
			// If we get here it means no barman is available
			philosopher->waiting_times++;			
			// Insert the philosopher in the waiting queue
			push(&p_queue, philosopher->id);
			// Wait..
			printf("P: %d waiting - Total: %d\n", philosopher->id + 1, philosopher->waiting_times);
			pthread_cond_wait(&p_cond[philosopher->id], &p_mutex);
			printf("Woke up: %d\n", philosopher->id + 1);
		}
	}

	pthread_mutex_unlock(&p_mutex);
}

void think(Philosopher *philosopher) {
	// Think for a random interval
	int thinking_time = (rand() % 5) + 3;
	printf("P: %d thinking for %d seconds.\n", philosopher->id + 1, thinking_time);
	sleep(thinking_time);	
}

// What a philosopher does
void *behavior_philosopher(void *arg) {
	Philosopher *philosopher = (Philosopher *)arg;
	srand(philosopher->id);
		
	while (1) {
		think(philosopher);
		int drink_type = 1;
		get_barman(philosopher);
		request_drink(barmans[philosopher->barman_id], drink_type);
		drink(philosopher);
	}

	return (void *) 0;
}

// What a barman does
void *behavior_barman(void *arg) {
	Barman *barman = (Barman *)arg;
	while (1) {
		if (barman->status == WORKING && barman->drink != NONE) {
			prepare_drink(barman);
		}
	}

	return (void *) 0;
}

int main() {

	pthread_t *p_threads, *b_threads;
	p_threads = malloc(N_PHILOSOPHERS * sizeof(pthread_t));
	b_threads = malloc(N_BARMANS * sizeof(pthread_t));
	
	int i;

	// Initialize stuff
	p_queue = createQueue();

	for (i = 0; i < N_BARMANS; i++) {
		pthread_cond_init(&b_cond[i], NULL);
		barmans[i] = (Barman *) malloc(sizeof(Barman));
		barmans[i]->id = i;
		barmans[i]->waiting_times = 0;
		barmans[i]->drink = NONE;
		barmans[i]->status = FREE;
		pthread_create(&b_threads[i], NULL, behavior_barman, barmans[i]);
	}

	/*
	for (i = 0; i < N_PHILOSOPHERS; i++) {
		sticks[i] = FREE;
		pthread_cond_init(&cond[i], NULL);
	}
	*/	

	for (i = 0; i < N_PHILOSOPHERS; i++) {
		pthread_cond_init(&p_cond[i], NULL);		
		philosophers[i] = (Philosopher *) malloc(sizeof(Philosopher));
		philosophers[i]->id = i;
		philosophers[i]->waiting_times = 0;
		pthread_create(&p_threads[i], NULL, behavior_philosopher, philosophers[i]);
	}

	// TODO change to key pressed
	while (1) {}

	for (i = 0; i < N_PHILOSOPHERS; i++) {
		pthread_join(p_threads[i], NULL);
		free(philosophers[i]);
	}

	free(p_threads);
	return 0;
}