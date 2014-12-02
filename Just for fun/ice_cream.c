/*
The Ice Cream problem.
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "queue.h"
#include "simclist.h"

#define N_KIDS 10
#define BATCH_SIZE 5
#define SELLER_WAITING_TIME 3
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
	int kid_status;
	int ice_cream_status;
	
	int is_accompanied;
	int waiting_time;
	int flavor;
	int got_ice_cream;
} Kid;

Kid *children;
pthread_t *kids;
pthread_cond_t kids_cond[N_KIDS];
pthread_cond_t ice_cream_cond;

typedef enum {WAITING_FOR_SELLER, WAITING_FOR_ICE_CREAM} kid_status;
typedef enum {READY, NOT_READY} ice_cream_status;

/*
Seller
*/
typedef struct {
	int first_kid_arrival;
	// Kids that are already with the seller
	list_t batch_list;
} Seller;

Seller *seller;

pthread_t seller_t;
pthread_cond_t seller_cond;

list_t kids_list;

/*
Sleeps if there's no kids.
*/
void *behavior_seller(void *arg) {

	while (1) {

		pthread_mutex_lock(&kids_mutex);

		// Wait for the batch to be complete
		// TODO or time expired - who sends the signal?
		// (time(NULL) - seller->first_kid_arrival) >= SELLER_WAITING_TIME
		while (list_size(&kids_list) < BATCH_SIZE) {
			
			pthread_cond_wait(&seller_cond, &seller_mutex);	
			/*		
			// Keep track of the first kid's arrival
			if (list_size(&kids_list) == 1) {
				printf("First kid got here. Save the time\n");
				seller->first_kid_arrival = time(NULL);
			} */
		}

		printf("Hi, I'm the seller!\n");

		// Choose 5 kids among the list
		int count = list_size(&kids_list);
		for (int i = 0; i < count; i++) {
			if (list_size(&seller->batch_list) < 5) {
				// TODO Nice algorithm
				Kid *kid = list_extract_at(&kids_list, i);
				printf(".. %d\n", kid->id);
				list_append(&seller->batch_list, kid);
				//list_extract_at(&kids_list, i);
			} else {
				// Break the loop
				i = count;
			}
		}


		// Prepare the ice-creams
		sleep(3);

		// Tell the kids their ice-cream is ready
		int batch_count = list_size(&seller->batch_list);
		for (int i = 0; i < batch_count; i++) {
			Kid *kid = list_extract_at(&seller->batch_list, i);
			kid->ice_cream_status = READY;
			printf("%d\n", kid->ice_cream_status);
			//pthread_cond_signal(&kids_cond[kid->id]);
			//list_extract_at(&seller->batch_list, i);
		}

		pthread_mutex_unlock(&kids_mutex);

		/*
		int count = seller->kids_queue->count;
		for (int i = 0; i < count; i++) {
			int kid_id = dequeue(seller->kids_queue);
			children[kid_id].got_ice_cream = YES;		
		}


		// Notify all the kids
		pthread_cond_broadcast(&ice_cream_cond);

		seller->status = AVAILABLE;
		*/

		// pthread_mutex_unlock(&seller_mutex);
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
		// Chose a flavor!
		kid->flavor = (rand() % 3) + 1;

		// Add to the "list"
		list_append(&kids_list, &kid);
		printf("Hi, I'm kid %d.\n", kid->id);

		/*
		* Wait for the ice-cream
		*/
		pthread_mutex_lock(&kids_mutex);

		//pthread_cond_signal(&seller_cond);

		while (kid->ice_cream_status != READY) {
			pthread_cond_wait(&kids_cond[kid->id], &kids_mutex);
		}

		printf("Kid %d got the ice-cream.\n", kid->id);
		
		kid->ice_cream_status = NOT_READY;
		pthread_mutex_unlock(&kids_mutex);

		// Wait until the next day
		sleep(10);
	}

	return (void *)0;
}

int main() {

	srand(time(NULL));

	// List of kids
	list_init(&kids_list);

	// Create a seller
	seller = malloc(sizeof(Seller));
	// Batch
	list_init(&seller->batch_list);

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
		children[i].ice_cream_status = NOT_READY;
		pthread_create(&kids[i], NULL, behavior_kids, &children[i]);
		pthread_cond_init(&kids_cond[i], NULL);
	}

	while (1) {
		sleep(3);
		pthread_cond_signal(&seller_cond);
	}

	return 0;
}
