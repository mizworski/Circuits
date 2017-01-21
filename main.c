#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "err.h"

#define NR_PROC 10
#define BUF_SIZE               1024

int main ()
{
    pid_t pid;
    char buf[BUF_SIZE];

    for (int i = 1; i <= NR_PROC; i++) {

        int fd[2];
        if (pipe(fd) == -1) syserr("Error in pipe\n");

        switch (pid = fork()) {
            case -1:
                syserr("Error in fork\n");

            case 0:
                if (close(fd[1]) == -1) syserr("Error while closing fd[1]\n");
                if(read(fd[0], buf, BUF_SIZE) == -1)    syserr("Error while reading\n");
                if (close(fd[0]) == -1) syserr("Error while closing fd[0]\n");

                if(i == NR_PROC)
                    printf("%s\n", buf);
                break;

            default:
                if (close(fd[0]) == -1) syserr("Error while closing fd[0]\n");

                char message[50] = "Happy xmas from ";
                char pid_str[15];
                sprintf(pid_str, "%d", getpid());
                strcat(message, pid_str);
                strcat(message, "!\n");

                if(i != 1) {
                    strcat(buf, message);
                    if(write(fd[1], buf, BUF_SIZE) == -1)
                        syserr("Error while writing\n");
                } else {
                    if(write(fd[1], message, sizeof(message)) == -1)
                        syserr("Error while writing\n");
                }

                if (close(fd[1]) == -1)
                    syserr("Error while closing fd[0]\n");

                if (wait(0) == -1)
                    syserr("Error in wait\n");
                return 0;
        }
    }
    /*
    int val;
    char *exp = "x[3]";
    sscanf(exp, "x[%d]", &val);
    printf("%d", val);
     */
    return 0;
}