//
// Created by s-gheldd on 4/25/16.
//

#ifndef PRAKTIKUM1_MAIN_H
#define PRAKTIKUM1_MAIN_H

#define MSGLEN 3
#define BUFSIZE 6
#define CHAR_ZERO '0'
#define CHAR_ONE '1'
#define CHAR_TERMINATOR '\0'
#define TIME_OUT 1

void write_to_stdout(char *string);

int pipe_one[2], pipe_two[2];

#endif //PRAKTIKUM1_MAIN_H
