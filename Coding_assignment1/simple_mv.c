/*--------------------------------- Private includes -------------------------------------*/
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <common_utils.h>
/*--------------------------------- Private definitions ---------------------------------  */
#define MAX_NUM_ARGUMENTS                     3
#define SIZE_OF_NULL_TERMINATOR               1
#define SIZE_OF_BACK_SLASH_PATH_DELIMITER     1
/*--------------------------------- Private functions ---------------------------------  */
int check_if_directory(const char *path) 
{
    struct stat path_stat;

    if (lstat(path, &path_stat) < 0) 
    {
        if (errno == ENOENT) 
        {
            /* skip error of not existing file as this is expected for now */
            return FALSE; 
        } 
        else 
        {
            log_and_handle(LOG_ERR, "stat failed for %s", path);
        }
    }
    return S_ISDIR(path_stat.st_mode);
}
/*---------------------------------------- Main ----------------------------------------  */
int main(int argc, char *argv[])
{
    char * dest_path = NULL;
    int cleanup = FALSE;
    
    /* Arguments check*/
    if (argc != MAX_NUM_ARGUMENTS)
    {
        log_and_handle(LOG_ERR, "UsageError:%s src-file dest-file\n", argv[0]);
    }

    if(check_if_directory(argv[2]))
    {
        int pathLen = strlen(argv[2]) + strlen(basename(argv[1])) + 
                        SIZE_OF_BACK_SLASH_PATH_DELIMITER + SIZE_OF_NULL_TERMINATOR; 
                        
        if ((dest_path = malloc(sizeof(char) * pathLen)) == NULL)
        {
            log_and_handle(LOG_ERR, "RunTime error: malloc\n");
        }
        strncpy(dest_path,argv[2], strlen(argv[2]));
        strcat(dest_path,"/");
        strcat(dest_path,basename(argv[1]));
        cleanup = TRUE;
    }
    else
    {
        dest_path = argv[2];
    }

    if (rename(argv[1], dest_path) < 0) 
    {
        log_and_handle(LOG_ERR, "RunTime error: rename\n");
    }

    if (cleanup == TRUE)
    {
        free(dest_path);
    }
    exit(EXIT_SUCCESS);
}
/*----------------------------------------- EOF ----------------------------------------  */
