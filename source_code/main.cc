#include "general.h"
#include "utilities.h"
#include "defines.h"
#include "workers.h"
#include "global_vars.h"

using namespace std;

ofstream log_file_child;
ofstream log_file_parent;
int num_seq;
int lin_per_seq;
int line, seq;
int N;
char *file;

int main(int argc, char** argv) {
	srand(time(NULL));

	//////////////////////////////////////////////////
	// Read input parameters
	int lines;
	int requests;
	if (argc == 5) {
		file = argv[1];
		lines = count_lines(file);
		if (lines <= MIN_LINES) {
			std::cerr << "File is too small" << endl;
			return 1;
		}
		lin_per_seq = atoi(argv[2]);
		N = atoi(argv[3]);
		requests = atoi(argv[4]);
	} else {
		std::cerr << "Usage: ./program <file> <lines per sequence> <child processes> <requests>" << endl;
	}
	//////////////////////////////////////////////////

	std::cerr << "Please wait till Done is printed..." << endl;

	//////////////////////////////////////////////////
	// shell commands to create the log directory and clean previous logs
	string command1 = "rm -rf " + string(LOG_DIR);
	system(command1.c_str());
	string command2 = "mkdir " + string(LOG_DIR);
	system(command2.c_str());
	//////////////////////////////////////////////////

	log_file_parent.open(string(LOG_DIR) + "parent.txt");
	
	num_seq = lines / lin_per_seq;

	int max_char_per_line = get_max_line_size(file);


	//////////////////////////////////////////////////
	// shared memory and semaphores

	int id1, id2, id3, id4;
	shared_seg *seg;
	if ((id1 = shmget(IPC_PRIVATE, sizeof(shared_seg), IPC_CREAT | 0666)) == -1) { perror("shmget"); exit(1); }
	seg = (shared_seg *)shmat(id1, NULL, 0);
	if ((id2 = shmget(IPC_PRIVATE, lin_per_seq * sizeof(char *), IPC_CREAT | 0666)) == -1) { perror("shmget"); exit(1); }
	seg->data = (char **)shmat(id2, NULL, 0);
	int *id = new int[lin_per_seq];
	for (int i = 0; i < lin_per_seq; i++) {
		if ((id[i] = shmget(IPC_PRIVATE, max_char_per_line * sizeof(char), IPC_CREAT | 0666)) == -1) { perror("shmget"); exit(1); }
		seg->data[i] = (char *)shmat(id[i], NULL, 0);
	}
	if ((id3 = shmget(IPC_PRIVATE, sizeof(sem_t) * num_seq, IPC_CREAT | 0666)) == -1) { perror("shmget"); exit(1); }
	seg->mutex = (sem_t *)shmat(id3, NULL, 0);
	if ((id4 = shmget(IPC_PRIVATE, sizeof(int) * num_seq, IPC_CREAT | 0666)) == -1) { perror("shmget"); exit(1); }
	seg->user_counter = (int *)shmat(id4, NULL, 0);
	seg->child_finished = 0;
	for (int i = 0; i < num_seq; i++) {
		if (sem_init(&seg->mutex[i], 1, 1) == -1) { perror("sem_init"); exit(1); }
		seg->user_counter[i] = 0;
	}
	if (sem_init(&seg->rw_mutex, 1, 1) == -1) { perror("sem_init"); exit(1); }
	if (sem_init(&seg->parent, 1, 0) == -1) { perror("sem_init"); exit(1); }
	if (sem_init(&seg->child, 1, 0) == -1) { perror("sem_init"); exit(1); }
	if (sem_init(&seg->id_sem, 1, 1) == -1) { perror("sem_init"); exit(1); }
	if (sem_init(&seg->child_finished_sem, 1, 1) == -1) { perror("sem_init"); exit(1); }
	seg->finish = false;
	//////////////////////////////////////////////////


	//////////////////////////////////////////////////
	// fork child processes
	
	pid_t *pid = new pid_t[N];
	for (int i = 0; i < N; i++) {
		pid[i] = fork();
		if (pid[i] == -1) {
			perror("fork");
		}
		else if (pid[i] == 0) {
			// create file for logging
			log_file_child.open(LOG_DIR + to_string(getpid()) + ".txt");
			srand(time(NULL) ^ (getpid()<<16)); // need to seed with something different for each child
			seq = rand() % num_seq;
			line = rand() % lin_per_seq;
			for (int j = 0; j < requests; j++) {
				child_worker(seg);
			}
			sem_wait(&seg->child_finished_sem);
			seg->child_finished++;
			seg->finish = true;
			sem_post(&seg->child_finished_sem);
			sem_post(&seg->parent);
			delete[] pid;
			delete[] id;
			log_file_child.close();
			exit(EXIT_SUCCESS);
		}

	}
	//////////////////////////////////////////////////
	
	//////////////////////////////////////////////////
	// parent
	parent_worker(seg);

	// wait for child processes to finish
	for (int i = 0; i < N; i++) {
		waitpid(pid[i], NULL, 0);
	}

	log_file_parent.close();

	// detach from the segment
	shmdt(seg->data);
	shmdt(seg->mutex);
	shmdt(seg->user_counter);
	shmdt(seg);
	// destroy the segment
	shmctl(id1, IPC_RMID, NULL);
	shmctl(id2, IPC_RMID, NULL);
	shmctl(id3, IPC_RMID, NULL);
	shmctl(id4, IPC_RMID, NULL);
	for (int i = 0; i < lin_per_seq; i++) {
		shmctl(id[i], IPC_RMID, NULL);
	}
	delete[] id;
	delete[] pid;

	std::cerr << "Done! Log files are in " << LOG_DIR << endl;

	return EXIT_SUCCESS;

	//////////////////////////////////////////////////

}
