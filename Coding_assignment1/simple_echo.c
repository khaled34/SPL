/*--------------------------------- Private includes -------------------------------------*/
#include <common_utils.h>
/*--------------------------------- Private definitions ---------------------------------  */
#define MIN_NUM_ARGUMENTS       1
#define INDEX_FIRST_ARGUMENT    1
#define DELIMITER_CHAR         " "
/*---------------------------------------- Main ----------------------------------------  */
int main(int argc, char *argv[])
{
    /* Arguments check*/
    if (argc < MIN_NUM_ARGUMENTS)
    {
        log_and_handle(LOG_ERR, "UsageError:%s [something you need to echo]\n", argv[0]);
    }

    for (int argi = INDEX_FIRST_ARGUMENT; argi < argc; argi++)
    {
        printf("%s"DELIMITER_CHAR, argv[argi]);
    }
    printf("\n");
    
    exit(EXIT_SUCCESS);
}
/*----------------------------------------- EOF ----------------------------------------  */
