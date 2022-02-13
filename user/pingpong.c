#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    char* data = (char *)malloc(1);
    *data = 8;
    int p1[2];
    int p2[2];
    pipe(p1);
    pipe(p2);

    int pid = fork();
    if(pid == 0){
        //child
        close(p1[0]); // close unused read end of p1
        close(p2[1]); // close unused write end of p2
        void* readResult = malloc(1);
        read(p2[0], readResult, 1);
        printf("%d: received ping\n", getpid());
        write(p1[1], readResult,1);
        close(p1[1]);
        close(p2[0]);
        free(readResult);
        exit(0);
    }
    else{
        //parent
        close(p1[1]); // close unused write end of p1
        close(p2[0]); // close unused read end of p2
        write(p2[1], data , 1); // write byte of data to child
        close(p2[1]); // finished writing to p2
        int status;
        wait(&status); // wait for child
        void* readResult = malloc(1);
        read(p1[0], readResult, 1);
        if(*(char*)readResult == *data){
            printf("%d: received pong", getpid());
        }
        free(readResult);
        close(p1[0]);
        close(p2[1]);
    }
    free(data);

  exit(0);
}