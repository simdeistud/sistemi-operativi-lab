#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


#define MAXPROCESSES 16
#define MAXQUEUESIZE 16
#define MAXCMD 512
#define MAXARG 256
#define MAXARGS 256


int addArgsToCommand(char* command, char* args);

int main(int argc, char** argv)
{

    // controllo se il numero degli agomenti è corretto
    if (argc != 4) {
        printf("Wrong number of arguments\n");
        return 1;
    }

    // controllo se il path specificato corrisponde ad un file
    FILE* f;
    if ((f = fopen(argv[1], "r")) == NULL) {
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
        printf("Command does not contain the %% parameter\n");
        return 1;
    }

    // creo una coda di comandi per ogni processo e la relativa capienza
    char proc_queues[MAXPROCESSES][MAXQUEUESIZE][MAXCMD];
    int proc_queues_size[MAXPROCESSES] = { 0 };

    // in base al numero di processi specificato dall'utente riempio le code con i processi distribuiti equamente
    char args_line[MAXARG];
    int cmd_num;
    for (cmd_num = 0; fgets(args_line, MAXARG, f) != NULL; cmd_num++) {

        // applico la sostituzione di % con gli argomenti
        char command[MAXCMD];
        strcpy(command, argv[3]);
        if (addArgsToCommand(command, args_line)) {
            printf("Something happened in the args replacement\n");
            free((void*)proc_queues);
            return 1;
        }

        //aggiungo il comando alla coda corretta
        strcpy(proc_queues[cmd_num%n_conc][proc_queues_size[cmd_num%n_conc]++], command);

    }

    if(cmd_num < n_conc){
        printf("There are more processes than available commands!\n");
        free((void*)proc_queues);
        return 1;
    }

    // chiudo il file
    if (fclose(f) != 0) {
        perror("Error closing the file");
        free((void*)proc_queues);
        return 1;
    }

    // eseguo in parallelo le code di comandi
    for (int i = 0; i < n_conc; i++) {
        if (!fork()) {
            for (int j = 0; j < proc_queues_size[i]; j++) {

                // costruisco la stringa contenente il programma e il vettore degli argomenti
                char* prog = strtok(proc_queues[i][j], " ");
                char* args[MAXARGS];
                int c = 0;
                args[c++] = prog;
                while (c < MAXARGS && ((args[c++] = strtok(NULL, " ")) != NULL));

                // eseguo il comando
                if(!fork()){
                    if (execvp(prog, args) == -1) {
                        perror("There was an error executing the command");
                        free((void*)proc_queues);
                        return 1;
                    }
                    free((void*)proc_queues);
                    return 0;
                } else {wait(NULL);}

            }
            break;
        }
    }
    wait(NULL);
    free((void*)proc_queues);
    return 0;
}

int addArgsToCommand(char* command, char* args)
{
    char* wildcard = strstr(command, "%");
    char command_tail[MAXCMD/2];
    strcpy(command_tail, wildcard + 1);
    strcpy(wildcard, args);
    char* newln = strstr(command, "\n");
    strcpy(newln, command_tail);
    return 0;
}
