#define _BSD_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "main_tests.h"

void load_inodes(char arr[], int inode_limit)
{
    // Open inodes_list file.
    FILE * inodes_list = fopen("inodes_list", "r");

    size_t len = 0;
    char *line = NULL;
    int num_inodes = 0;
    size_t i;

    while(getline(&line, &len, inodes_list) != -1)
    {
        // Duplicate line so that it isn't mutated by strsep().
        char *inode_line = line;
        int inode_index = atoi(strsep(&inode_line, " "));
        char type = *(strsep(&inode_line, " "));
        
        // Validate inode number and type.
        if ((inode_index >= 0 && inode_index < inode_limit) && (type == 'f' || type == 'd'))
        {
            // Load inode info into array at index.
            arr[inode_index] = type;
            num_inodes++;
        } 
        else
        {
            // Report invalid information.
            fprintf(stderr, "Invalid inode number or file type\n");
        }
    }

    // Make sure all non-used inode indices are not containing junk data.
    for (i = num_inodes; i < inode_limit; i++)
    {
        arr[i] = '\0';
    }
    fclose(inodes_list);
    free(line);
}

char *validate_fs(char *str)
{
    // Check if file system is valid and accessible.
    if (chdir(str) == 0)
    {
        return str;
    }
    else
    {
        fprintf(stderr, "File system is not valid and could not be accessed.\n");
        exit(1);
    }
}

char *validate_cwd(char inodes_arr[])
{
    // Check if inode #0 is a directory
    if (inodes_arr[0] == 'd')
    {
        char *str = malloc(sizeof(char) * 2);
        strcpy(str, "0");
        return str;
    }
    else 
    {
        fprintf(stderr, "Invalid inode #0");
        exit(1);
    }
}

void process_exit(char inodes_arr[], int inode_limit)
{
    // Open and clear inodes_list file.
    FILE * inodes_list = fopen("inodes_list", "w");

    char *line = NULL;
    size_t i;

    // Add all active inodes to inodes_list file.
    for (i = 0; i < inode_limit; i++)
    {
        // Check if inode is active.
        if (inodes_arr[i] == 'f' || inodes_arr[i] == 'd')
        {
            // Write inode to file.
            fprintf(inodes_list, "%zd %c\n", i, inodes_arr[i]);
        }
    }

    fclose(inodes_list);
    free(line);
}

void process_ls(char *cwd)
{
    // Open current working directory file to read.
    FILE * directory = fopen(cwd, "r");

    size_t len = 0;
    char *line = NULL;

    // Skip over first two lines in directory file.
    getline(&line, &len, directory);
    getline(&line, &len, directory);

    // Read directory file and print contents.
    while (getline(&line, &len, directory) != -1)
    {
        printf("%s", line);
    }

    fclose(directory);
    free(line);
}

char *get_inode(char *cwd, char *new_dir)
{
    // Get inode number by name.
    FILE * directory = fopen(cwd, "r");
    size_t len = 0;
    char *line = NULL;

    while (getline(&line, &len, directory) != -1)
    {
        if (strstr(line, new_dir) != NULL)
        {
            char *str = line;
            // Extract inode number as a string.
            char *inode = strdup(strsep(&line, " "));
            fclose(directory);
            free(str);
            return inode;
        }
    }

    fclose(directory);
    free(line);
    return NULL;
}

char *process_cd(char *cwd, char *new_dir, char inodes_arr[])
{
    // Get inode by specified directory name.
    char *inode = get_inode(cwd, new_dir);

    // Verify specified directory exists.
    if (inode != NULL)
    {
        // Verify specified directory is a directory.
        if (inodes_arr[atoi(inode)] == 'd')
        {
            free(cwd);
            return inode;
        }
        else
        {
            // Specific error message.
            fprintf(stderr, "Specified directory is not a directory.\n");
            free(inode);
            return cwd;
        }
    }
    else
    {
        // Specific error message.
        fprintf(stderr, "Specified directory does not exist.\n");
        free(inode);
        return cwd;
    }
}

int name_check(char *cwd, char *new_dir)
{
    // Check for conflicting name.
    FILE * directory = fopen(cwd, "r");
    size_t len = 0;
    char *line = NULL;

    while (getline(&line, &len, directory) != -1)
    {
        // Get entry name.

        // REMOVE SPACE.
        char *entry_name = strdup(strchr(line, ' '));
        
        // Check if directory name exists in entry.
        if (strstr(entry_name, new_dir) != NULL)
        {
            // Entry with name already exists.
            free(entry_name);
            free(line);
            fclose(directory);
            return 0;
        }

        free(entry_name);
    }
    
    fclose(directory);
    free(line);
    
    // Entry with name does not exist.
    return 1;
}

void process_mkdir(char inodes_arr[], int inode_limit, char *cwd, char *new_dir)
{
    bool found = false;
    int i = 0;

    // Check for inode with conflicting name.
    if (name_check(cwd, new_dir) == 1)
    {
        // Check for available inode.
        while (!found && i < inode_limit)
        {
            // Check if inode is unused.
            if (inodes_arr[i] != 'd' && inodes_arr[i] != 'f')
            {
                // Create directory.
                inodes_arr[i] = 'd';

                // Convert index to string.
                int len = snprintf(NULL, 0, "%d", i);
                char *new_dir_inode = (char *)malloc(len + 1);
                snprintf(new_dir_inode, len + 1, "%d", i);

                // Create directory file.
                FILE * directory = fopen(new_dir_inode, "w");
                free(new_dir_inode);
                fprintf(directory, "%d .\n", i);
                fprintf(directory, "%s ..\n", cwd);
                fclose(directory);

                // Update current working directory.
                FILE * file = fopen(cwd, "a");
                fprintf(file, "%d %s\n", i, new_dir);
                fclose(file);

                found = true;
            }
            i++;
        }
        if (found == false)
        {
            fprintf(stderr, "Not enough available inodes for operation.\n");
        }
    }
    else
    {
        // Specific error message.
        fprintf(stderr, "Entry with specified name already exists.\n");
    }
}

void process_touch(char inodes_arr[], int inode_limit, char *cwd, char *new_file)
{
    bool found = false;
    int i = 0;

    // Check for inode with conflicting name.
    if (name_check(cwd, new_file) == 1)
    {
        // Check for available inode.
        while (!found && i < inode_limit)
        {
            // Check if inode is unused.
            if (inodes_arr[i] != 'd' && inodes_arr[i] != 'f')
            {
                // Create file.
                inodes_arr[i] = 'f';

                // Convert index to string.
                int len = snprintf(NULL, 0, "%d", i);
                char *new_file_inode = (char *)malloc(len + 1);
                snprintf(new_file_inode, len + 1, "%d", i);

                // Create file.
                FILE * file = fopen(new_file_inode, "w");
                free(new_file_inode);
                fprintf(file, "%s\n", new_file);
                fclose(file);

                // Update current working directory.
                FILE * directory = fopen(cwd, "a");
                fprintf(directory, "%d %s\n", i, new_file);
                fclose(directory);

                found = true;
            }
            i++;
        }
        if (found == false)
        {
            fprintf(stderr, "Not enough available inodes for operation.\n");
        }
    }
    else
    {
        // Specific error message.
        fprintf(stderr, "Entry with specified name already exists.\n");
    }
}

void process_rm(char inodes_arr[], int inode_limit, char *cwd, char *file_name)
{
    // Remove named entry from current working directory.

    bool found = false;

    FILE * directory = fopen(cwd, "r");
    FILE * directory_new = fopen("copy", "w");
    size_t len = 0;
    char *line = NULL;

    // Search for specified entry by name.
    while (!found && getline(&line, &len, directory) != -1)
    {
        // Get entry name.
        char *entry_name = strdup(strchr(line, ' ') + 1);

        if (strstr(entry_name, file_name) != NULL)
        {
            char *str = get_inode(cwd, file_name);
            int inode = atoi(str);
            if (inodes_arr[inode] == 'f' || inodes_arr[inode] == 'd')
            {
                // Remove inode from inode_arr.
                inodes_arr[inode] = '\0';

                // Remove entry file.
                char *inode_str = get_inode(cwd, file_name);
                remove(inode_str);
                free(inode_str);
            }
            free(str);
            found = true;
        }
        else
        {
            // Copy line to new directory file.
            fprintf(directory_new, "%s", line);
        }

        free(entry_name);
    }
    free(line);
    fclose(directory);
    remove(cwd);
    rename("copy", cwd);
    fclose(directory_new);
}

char *get_command()
{
    // Get user input.
    size_t len = 0;
    char *user_input = NULL;
    getline(&user_input, &len, stdin);
    return user_input;
}

char *get_field(char *command)
{
    // Duplicate original string so it isn't mutated.
    char *cmd = command;
    
    // Get command field.
    char *str = strchr(cmd, ' ');

    // Remove whitespace.
    str++;

    // Remove null character.
    str[strlen(str) - 1] = '\0';

    // Dynamically allocate memory for field.
    char *field = strdup(str);

    return field;
}

void process_ui(char inodes_arr[], char* cur_dir, int inode_limit)
{
    bool run = true;
    char *cwd = cur_dir;

    while (run)
    {
	printf(">>> ");
	char *command = get_command();

        if (strstr(command, "exit") != NULL)
        {
            // Update inodes_list and exit program.
            process_exit(inodes_arr, inode_limit);
            run = false;
        }
        else if (strstr(command, "ls") != NULL)
        {
            // List names and inodes of the current working directory.
            process_ls(cwd);
            
        }
        else if (strstr(command, "cd") != NULL)
        {
            // Get specified directory.
            char *new_dir = get_field(command);
            
            // Change to specified directory.
            cwd = process_cd(cwd, new_dir, inodes_arr);

            free(new_dir);
        }
        else if (strstr(command, "mkdir") != NULL)
        {
            // Get specified directory name.
            char *new_dir = get_field(command);

            process_mkdir(inodes_arr, inode_limit, cwd, new_dir);

            free(new_dir);
        }
        else if (strstr(command, "touch") != NULL)
        {
            // Get specified file name.
            char *new_file = get_field(command);

            process_touch(inodes_arr, inode_limit, cwd, new_file);

            free(new_file);
        }
        else if (strstr(command, "rm") != NULL)
        {
            // Get specified entry name.
            char *file_name = get_field(command);

            process_rm(inodes_arr, inode_limit, cwd, file_name);
            
            free(file_name);
        }
        free(command);
        printf("\n");
    }
    free(cwd);
}

int main(int argc, char *argv[])
{
    if (argc == 3)
    {
        char *file_system = validate_fs(argv[2]);
        int inode_limit = atoi(argv[1]);

        // Move into specified fs directory.
        chdir(file_system);

        char inodes_arr[inode_limit];
        load_inodes(inodes_arr, inode_limit);

        // test_array_loading(inodes_arr);

        // Validate inode 0 and set CWD.
        char *cwd = validate_cwd(inodes_arr);

        process_ui(inodes_arr, cwd, inode_limit);
    }
    else
    {
        fprintf(stderr, "Invalid number of arguments\n");
        exit(1);
    }
}
