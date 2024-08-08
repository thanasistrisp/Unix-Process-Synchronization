#include "general.h"
#include "global_vars.h"
#include "defines.h"
#include "utilities.h"

using namespace std;

void parent_worker(void *args) {
	shared_seg *seg = (shared_seg *) args;
	sem_wait(&seg->parent);
	while (1) {
		auto start = clock();
		sem_wait(&seg->id_sem);
		int idt = seg->id;
		sem_post(&seg->id_sem);

		/////////////////////////////////////
		// critical section
		get_char_array(file, lin_per_seq, idt, seg->data);
		/////////////////////////////////////
		
		log_file_parent << "Sequence " << idt << " remained " << ((float)clock()-start) / CLOCKS_PER_SEC << " seconds in shmem" << endl;
		sem_post(&seg->child); // update the child that sequence is ready for reading
		sem_wait(&seg->parent);
		sem_wait(&seg->child_finished_sem);
		if (seg->child_finished == N) {
			sem_post(&seg->child_finished_sem);
			break;
		}
		else if (seg->finish) {
			seg->finish = false;
			sem_wait(&seg->parent);
		}
		sem_post(&seg->child_finished_sem);
	}
}
