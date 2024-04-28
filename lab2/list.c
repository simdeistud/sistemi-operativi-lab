#include <dirent.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>

typedef struct {
    char relative_path[512];
    ino_t inode_number;
    mode_t type;
    char type_name[32];
    off_t size;
    uid_t owner;
    char owner_name[128];
    gid_t group;
    char group_name[128];
} entry_info;

void print_entry_info(entry_info* info);

entry_info get_entry_info(char entry_path[], char relative_base_path[], char entry_name[]);

void put_type_name(char type_name[], unsigned short type);

void put_owner_name(char owner_name[], short uid);

void put_group_name(char group_name[], short gid);

void list(DIR *dir, char relative_path[]);

int main(int argc, char *argv[]) {

    // controllare se gli argomenti sono giusti
    if (argc != 2) {
        printf("Wrong number of arguments!\n");
        return 1;
    }

    // controllare se argv[1] riferisce una cartella

    struct stat dir_stat;
    int has_failed = stat(argv[1], &dir_stat);
    if(has_failed == -1){
                perror("stat fallita");
                exit(EXIT_FAILURE);
            }

    if (!S_ISDIR(dir_stat.st_mode)) {
        printf("Path does not refer to a folder!\n");
        return 1;
    }

    // stampa le info della cartella iniziale

    entry_info root_info = get_entry_info(argv[1], "", argv[1]);
    print_entry_info(&root_info);

    // apre la cartella iniziale e chiama list()

    DIR *starting_dir = opendir(argv[1]);

    char relative_path[256];
    sprintf(relative_path, "%s/", argv[1]);
    list(starting_dir, relative_path);
    closedir(starting_dir);

    return 0;

}

void list(DIR *dir, char relative_path[]) {

    DIR *subdirs[256];
    char *subdirs_name[256];
    int subdirs_num = 0;

    struct dirent *dir_entry;
    while ((dir_entry = readdir(dir)) != NULL) {
        if (strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0) {
            char entry_path[512];
            strcpy(entry_path, relative_path);
            strcat(entry_path, dir_entry->d_name);
            entry_info info = get_entry_info(entry_path, relative_path, dir_entry->d_name);
            print_entry_info(&info);

            struct stat entry_stat;
            int has_failed = stat(entry_path, &entry_stat);
            if(has_failed == -1){
                perror("stat fallita");
                
                exit(EXIT_FAILURE);
            }
            if (S_ISDIR(entry_stat.st_mode)) {
                subdirs[subdirs_num] = opendir(entry_path);
                subdirs_name[subdirs_num] = dir_entry->d_name;
                subdirs_num++;
            }
        }

    }

    while (--subdirs_num >= 0) {
        char new_relative_path[256];
        strcpy(new_relative_path, relative_path);
        strcat(new_relative_path, subdirs_name[subdirs_num]);
        strcat(new_relative_path, "/");
        list(subdirs[subdirs_num], new_relative_path);
        closedir(subdirs[subdirs_num]);
    }
}

void print_entry_info(entry_info* info) {
    printf("Node: %s\n", info->relative_path);
    printf("    ");
    printf("Inode: %ju\n", (uintmax_t) info->inode_number);
    printf("    ");
    printf("Type: %s\n", info->type_name);
    printf("    ");
    printf("Size: %jd\n", (uintmax_t) info->size);
    printf("    ");
    printf("Owner: %jd %s\n", (uintmax_t) info->owner, info->owner_name);
    printf("    ");
    printf("Group: %jd %s\n", (uintmax_t) info->group, info->group_name);
}

entry_info get_entry_info(char entry_path[], char relative_base_path[], char entry_name[]) {

    struct stat entry_stat; 
    int has_failed = stat(entry_path, &entry_stat); // la stat non viene aperta correttamente
    if(has_failed == -1){
                perror("stat fallita");
                exit(EXIT_FAILURE);
            }

    entry_info info;
    strcpy(info.relative_path, entry_path);
    info.inode_number = entry_stat.st_ino;
    info.type = entry_stat.st_mode;
    put_type_name(info.type_name, info.type);
    info.size = entry_stat.st_size;
    info.owner = entry_stat.st_uid;
    put_owner_name(info.owner_name, info.owner);
    info.group = entry_stat.st_gid;
    put_group_name(info.group_name, info.group);

    return info;
}

void put_owner_name(char owner_name[], short uid) {
    struct passwd *entry_passwd = getpwuid(uid);
    strcpy(owner_name, entry_passwd->pw_name);
}

void put_group_name(char group_name[], short gid) {
    struct group *entry_group = getgrgid(gid);
    strcpy(group_name, entry_group->gr_name);
}

void put_type_name(char type_name[], unsigned short type) {
    switch (type & S_IFMT) {
        case S_IFDIR:
            sprintf(type_name, "directory");
            break;
        case S_IFIFO:
            sprintf(type_name, "FIFO");
            break;
        case S_IFLNK:
            sprintf(type_name, "symbolic link");
            break;
        case S_IFREG:
            sprintf(type_name, "file");
            break;
        default:
            sprintf(type_name, "other");
            break;
    }
}

