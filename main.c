// HW0 Part 1
// Group: Cody Nguyen, Ian Khoo, Olivia Pham

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipe_pc[2]; // producer -> consumer
    int pipe_cp[2]; // consumer -> producer (ack)
    pid_t pid;
    int num;
    const int LIMIT = 5;

    // make two pipes
    pipe(pipe_pc);
    pipe(pipe_cp);

    pid = fork();

    if (pid == 0) {
        // child = Consumer
        close(pipe_pc[1]); // not writing here
        close(pipe_cp[0]); // not reading here

        for (int i = 0; i < LIMIT; i++) {
            read(pipe_pc[0], &num, sizeof(num));
            printf("Consumer: %d\n", num);
            fflush(stdout);
            char ack = 'x';
            write(pipe_cp[1], &ack, 1);
        }
        exit(0);

    } else {
        // parent = Producer
        close(pipe_pc[0]); // not reading here
        close(pipe_cp[1]); // not writing here

        for (int i = 1; i <= LIMIT; i++) {
            num = i;
            printf("Producer: %d\n", num);
            fflush(stdout);
            write(pipe_pc[1], &num, sizeof(num));
            char ack;
            read(pipe_cp[0], &ack, 1); // wait for Consumer
        }
        
        wait(NULL);
    }

    return 0;
}
