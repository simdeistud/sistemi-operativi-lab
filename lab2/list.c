#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

#define MAX_DEPTH 4096
#define MAX_PATH_LENGTH 1024

int printFileInfo(char path[]);

int main(int argc, char* argv[])
{

	// checks if number of arguments is correct
	if (argc != 2) {
		printf("Invalid number of arguments.\n");
		return 1;
	}

	// tries to open the folder specified in the second argument
	DIR* starting_dir;
	if ( (starting_dir = opendir(argv[1]) ) == NULL) {
		perror("Error opening starting folder");
		return 1;
	}
	if (closedir(starting_dir) == -1) {
		perror("Error closing starting folder");
		return 1;
	}

	// prints information of the starting folder
	printFileInfo(argv[1]);

	// creates array containing the paths of all the directories to explore
	char dirs_paths[MAX_DEPTH][MAX_PATH_LENGTH];
	for (int c = 0; c < MAX_DEPTH; c++) {
		strcpy(dirs_paths[c], "");
	}
	strcpy(dirs_paths[0], argv[1]);
	

	// explores the folders
	DIR* current_dir_entry;
	for (int c = 0, last_dir = 0; (current_dir_entry = opendir(dirs_paths[c])) != NULL || c <= last_dir; c++) {
		struct dirent *current_entry;

		while ( (current_entry = readdir(current_dir_entry)) != NULL ) {

			// we don't consider the "." and ".." entries
			if ( strcmp(current_entry->d_name, ".") == 0 || strcmp(current_entry->d_name, "..") == 0 ) {
				continue;
			}

			// build the full path of the file
			char full_path[MAX_PATH_LENGTH];
			strcpy(full_path, dirs_paths[c]);
			strcat(full_path, "/");
			strcat(full_path, current_entry->d_name);

			// print the info of the file
			if (printFileInfo(full_path)) {
				printf("Unable to retrieve info about file %s", full_path);
				perror("");
				return 1;
			}

			// if it's a folder, add it to the dirs_paths array
			struct stat dir_stat;
			if (stat(full_path, &dir_stat) == -1) {
				perror("stat failed");
				return 1;
			}
			if (S_ISDIR(dir_stat.st_mode) && ++last_dir < MAX_DEPTH) {
				strcpy(dirs_paths[last_dir], full_path);
			}
		
		}

		if (closedir(current_dir_entry) == -1) {
			printf("Error closing folder %s", dirs_paths[c]);
			perror("");
			return 1;
		}
	}

}

int printFileInfo(char path[])
{
	// using stat() to find the information of the file
	struct stat file_stat;
	if (stat(path, &file_stat) == -1) {
		return 1;
	}

	// discerning the type of the file
	char type[32];
	switch (file_stat.st_mode & S_IFMT) {
		case S_IFREG:
			strcpy(type, "file");
			break;
		case S_IFDIR:	
			strcpy(type, "directory");
			break;
		case S_IFIFO:
			strcpy(type, "FIFO");
			break;
		case S_IFLNK:
			strcpy(type, "symbolic link");
			break;
		default:
			strcpy(type, "other");
	}

	// obtaining the name of the user
	struct passwd* file_usr = getpwuid(file_stat.st_uid);
	if (file_usr == NULL) {
		return 1;
	}
	
	// obtaining the name of the group
	struct group* file_grp = getgrgid(file_stat.st_gid);
	if (file_grp == NULL) {
		return 1;
	}

	printf("Node: %s\n", path);
	printf("    ");

	printf("Inode: %u\n", file_stat.st_ino);
	printf("    ");

	printf("Type: %s\n", type);
	printf("    ");

	printf("Size: %u\n", file_stat.st_size);
	printf("    ");

	printf("Owner: %u %s\n", file_stat.st_uid, file_usr->pw_name);
	printf("    ");

	printf("Group: %u %s\n", file_stat.st_gid, file_grp->gr_name);
	printf("\n");

	return 0;
}