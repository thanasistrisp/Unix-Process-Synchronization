#include "general.h"

using namespace std;

int count_lines(char *f) {
	ifstream file(f);
	if (file.is_open()) {
		int count = 0;
		string line;
		while (getline(file, line)) {
			count++;
		}
		return count;
	}
	perror("File does not exist");
	exit(1);
}


void get_char_array(char *file, int lines, int seq, char **arr) {
	// from sequence seq to sequence seq + lines
	ifstream f(file);
	if (f.is_open()) {
		string line;
		int count = 0;
		int i = 0;
		while (getline(f, line)) {
			if (count >= seq*lines && count < seq*lines+lines) {
				strcpy(arr[i], line.c_str());
				i++;
			}
			if (count >= seq*lines + lines) {
				break;
			}
			count++;
		}
	}
}


int get_max_line_size(char *f) {
	int max = 0;
	ifstream file(f);
	if (file.is_open()) {
		string line;
		while (getline(file, line)) {
			int len = line.length();
			if (len > max) {
				max = line.length();
			}
		}
		return max;
	}
	perror("File does not exist");
	exit(1);
}
