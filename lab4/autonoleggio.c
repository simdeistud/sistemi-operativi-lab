#include <stdio.h>
#include <string.h>

#define CMD_MAX 20

int main(void)
{

    while(1){
        char command[CMD_MAX];
        printf("Command: ");
        fgets(command, CMD_MAX, stdin);
        if(strncmp(command, "quit", 4) == 0){
            exit(1);
        }
    }
}