cmake_minimum_required(VERSION 3.4)
project(Praktikum1)


set(SOURCE_FILES src/main.c src/abp_sender.c src/abp_empfaenger.c src/abp_empfaenger.h)
add_executable(Praktikum1 ${SOURCE_FILES})