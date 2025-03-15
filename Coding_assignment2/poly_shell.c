#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <limits.h>

#define RED_TEXT            "\033[1;31m"
#define GREEN_TEXT          "\033[1;32m"
#define BLUE_TEXT           "\033[1;34m"
#define DEFAULT_TEXT        "\033[0m"


#define INIT_BUF_SIZE       PATH_MAX
#define TOKEN_DELIMITER     " \t\n"

#define FEMTO_SHELL_NUM_SUPPORTED_COMMANDS  2
#define PICO_SHELL_NUM_SUPPORTED_COMMANDS   4

#define FEMTO_SHELL         0
#define PICO_SHELL          1
#define NANO_SHELL          2

const char* commands_str [] = {"exit", "echo","pwd", "cd"};
extern char **environ;

static void print_prompt()
{
    char curr_path[PATH_MAX] = {0};
    char* ptr_home_loc = NULL;
    char *user = getenv("USER");
    char *home = getenv("HOME");
    char *pwd = NULL;
    if ((pwd = get_current_dir_name()) == NULL)
    {
        printf("Error in usage of get_current_dir_name()\n");
        exit(-1);
    }

    if ((ptr_home_loc = strstr(pwd,home)) != NULL)
    {
        curr_path[0] = '~';
        strcpy(&curr_path[1], &pwd[strlen(home)]);
    }
    else
    {
        strcpy(curr_path, pwd);
    }
    printf(GREEN_TEXT"%s"DEFAULT_TEXT":"BLUE_TEXT"%s"DEFAULT_TEXT"$ ",user, curr_path);
}

/* IMPORTANT! THIS FUNCTION RETURNS A DYNAMICALLY ALLOCATED BUFFER THAT REQUIRED TO BE FREED FROM THE CALLER */
static char * dynamic_fgets(FILE *stream) 
{
    char *buffer;
    char *current_pos;
    char *new_buffer = NULL;
    size_t size = INIT_BUF_SIZE;
    size_t remaining_size = size;
    size_t len;

    buffer = (char *)malloc(size * sizeof(char));
    if (buffer == NULL)
    {
        printf("Failed to allocate memory: dynamic_fgets\n");
        return NULL;
    } 
    
    current_pos = buffer;
    while (fgets(current_pos, remaining_size, stream) != NULL) 
    {
        len = strlen(buffer);
        
        /* If the last character is a newline, return immediately */ 
        if ((len > 0) && (buffer[len - 1] == '\n')) 
        {
            buffer[len - 1] = '\0';  // replace \n by null terminator
            return buffer;
        }

        size += INIT_BUF_SIZE;
        if ((new_buffer= realloc(buffer, size))== NULL)
        { 
            free(buffer);
            return NULL;
        }

        /* update next position */
        current_pos = new_buffer + len;
        remaining_size = size - len;
        buffer = new_buffer;
    }

    /* If fgets failed without reading anything, free buffer and return NULL*/
    if (current_pos == buffer) 
    {
        free(buffer);
        return NULL;
    }

    return buffer;
}

/* IMPORTANT! THIS FUNCTION RETURNS A DYNAMICALLY ALLOCATED 2D BUFFER THAT REQUIRED TO BE FREED FROM THE CALLER */
static char** tokenize(char *buf) 
{
    int token_count = 0;
    char *token;
    char **argv = NULL;
    char **temp = NULL;
    int status =  EXIT_FAILURE;

    token = strtok(buf, TOKEN_DELIMITER);
    while (token != NULL) 
    {
        if ((temp = (char**)realloc(argv, (token_count + 1) * sizeof(char*))) == NULL)
        {
            printf("Failed to allocate memory for tokens array\n");
            goto func_error;
        }
        argv = temp;

        if ((argv[token_count] = (char*)malloc(strlen(token) + 1)) == NULL)
        {
            printf("Failed to allocate memory for token\n");
            goto func_error;
        }
        strcpy(argv[token_count], token);

        token_count++;
        token = strtok(NULL, TOKEN_DELIMITER);
    }

    /* NULL Pointer for the last parameters */
    if((temp = (char**)realloc(argv, (token_count + 1) * sizeof(char*))) == NULL)
    {
        printf("Failed to allocate memory for NULL\n");
        goto func_error;
    }
    argv = temp;
    argv[token_count] = NULL;
    status =  EXIT_SUCCESS;
#if 0
    printf("Tokens:\n");
    for (int i = 0; argv[i] != NULL; ++i) 
    {
        printf("[%d]: %s\n", i, argv[i]);
    }
#endif

func_error:
    if (status == EXIT_FAILURE)
    {
        for (int i = 0; i < token_count; ++i) 
        {
            free(argv[i]);
        }
        free(argv);
        exit(EXIT_FAILURE);
    }
    else
    {
        return argv; // Return the properly constructed array
    }
}

static char** wait_parse_inputs()
{
    char* local_contiguous_argv = NULL;
    char** local_argv = NULL;

    local_contiguous_argv = dynamic_fgets(stdin);
    if (local_contiguous_argv == NULL)
    {
        printf("Unexpected failure in fgets\n");
        exit(-1);
    }
    
    local_argv = tokenize(local_contiguous_argv);
    /* Free the intermediate state of argv*/
    free(local_contiguous_argv);
    if (local_argv == NULL)
    {
        printf("Unexpected failure in tokenize\n");
        exit(-1);
    }

    return local_argv;
}

static void child_shell_handler(char **newargv)
{
    
    execvp(newargv[0], newargv);
    printf("Exec failed, kernel is not the mode of executing programs\n");
    exit(-1);
}

static int check_flavored_shell(char* command)
{
    int end_index_sprtd_cmds = 0;
#if (shell_flavor == FEMTO_SHELL)
    end_index_sprtd_cmds = FEMTO_SHELL_NUM_SUPPORTED_COMMANDS;
#elif (shell_flavor == PICO_SHELL)
    end_index_sprtd_cmds = PICO_SHELL_NUM_SUPPORTED_COMMANDS;
#endif /*shell_flavor == femto*/

    for (int i = 0; i < end_index_sprtd_cmds; i++)
    {
        if (strcmp(command, commands_str[i]) == 0)
        {
            return EXIT_SUCCESS;
        }
    }

    return EXIT_FAILURE;
}


static void clean_local_argv(char** local_argv)
{
    for (int i = 0; local_argv[i] != NULL; i++) 
    {
        free(local_argv[i]);
    }
    free(local_argv);
}
static int handle_special_commands(char** local_argv)
{
    int is_special_command = EXIT_FAILURE;
    if (strcmp(local_argv[0], "exit") == 0)
    {
        printf("Good Bye :)\n");
        clean_local_argv(local_argv);
        exit(0);
    }
    else if (strcmp(local_argv[0], "cd") == 0)
    {
        /* TODO: Add support for special characters like - and ~*/
        if(chdir(local_argv[1]) < 0)
        {
            printf("shell: cd: %s: No such file or directory\n", local_argv[1]);
        }
        is_special_command = EXIT_SUCCESS;
    }

    return is_special_command;
}
static void parent_shell_handler()
{
    char** local_argv = NULL;
    pid_t pid; 
    int status;

    print_prompt();
    local_argv = wait_parse_inputs();
    if (local_argv[0] == NULL)
    {
        goto clean_up;
    }
    /* Based on the supported shell check the allowed commands and handle special commands */
    if ((check_flavored_shell(local_argv[0])) == EXIT_FAILURE)
    {
        printf("Invalid command\n");
        goto clean_up;
    }
    if (handle_special_commands(local_argv) == EXIT_SUCCESS)
    {
        goto clean_up;
    }
#if (shell_flavor == NANO_SHELL)
    /* TODO*/
#endif
    pid = fork();
    if (pid > 0)
    {
        /* Wait for the child process to be terminated to clean */
        wait(&status);
#if 0
        printf("PARENT: my pid = %d, my child status = %d\n", getpid(), WEXITSTATUS(status));
#endif
    }
    else if (pid == 0)
    {
        child_shell_handler(local_argv);
    }
    else
    {
        printf("PARENT: failed to fork try again\n");
    }

clean_up:
    clean_local_argv(local_argv);
    
}

int main(int argc, char ** argv)
{
    
    while(1)
    {
        parent_shell_handler();
    }

    exit(EXIT_SUCCESS);
}