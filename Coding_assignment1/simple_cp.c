/*--------------------------------- Private includes -------------------------------------*/
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <common_utils.h>
/*--------------------------------- Private definitions ---------------------------------  */
#define MAX_NUM_CP_ARGUMENTS   3

#define SRC_FILE_FLAGS    (O_RDONLY)
#define DST_FILE_FLAGS    (O_CREAT | O_WRONLY | O_TRUNC)
#define DST_FILE_MODE     (S_IRWXU | S_IRGRP | S_IROTH)

#ifndef BUF_SIZE
#define BUF_SIZE 1024
#endif
/*---------------------------------------- Main ----------------------------------------  */
int main(int argc, char *argv[])
{
    int sourceFd, destFd;
    ssize_t numRead;
    char buf[BUF_SIZE];
    
    /* Arguments check*/
    if (argc != MAX_NUM_CP_ARGUMENTS)
    {
        log_and_handle(LOG_ERR, "UsageError:%s src-file dest-file\n", argv[0]);
    }

    /* Open source file */
    if ( (sourceFd = open(argv[1], SRC_FILE_FLAGS)) < 0)
    {
        log_and_handle(LOG_ERR, "opening file %s", argv[1]);
    }

    /* Open/create destination file */
    if ((destFd = open(argv[2], DST_FILE_FLAGS, DST_FILE_MODE)) < 0)
    {
        log_and_handle(LOG_ERR, "opening file %s", argv[2]);
    }
    
    /* Transfer data until we encounter end of input or an error */
    while ((numRead = read(sourceFd, buf, BUF_SIZE)) > 0)
    {
        if (write(destFd, buf, numRead) != numRead)
        {
            log_and_handle(LOG_ERR, "couldn't write whole buffer");
        }
    }
    
    /* Error handling */
    if (numRead == -1)
    {
        log_and_handle(LOG_ERR, "Error Reading from source file (errno: %s)\n", strerror(errno));
    }
    if (close(sourceFd) == -1)
    {
        log_and_handle(LOG_ERR, "Error closing the source file (errno: %s)\n", strerror(errno));
    }
    if (close(destFd) == -1)
    {
        log_and_handle(LOG_ERR, "Error closing the  destination (file errno: %s)\n", strerror(errno));
    }
    
    exit(EXIT_SUCCESS);
}
/*----------------------------------------- EOF ----------------------------------------  */
