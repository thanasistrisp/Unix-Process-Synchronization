#include "general.h"
#include "global_vars.h"
#include "defines.h"

using namespace std;

void child_worker(void *args) {
	shared_seg *seg = (shared_seg *) args;
	auto submit = clock();
	sem_wait(&seg->mutex[seq]);
	seg->user_counter[seq]++;
	if (seg->user_counter[seq] == 1) {
		sem_wait(&seg->rw_mutex);
		sem_wait(&seg->id_sem);
		if (seg->id != seq) {
			seg->id = seq;
			sem_post(&seg->id_sem);
			sem_post(&seg->parent); // wake up parent
			sem_wait(&seg->child); // wait for response
		}
		else {
			sem_post(&seg->id_sem);
		}
	}
	sem_post(&seg->mutex[seq]);

	///////////////////////////////////////
	// critical section

	log_file_child << "Request " << " for <x,y> = <" << seq << "," << line << ">" << endl;
	log_file_child << "Line: " << seg->data[line] << endl;
	log_file_child << "Submit time: " << ((float)clock()-submit) / CLOCKS_PER_SEC << " seconds" << endl;
	usleep(RESPONSE_TIME * 1000);

	///////////////////////////////////////

	sem_wait(&seg->mutex[seq]);
	seg->user_counter[seq]--;
	if (seg->user_counter[seq] == 0) {
		sem_post(&seg->rw_mutex);
	}
	sem_post(&seg->mutex[seq]);
	double r = (double)rand() / (double)RAND_MAX;
	if (r > PROB_SAME) {
		seq = rand() % num_seq; // request a different sequence
	}
	line = rand() % lin_per_seq;
}
