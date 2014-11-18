#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define N_PHILOSOPHERS 5
#define EATING_TIME 3
#define MAX_WAITING_TURNS 10
#define LEFT_STICK(philosopher_id) philosopher_id
#define RIGHT_STICK(philosopher_id) ((philosopher_id + 1) % N_PHILOSOPHERS)
#define LEFT_PHILOSOPHER(philosopher_id) (philosopher_id == 0 ? N_PHILOSOPHERS - 1 : philosopher_id - 1)
#define RIGHT_PHILOSOPHER(philosopher_id) (philosopher_id == N_PHILOSOPHERS - 1 ? 0 : philosopher_id + 1)

// The amount of sticks is always the same as the N_PHILOSOPHERS
// The left stick corresponds to the philosopher's id
int sticks[N_PHILOSOPHERS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond[N_PHILOSOPHERS];

typedef enum {USED, FREE} stick_status;

typedef struct {
	int id;
	int waiting_times;
} Philosopher;

Philosopher *philosophers[N_PHILOSOPHERS];

void print_status() {
	printf("S1\tS2\tS3\tS4\ts5\n");
	printf("%c\t%c\t%c\t%c\t%c\n", sticks[0] == 0 ? 'U' : 'F', sticks[1] == 0 ? 'U' : 'F', sticks[2] == 0 ? 'U' : 'F', sticks[3] == 0 ? 'U' : 'F', sticks[4] == 0 ? 'U' : 'F');
}

void think(Philosopher *philosopher) {
	// Think for a random interval
	int thinking_time = (rand() % 5) + 3;
	printf("P: %d thinking for %d seconds.\n", philosopher->id + 1, thinking_time);
	sleep(thinking_time);	
}

void take_chopsticks(Philosopher *philosopher) {

	pthread_mutex_lock(&mutex);

	//print_status();

	while (sticks[LEFT_STICK(philosopher->id)] == USED || sticks[RIGHT_STICK(philosopher->id)] == USED) {
		philosopher->waiting_times++;
		printf("P: %d waiting - Total: %d\n", philosopher->id + 1, philosopher->waiting_times);
		pthread_cond_wait(&cond[philosopher->id], &mutex);
	}

	//printf("P: %d taking the chopsticks.\n", philosopher->id + 1);
	sticks[LEFT_STICK(philosopher->id)] = USED;
	sticks[RIGHT_STICK(philosopher->id)] = USED;

	pthread_mutex_unlock(&mutex);
}

void eat(Philosopher *philosopher) {
	sleep(EATING_TIME);
	philosopher->waiting_times = 0;
	printf("P: %d eating for %d seconds.\n", philosopher->id + 1, EATING_TIME);
}

void leave_chopsticks(Philosopher *philosopher) {
	
	pthread_mutex_lock(&mutex);

	sticks[LEFT_STICK(philosopher->id)] = FREE;
	sticks[RIGHT_STICK(philosopher->id)] = FREE;

	pthread_cond_signal(&cond[RIGHT_PHILOSOPHER(philosopher->id)]);
	pthread_cond_signal(&cond[LEFT_PHILOSOPHER(philosopher->id)]);

	pthread_mutex_unlock(&mutex);
}

// What a philosopher does
void *behavior(void *arg) {

	Philosopher *philosopher = (Philosopher *)arg;
	//int *philosopher = (int *)arg;	
	srand(philosopher->id);

	while(1) {
		think(philosopher);
		// Extra check for starvation - This is very unlikely to happen but it could happen so.
		if (philosophers[RIGHT_PHILOSOPHER(philosopher->id)]->waiting_times < MAX_WAITING_TURNS ||
			philosophers[RIGHT_PHILOSOPHER(philosopher->id)]->waiting_times < MAX_WAITING_TURNS) {
			take_chopsticks(philosopher);
			eat(philosopher);
			leave_chopsticks(philosopher);
		}
	}

	return (void *) 0;
}

int main() {

	pthread_t *tids;
	tids = malloc(N_PHILOSOPHERS * sizeof(pthread_t));	
		
	// Initialize stuff
	int i;
	for (i = 0; i < N_PHILOSOPHERS; i++) {
		sticks[i] = FREE;
		pthread_cond_init(&cond[i], NULL);
	}

	for (i = 0; i < N_PHILOSOPHERS; i++) {
		philosophers[i] = (Philosopher *) malloc(sizeof(Philosopher));
		philosophers[i]->id = i;
		philosophers[i]->waiting_times = 0;
		pthread_create(&tids[i], NULL, behavior, philosophers[i]);
	}

	// TODO change to key pressed
	while (1) {}

	for (i = 0; i < N_PHILOSOPHERS; i++) {
		pthread_join(tids[i], NULL);
		free(philosophers[i]);
	}

	free(tids);
	return 0;
}
