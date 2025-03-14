/*--------------------------------- Private includes -------------------------------------*/
#define _GNU_SOURCE
#include <common_utils.h>
#include <unistd.h>
#include <string.h>
/*--------------------------------- Private definitions ---------------------------------  */
#define NUM_ARGUMENTS   1
/*---------------------------------------- Main ----------------------------------------  */
int main(int argc, char *argv[])
{
    char *cwd = NULL;
    /* Arguments check*/
    if (argc != NUM_ARGUMENTS)
    {
        log_and_handle(LOG_WARN, "WARNING:[UsageError:%s] rest of arguments will be discarded\n", argv[0]);
    }

    /* get the current working directory dynamically */
    if ((cwd = get_current_dir_name()) == NULL)
    {
        log_and_handle(LOG_ERR, "Error in usage of get_current_dir_name() errno: %s\n", strerror(errno));
    }
    printf("%s\n", cwd);

    free(cwd);
    exit(EXIT_SUCCESS);
}
/*----------------------------------------- EOF ----------------------------------------  */
