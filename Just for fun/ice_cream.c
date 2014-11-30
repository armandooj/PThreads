/*
The Ice Cream problem.
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "queue.h"

#define N_KIDS 10
#define BATCH_SIZE 5
#define SELLER_WAITING_TIME 2
#define DAY_DURATION 10

typedef struct {
	int vanilla;
	int chocolate;
	int strawberry;
} Batch;

pthread_mutex_t seller_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t ice_cream_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t kids_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
Kids
*/
typedef struct {
	int id;
	// Order two ice creams at once
	int is_accompanied;
	int waiting_time;
	int flavor;
	int got_ice_cream;
} Kid;

Kid *children;
pthread_t *kids;
pthread_cond_t kids_cond[N_KIDS];
pthread_cond_t ice_cream_cond;

typedef enum {YES, NO} has_ice_cream;

/*
Seller
*/
typedef struct {
	int status;
	int first_kid_arrival;
	// Kids that are already with the seller
	queue *kids_queue;
} Seller;

typedef enum {UNAVAILABLE, AVAILABLE} statuses;
Seller *seller;

pthread_t seller_t;
pthread_cond_t seller_cond;

/*
Sleeps if there's no kids.
*/
void *behavior_seller(void *arg) {

	while (1) {

		pthread_mutex_lock(&seller_mutex);

		// Wait for the batch to be complete
		// TODO or time expired - who sends the signal?
		// (time(NULL) - seller->first_kid_arrival) >= SELLER_WAITING_TIME
		while (seller->kids_queue->count != BATCH_SIZE) {
			
			pthread_cond_wait(&seller_cond, &seller_mutex);

			// Keep track of the first kid's arrival
			if (seller->kids_queue->count == 1) {
				printf("First kid got here. Save the time\n");
				seller->first_kid_arrival = time(NULL);
			}
		}

		seller->status = UNAVAILABLE;
		printf("Hi, I'm the seller!\n");

		// Prepare the ice-creams
		sleep(3);
		int count = seller->kids_queue->count;
		for (int i = 0; i < count; i++) {
			int kid_id = dequeue(seller->kids_queue);
			children[kid_id].got_ice_cream = YES;		
		}

		// Notify all the kids
		pthread_cond_broadcast(&ice_cream_cond);

		seller->status = AVAILABLE;

		pthread_mutex_unlock(&seller_mutex);
	}

	return (void *)0;
}

/*
*/
void *behavior_kids(void *arg) {

	while (1) {

		// Make the kids arrive randomly
		sleep((rand() % 3) + 1);
		
		Kid *kid = (Kid *)arg;
		printf("Hi, I'm kid %d.\n", kid->id);

		// Chose a flavor!
		kid->flavor = (rand() % 3) + 1;

		/*
		* Wait for the seller
		*/
		pthread_mutex_lock(&kids_mutex);

		// Wait till the seller is available
		while (seller->status != AVAILABLE) {
			pthread_cond_wait(&kids_cond[kid->id], &kids_mutex);
		}

		printf("Kid %d is now in the batch.\n", kid->id);	
		enqueue(seller->kids_queue, kid->id);
		
		// Let the seller know the batch has grown
		pthread_cond_signal(&seller_cond);

		/*
		* Wait for the ice-cream
		*/		
		while (kid->got_ice_cream == NO) {
			// Wait for the ice cream to be ready
			pthread_cond_wait(&ice_cream_cond, &kids_mutex);		
		}

		printf("Kid %d got the ice-cream.\n", kid->id);
		kid->got_ice_cream = NO;

		pthread_mutex_unlock(&kids_mutex);

		// Wait until the next day
		sleep(10);
	}

	return (void *)0;
}

int main() {

	srand(time(NULL));

	// Create a seller
	seller = malloc(sizeof(Seller));
	seller->status = AVAILABLE;
	seller->kids_queue = malloc(sizeof(queue));
	init_queue(seller->kids_queue);

	pthread_cond_init(&seller_cond, NULL);
	pthread_cond_init(&ice_cream_cond, NULL);

	seller_t = malloc(sizeof(pthread_t));
	pthread_create(&seller_t, NULL, behavior_seller, NULL);

	// Create N kids
	kids = malloc(N_KIDS * sizeof(pthread_t));
	children = malloc(N_KIDS * sizeof(Kid));

	for (int i = 0; i < N_KIDS; i++) {
		//Kid *kid = malloc(sizeof(Kid));
		children[i].id = i;
		children[i].got_ice_cream = NO;
		pthread_create(&kids[i], NULL, behavior_kids, &children[i]);
		pthread_cond_init(&kids_cond[i], NULL);
	}

	while (1) {}

	return 0;
}
