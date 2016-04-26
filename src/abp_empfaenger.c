//
// Created by s-gheldd on 4/25/16.
//

#include "abp_empfaenger.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "main.h"

enum empfaenger_state {
    E1, E2
};

enum empfaenger_state abp_empfaenger_state;

volatile sig_atomic_t abp_empfaenger_signal = SIGUSR2;

void abp_empfaenger_sighandler(int signum, siginfo_t *info, void *ptr);

void abp_empfaenger_sighandler(int signum, siginfo_t *info, void *ptr) {
    const char *FAIL = "\tFAIL";
    const char *OK = "\tok";
    char buffer[MSGLEN + BUFSIZE];
    int incoming = pipe_one[0];
    read(incoming, buffer, MSGLEN);
    int ok = 0;
    switch (abp_empfaenger_state) {
        case E1:
            ok = buffer[1] == '0';
            break;
        case E2:
            ok = buffer[1] == '1';
            break;
    }
    if (ok) {
        strcat(buffer, OK);
    } else {
        strcat(buffer, FAIL);
    }
    if ((rand() % 100) < (ABP_EMPFAENGER_LOSSRATE)) {
#ifdef VERBOSE
        write_to_stdout("LOST");
#endif
        abp_empfaenger_signal = SIGALRM;
    } else {
        write_to_stdout(buffer);
        abp_empfaenger_signal = signum;
    }
}

int abp_empfaenger_run() {
    const char *message = "JumpsOverTheLazyDog";
    char buffer[MSGLEN];
    int outgoing = pipe_two[1];

    pid_t sender = getppid();
    abp_empfaenger_state = E1;

    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = abp_empfaenger_sighandler;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR2, &act, NULL) != 0) {
        return 1;
    }

    srand(time(NULL));
    pause();

    while (*message) {
        if (abp_empfaenger_signal != SIGALRM) {
            switch (abp_empfaenger_state) {
                case E1:
                    *buffer = *(message++);
                    *(buffer + 1) = CHAR_ZERO;
                    *(buffer + 2) = CHAR_TERMINATOR;
                    write(outgoing, buffer, MSGLEN);
                    abp_empfaenger_state = E2;
                    break;

                case E2:
                    *buffer = *(message++);
                    *(buffer + 1) = CHAR_ONE;
                    *(buffer + 2) = CHAR_TERMINATOR;
                    write(outgoing, buffer, MSGLEN);
                    abp_empfaenger_state = E1;
                    break;
            }
            //write_to_stdout("empfaenger wait");
            kill(sender, SIGUSR1);
        }
        pause();
    }

    kill(sender, SIGUSR1);
    sleep(1);
    return 0;
}