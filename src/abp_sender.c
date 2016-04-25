//
// Created by s-gheldd on 4/25/16.
//

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "abp_sender.h"
#include "main.h"

enum sender_state {
    S0, S1, S2
};

struct sender_saved_state {
    char *message;
    enum sender_state state;
};

void abp_sender_save_state(struct sender_saved_state *saved_state, enum sender_state sender_state, const char *message);

void abp_sender_restore_state(struct sender_saved_state saved_state, enum sender_state *sender_state,
                              const char **message);

void abp_sender_sighandler(int signum, siginfo_t *info, void *ptr);

void abp_alarm_handler(int signum, siginfo_t *info, void *ptr);

enum sender_state abp_sender_state;

volatile sig_atomic_t abp_sender_signal = SIGALRM;

void abp_alarm_handler(int signum, siginfo_t *info, void *ptr) {
    //write_to_stdout("ALARM");
    abp_sender_signal = signum;
}

void abp_sender_sighandler(int signum, siginfo_t *info, void *ptr) {
    const char *FAIL = "\tfail";
    const char *OK = "\tok";
    char buffer[MSGLEN + BUFSIZE];
    int incoming = pipe_two[0];
    int ok = 0;

    read(incoming, buffer, MSGLEN);

    switch (abp_sender_state) {
        case S1:
            ok = buffer[1] == '0';
            break;
        case S2:
            ok = buffer[1] == '1';
            break;
    }
    if (ok) {
        strcat(buffer, OK);
    } else {
        strcat(buffer, FAIL);
    }
    write_to_stdout(buffer);
    abp_sender_signal = signum;
}

int abp_sender_run(pid_t empfaenger) {
    const char *message = "TheQuickBrownFoxOle";
    char buffer[MSGLEN];
    int outgoing = pipe_one[1];
    struct sender_saved_state saved_state;


    abp_sender_state = S0;
    //enum sender_state state = S0;

    struct sigaction act;
    struct sigaction alarm_act;

    memset(&act, 0, sizeof(act));
    memset(&alarm_act, 0, sizeof(alarm_act));
    act.sa_sigaction = abp_sender_sighandler;
    alarm_act.sa_sigaction = abp_alarm_handler;
    act.sa_flags = alarm_act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR1, &act, NULL) != 0) {
        return 1;
    }
    if (sigaction(SIGALRM, &alarm_act, NULL) != 0) {
        return 2;
    }


    sleep(1);

    while (*message) {
        alarm(0);

        abp_sender_save_state(&saved_state, abp_sender_state, message);
        switch (abp_sender_state) {
            case S0:
                *buffer = *(message);
                *(buffer + 1) = CHAR_ZERO;
                *(buffer + 2) = CHAR_TERMINATOR;
                write(outgoing, buffer, MSGLEN);
                abp_sender_state = S1;
                break;

            case S1:
                *buffer = *(message++);
                *(buffer + 1) = CHAR_ONE;
                *(buffer + 2) = CHAR_TERMINATOR;
                write(outgoing, buffer, MSGLEN);
                abp_sender_state = S2;
                break;

            case S2:
                *buffer = *(message++);
                *(buffer + 1) = CHAR_ZERO;
                *(buffer + 2) = CHAR_TERMINATOR;
                write(outgoing, buffer, MSGLEN);
                abp_sender_state = S1;
                break;
        }
        //write_to_stdout("sender wait");
        alarm(TIME_OUT);
        kill(empfaenger, SIGUSR2);

        if (*message) {
            pause();
        }
        if (abp_sender_signal == SIGALRM) {
            abp_sender_restore_state(saved_state, &abp_sender_state, &message);
        }
    }
    alarm(0);
    sleep(1);
    return 0;
}

void abp_sender_save_state(struct sender_saved_state *saved_state, enum sender_state sender_state,
                           const char *message) {
    saved_state->message = message;
    saved_state->state = sender_state;
}

void abp_sender_restore_state(struct sender_saved_state saved_state, enum sender_state *sender_state,
                              const char **message) {
    *message = saved_state.message;
    *sender_state = saved_state.state;
}