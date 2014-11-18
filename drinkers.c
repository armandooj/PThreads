
/*
Extra - Generalization
Drinking N_PHILOSOPHER

M barmans
N N_PHILOSOPHER

They want coctails. Each drink may use many bottles (so barmans have to wait sometimes)
*/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define N_PHILOSOPHERS 5
#define N_BARMANS 3
#define N_BOTTLES 3

typedef struct {
	int id;
	int waiting_times;
	int status;
} Philosopher;

typedef struct {
	int id;
	int waiting_times;
	int status;
	int current_philosopher;
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

// Conditions
pthread_mutex_t p_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t p_cond[N_PHILOSOPHERS];
pthread_cond_t b_cond[N_BARMANS];

void prepare_drink(Barman *barman) {
	sleep(2);
	barman->status = FREE;
	barman->drink = NONE;
	//pthread_cond_signal(&b_cond[barman_id]);
}

// Request a barman or wait for one in a queue
void get_barman(Philosopher *philosopher, int drink) {
	pthread_mutex_lock(&p_mutex);
	int barman_id = NONE;

	while (barman_id == NONE) {		
		int i;
		for (i = 0; i < N_BARMANS; i++) {
			// A barman is available
			if (barmans[i]->status == FREE) {
				barmans[i]->status = WORKING;
				barmans[i]->current_philosopher = i;
				barman_id = i;
				i = N_BARMANS;
			}
		}

		if (barman_id == NONE) {
			// If we get here it means no barman is available - Join the waiting queue
			philosopher->waiting_times++;
			printf("P: %d waiting - Total: %d\n", philosopher->id + 1, philosopher->waiting_times);
			// TODO Join queue
			pthread_cond_wait(&p_cond[philosopher->id], &p_mutex);
		} else {
			// Found a barman, request a drink and wait for it
			barmans[barman_id]->drink = drink;
			pthread_cond_wait(&b_cond[barman_id], &p_mutex);
		}
	}

	pthread_mutex_unlock(&p_mutex);
}

void think(Philosopher *philosopher) {
	// Think for a random interval
	int thinking_time = (rand() % 5) + 3;
	sleep(thinking_time);
	printf("P: %d thinking for %d seconds.\n", philosopher->id + 1, thinking_time);
}

// What a philosopher does
void *behavior_philosopher(void *arg) {
	Philosopher *philosopher = (Philosopher *)arg;
	srand(philosopher->id);
		
	while (1) {
		//think(philosopher);
		int drink = 1;
		get_barman(philosopher, drink);
		// request_drink(philosopher, drink);
	}

	return (void *) 0;
}

// What a barman does
void *behavior_barman(void *arg) {
	Barman *barman = (Barman *)arg;
	while (1) {
		if (barman->status == WORKING && barman->drink != NONE) {
			//prepare_drink(barman);
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