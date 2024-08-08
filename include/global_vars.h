#include "general.h"

extern std::ofstream log_file_child;
extern std::ofstream log_file_parent;
extern int num_seq;
extern int lin_per_seq;
extern int line, seq;
extern int N;
extern char *file;

typedef struct {
	char **data;
	int id;
	sem_t *mutex;
	int *user_counter;
	sem_t rw_mutex;
	sem_t parent;
	sem_t child;
	sem_t id_sem;
	sem_t child_finished_sem;
	int child_finished;
	bool finish;
} shared_seg;
