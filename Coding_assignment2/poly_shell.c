#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdint.h>

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

extern char **environ;

const char* commands_str [] = {"exit", "echo","pwd", "cd"};

char** g_local_env_vars = NULL;
uint32_t g_local_env_vars_cnt = 0;

static int search_and_get_env_var_index(const char *env_var);


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

static void child_shell_handler(char **newargv, const uint8_t cmd_indx)
{
    
    execvp(newargv[cmd_indx], &newargv[cmd_indx]);
    printf("Exec failed, kernel is not the mode of executing programs\n");
    exit(-1);
}

static int check_flavored_shell(char* command)
{
#if (shell_flavor <= PICO_SHELL) 
    int end_index_sprtd_cmds = 0;
#endif // pico and femto 

#if (shell_flavor == FEMTO_SHELL)
    end_index_sprtd_cmds = FEMTO_SHELL_NUM_SUPPORTED_COMMANDS;
#elif (shell_flavor == PICO_SHELL)
    end_index_sprtd_cmds = PICO_SHELL_NUM_SUPPORTED_COMMANDS;
#endif

#if (shell_flavor <= PICO_SHELL)
for (int i = 0; i < end_index_sprtd_cmds; i++)
{
    if (strcmp(command, commands_str[i]) == 0)
    {
        return EXIT_SUCCESS;
    }
}
#else
    return EXIT_SUCCESS;
#endif /*shell_flavor*/

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

static void clean_local_env_vars(void)
{
    for (int i = 0; i < g_local_env_vars_cnt; i++) 
    {
        free(g_local_env_vars[i]);
    }
    free(g_local_env_vars);
}

static int handle_special_commands(char** local_argv, const uint8_t cmd_indx)
{
    int is_special_command = EXIT_FAILURE;
    if (strcmp(local_argv[cmd_indx], "exit") == 0)
    {
        printf("Good Bye :)\n");
        clean_local_argv(local_argv);
        clean_local_env_vars();
        exit(0);
    }
#if (shell_flavor >= PICO_SHELL)
    else if (strcmp(local_argv[cmd_indx], "cd") == 0)
    {
        /* TODO: Add support for special characters like - and ~*/
        if(chdir(local_argv[cmd_indx + 1]) < 0)
        {
            printf("shell: cd: %s: No such file or directory\n", local_argv[cmd_indx + 1]);
        }
        is_special_command = EXIT_SUCCESS;
    }
#endif //PICO_SHELL
#if (shell_flavor >= NANO_SHELL)
    else if (strcmp(local_argv[cmd_indx], "export") == 0)
    {
        int local_var_index = search_and_get_env_var_index(local_argv[cmd_indx + 1]);
        if (local_var_index >= 0)
        {
            if (putenv(g_local_env_vars[local_var_index]) != 0)
            {
                printf("Error in export built-in command\n");
            }
            
        }
        else
        {
            printf("Error: no local environmental variable called %s\n", local_argv[cmd_indx + 1]);
        }
        is_special_command = EXIT_SUCCESS;
    }
#endif // nano shell
    

    return is_special_command;
}

static void handle_shell_preproc_for_non_special_commands(char** local_argv, const uint8_t cmd_indx)
{
#if (shell_flavor < NANO_SHELL)
    char * env_var_exp;
#endif // pico and femto

    if ((strcmp(local_argv[cmd_indx], "echo") == 0) && 
    (local_argv[cmd_indx + 1] != NULL) && (local_argv[cmd_indx + 1][0] == '$'))
    {
        
#if (shell_flavor < NANO_SHELL)
        /* Expand the passed variable iff $ exists this means that the variable isn't a local variable*/
        printf("%s echo hit  %ld\n",local_argv[cmd_indx + 1], cmd_indx);
        if ((env_var_exp = getenv(&local_argv[cmd_indx + 1][1])) == NULL)
        {
            return;
        }
        free(local_argv[cmd_indx + 1]);
        local_argv[cmd_indx + 1] = (char*) malloc((strlen(env_var_exp) + 1) * sizeof(char));
        strcpy(local_argv[cmd_indx + 1], env_var_exp);
#else
        printf("Error Unhandled case of $ please check\n");
        exit(-1);
#endif  //(shell_flavor < NANO_SHELL)
    }
}
#if (shell_flavor >= NANO_SHELL)
static int search_env_var_exist(const char *env_var) 
{
    int index = -1;
    char *ptr_sub_str = NULL;
    uint32_t offset_to_equal_sign = 0;

    char *equal_sign = strchr(env_var, '=');
    if (equal_sign == NULL) {
        printf("Error:[UNEXPECTED] Invalid environment variable format.\n");
        exit(-1);
    }

    offset_to_equal_sign = (uint32_t)(equal_sign - env_var);
    ptr_sub_str = (char *)malloc(offset_to_equal_sign + 1);
    strncpy(ptr_sub_str, env_var, offset_to_equal_sign);
    ptr_sub_str[offset_to_equal_sign] = '\0';

    // Search in stored environment variables
    for (int i = 0; i < g_local_env_vars_cnt; i++) 
    {
        char *equal_pos = strchr(g_local_env_vars[i], '=');
        size_t name_length = (equal_pos) ? (equal_pos - g_local_env_vars[i]) : strlen(g_local_env_vars[i]);

        // Compare only the variable name part
        if ((strncmp(g_local_env_vars[i], ptr_sub_str, name_length) == 0) && 
            (name_length == offset_to_equal_sign))
        {
            index = i;
            break;
        }
    }

    free(ptr_sub_str);  // Free allocated memory
    return index;
}

static int search_and_get_env_var_index(const char *env_var) 
{
    int index = -1;
    uint32_t str_len = strlen(env_var);
    char * ptr_newly_alloc_str = (char*) malloc( sizeof(char) * (str_len +2));
    strcpy(ptr_newly_alloc_str, env_var);
    ptr_newly_alloc_str[str_len] = '=';
    ptr_newly_alloc_str[str_len + 1] = '\0';

    index = search_env_var_exist(ptr_newly_alloc_str);

    free(ptr_newly_alloc_str);
    return index;
}

static int handle_correct_local_var_format(char** local_argv, uint8_t* cmd_strt_indx)
{
    int continue_exec = EXIT_SUCCESS;
    int arg_index = 0;
    int env_var_index = 0;
    char * local_checker = NULL;
    uint32_t old_base = 0;
    uint32_t already_alloc = 0;
    uint32_t local_env_var_index = 0;
    
    /* Check if there is any command exists in the line or the = sign is alone */
    for (arg_index = 0; local_argv[arg_index] != NULL; arg_index++)
    {
        if (NULL == (local_checker = strchr(local_argv[arg_index], '=')))
        {
            /* if there is a command we shall ignore all local envirnomental variables settings before it */
            *cmd_strt_indx = arg_index; /* Parent code shall start execution from the cmd_strt_indx*/
#if 0
            printf("%s Command found at %ld\n",local_argv[arg_index], arg_index);
#endif
            goto func_exit;
        }
        else if (local_argv[arg_index] == local_checker)
        {
            printf("%s: command not found\n", local_argv[arg_index]);
            continue_exec = EXIT_FAILURE;
            goto func_exit;
        }
    }
    /* means that all arguments are environmental variables */
    old_base = g_local_env_vars_cnt;
    for (arg_index = 0; local_argv[arg_index] != NULL; arg_index++)
    {
        if ((env_var_index = search_env_var_exist(local_argv[arg_index])) >= 0)
        {
            free(g_local_env_vars[env_var_index]);
            g_local_env_vars[env_var_index] = (char*) malloc(sizeof(char) * (strlen(local_argv[arg_index]) + 1));
            strcpy(g_local_env_vars[env_var_index], local_argv[arg_index]); 
            already_alloc++;
        }
        else
        {
            local_env_var_index = old_base + arg_index - already_alloc;
            g_local_env_vars = (char **)realloc(g_local_env_vars, sizeof(char*) * (local_env_var_index + 1));
            g_local_env_vars[local_env_var_index] = (char*) malloc(sizeof(char) * (strlen(local_argv[arg_index]) + 1));
            strcpy(g_local_env_vars[local_env_var_index], local_argv[arg_index]);
            g_local_env_vars_cnt++;
        }
    }

#if 0
    for (int i = 0; i < g_local_env_vars_cnt ; i++)
    {
        printf("%s var\n",g_local_env_vars[i]);
    }
#endif
    continue_exec = EXIT_FAILURE;

func_exit:
    return continue_exec;
}

static void replace_all_local_global_var(char** local_argv, const uint8_t cmd_indx)
{
    int local_var_index;
    char * ptr_var_value = NULL;
    for(int i = cmd_indx; local_argv[i]!= NULL; i++)
    {
        if(local_argv[i][0] == '$')
        {
            if ((ptr_var_value = getenv(&local_argv[cmd_indx + 1][1])) == NULL)
            {
                local_var_index = search_and_get_env_var_index(&local_argv[i][1]);
                if (local_var_index == -1)
                {
                    /* Neither Global Nor local variable exist*/
                    continue;
                }

                if (NULL != (ptr_var_value = strchr(g_local_env_vars[local_var_index], '=')))
                {
                    ptr_var_value++;
                }
            }
            /* if global or local variable exist replace the argument in argv */
            free(local_argv[i]);
            local_argv[i] = (char*)malloc(sizeof(char) * strlen(ptr_var_value));
            strcpy(local_argv[i], ptr_var_value);
            local_argv[i][strlen(ptr_var_value)] = '\0';
        }

    }
}
#endif // nano shell or advance
static void parent_shell_handler()
{
    char** local_argv = NULL;
    pid_t pid; 
    int status;
    uint8_t cmd_indx = 0;

    /* prompt and tokenization */
    print_prompt();
    local_argv = wait_parse_inputs();
    if (local_argv[0] == NULL)
    {
        goto clean_up;
    }
    /* Handle inputs from the shell:
    *      - Check each supported shell commands and functionalities: if not supported print invalid command
    *      - Handle special commands (built-in) commands
    *      - Handle arguments preprocessing if required for some commands: example $ in echo
    *      - Handle var=value local environmental variables
    */
#if (shell_flavor >= NANO_SHELL)
    if ((handle_correct_local_var_format(local_argv, &cmd_indx)) == EXIT_FAILURE)
    {
        goto clean_up;
    }
    replace_all_local_global_var(local_argv, cmd_indx);
#endif
   /* Based on the supported shell check the allowed commands and handle special commands */
    if ((check_flavored_shell(local_argv[cmd_indx])) == EXIT_FAILURE)
    {
        printf("Invalid command\n");
        goto clean_up;
    }
    if (handle_special_commands(local_argv, cmd_indx) == EXIT_SUCCESS)
    {
        goto clean_up;
    }
    handle_shell_preproc_for_non_special_commands(local_argv, cmd_indx);   


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
        child_shell_handler(local_argv, cmd_indx);
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