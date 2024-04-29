#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	// contollo se il numero degli agomenti è corretto
	if (argc != 3) {
		printf("Wrong number of arguments\n");
		return 1;
	}

	// controllo se il path specificato corrisponde ad un file
	FILE* f;
	if ((f = fopen(argv[1])) == NULL) {
		perror("Unable to open file");
		return 1;
	}

	// controllo se il numero di processi da eseguire sia valido
	int n_conc = atoi(argv[2]);
	if (!n_conc) {
		printf("Invalid number of processes\n");
		return 1;
	}

	// controllo se il comando contiene il carattere %
	if ((strstr(argv[3], "%")) == NULL) {
		printf("Command does not contain the % parameter\n");
		return 1;
	}

	// eseguo il progamma bla bla bla


	return 0;
}