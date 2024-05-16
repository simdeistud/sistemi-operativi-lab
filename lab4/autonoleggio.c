#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define INPUT_MAX 20
#define CAR_NAME_MAX 10
#define SEMAPHORE_PREFIX "/autonoleggio_"
#define SEMAPHORE_PREFIX_LEN  14

void create_semaphores();
void view_cars_status();
void lock_car(char car_name[]);
void release_car(char car_name[]);

int main(int argc, char* argv[])
{

    if(argc != 0){
        return 1;
    }

    /* controllo se il programma è stato già avviato in precedenza, 
       in caso contrario inizializzo i semafori per ogni vettura    */
    sem_t * sem_starting = sem_open("/autonoleggio_starting", O_EXCL, S_IRUSR | S_IWUSR);
    if(sem_starting == SEM_FAILED){
        sem_starting = sem_open("/autonoleggio_starting", O_CREAT, S_IRUSR | S_IWUSR, 0);
        create_semaphores();
        sem_post(sem_starting);
    }
    /* se il semaforo di inizializzazione esiste mi assicuro che
       l'inizializzazione sia terminata nel caso il processo corrente
       non sia quello che l'ha fatta partire 
                                */
    sem_wait(sem_starting);
    sem_post(sem_starting);

    while(1){
        char input[INPUT_MAX];
        printf("Command: ");
        fgets(input, INPUT_MAX, stdin);
        input[strcspn(input, "\n")] = '\0';
        char * command = strtok(input, " ");
        char * argument = strtok(NULL, " ");
        if(strncmp(command, "quit", 4) == 0 && argument == NULL){
            break;
        } else if(strncmp(command, "view", 4) == 0 && argument == NULL){
            view_cars_status();
        } else if(strncmp(command, "lock", 4) == 0 ){
            if(strlen(argument) > CAR_NAME_MAX){
                printf("Error: car name too long\n");
                continue;
            }
            lock_car(argument);
        } else if(strncmp(command, "release", 7) == 0){
            if(strlen(argument) > CAR_NAME_MAX){
                printf("Error: car name too long\n");
                continue;
            }
            release_car(argument);
        } else printf("Unknown Command\n");
    }

    exit(0);
}

void create_semaphores()
{
    FILE * catalog;
    catalog = fopen("catalog.txt", "r");
    if(catalog == NULL){
        perror("Impossibile aprire catalog.txt");
        exit(1);
    }
    char car_name[CAR_NAME_MAX];
    char semaphore_name[CAR_NAME_MAX + SEMAPHORE_PREFIX_LEN + 1];
    while(fscanf(catalog, "%s", car_name) != EOF){
        strcpy(semaphore_name, SEMAPHORE_PREFIX);
        strcat(semaphore_name, car_name);
        if(sem_open(semaphore_name, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1) == SEM_FAILED){
            printf("Errore: Due vetture hanno lo stesso nome\n");
            exit(1);
        }
    }
    fclose(catalog);
}

void view_cars_status()
{
    FILE * catalog;
    catalog = fopen("catalog.txt", "r");
    if(catalog == NULL){
        perror("Impossibile aprire catalog.txt");
        exit(1);
    }
    char car_name[CAR_NAME_MAX];
    char semaphore_name[CAR_NAME_MAX + SEMAPHORE_PREFIX_LEN + 1];
    while(fscanf(catalog, "%s", car_name) != EOF){
        strcpy(semaphore_name, SEMAPHORE_PREFIX);
        strcat(semaphore_name, car_name);
        if(sem_open(semaphore_name, O_EXCL, S_IRUSR | S_IWUSR, 0) == SEM_FAILED){
            printf("Errore: una vettura non è stata propriamente sincronizzata\n");
            exit(1);
        }
        unsigned int status;
        sem_getvalue(sem_open(semaphore_name, O_CREAT, S_IRUSR | S_IWUSR, 0), &status);
        printf("Car: ");
        printf(car_name);
        printf(", status: ");
        printf(status == 1 ? "free" : "locked");
        printf("\n");
    }
    fclose(catalog);
}

void lock_car(char car_name[])
{
    char semaphore_name[CAR_NAME_MAX + SEMAPHORE_PREFIX_LEN + 1];
    strcpy(semaphore_name, SEMAPHORE_PREFIX);
    strcat(semaphore_name, car_name);
    if(sem_open(semaphore_name, O_EXCL, S_IRUSR | S_IWUSR) == SEM_FAILED){
        printf("Cannot find car ");
        printf(car_name);
        printf("\n");
    } else{
        sem_t * car_to_lock = sem_open(semaphore_name, O_CREAT, S_IRUSR | S_IWUSR, 0);
        unsigned int status;
        sem_getvalue(sem_open(semaphore_name, O_CREAT, S_IRUSR | S_IWUSR, 0), &status);
        if(status == 0){
            printf("Error. Car: ");
            printf(car_name);
            printf(" already locked\n");
        }
        if(status == 1){
            sem_wait(car_to_lock);
            printf("Car: ");
            printf(car_name);
            printf(" is now locked\n");
        }
    }
}

void release_car(char car_name[])
{
    char semaphore_name[CAR_NAME_MAX + SEMAPHORE_PREFIX_LEN + 1];
    strcpy(semaphore_name, SEMAPHORE_PREFIX);
    strcat(semaphore_name, car_name);
    if(sem_open(semaphore_name, O_EXCL, S_IRUSR | S_IWUSR) == SEM_FAILED){
        printf("Cannot find car ");
        printf(car_name);
        printf("\n");
    } else{
        sem_t * car_to_release = sem_open(semaphore_name, O_CREAT, S_IRUSR | S_IWUSR, 0);
        unsigned int status;
        sem_getvalue(sem_open(semaphore_name, O_CREAT, S_IRUSR | S_IWUSR, 0), &status);
        if(status == 0){
            sem_post(car_to_release);
            printf("Car: ");
            printf(car_name);
            printf(" is now free\n");
        }
        if(status == 1){
            printf("Error. Car: ");
            printf(car_name);
            printf(" already free\n");
        }
    }
    
}