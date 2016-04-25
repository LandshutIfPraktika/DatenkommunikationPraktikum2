//
// Created by s-gheldd on 3/21/16.
//
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "abp_sender.h"
#include "abp_empfaenger.h"

int main(void) {
    int exit_state;
    pid_t pid;
    pipe(pipe_one);
    pipe(pipe_two);


    if ((pid = fork()) == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid) {
        //parent
        close(pipe_one[0]);
        close(pipe_two[1]);
        exit_state = abp_sender_run(pid);
        close(pipe_one[1]);
        close(pipe_two[0]);
    } else {
        //child
        close(pipe_one[1]);
        close(pipe_two[0]);
        exit_state = abp_empfaenger_run();
        close(pipe_one[0]);
        close(pipe_two[1]);
    }
    return exit_state;
}

void write_to_stdout(char *string) {
    write(STDOUT_FILENO, string, strlen(string));
    write(STDOUT_FILENO, "\n", 1);
}
