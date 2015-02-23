/*
 * CS 551 : Project 1: Shell Development
 * Team: Group 1: Nagaraj, Darshan, Sairam
 * debug.h: debug related macros.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_info(M, ...) fprintf(stderr,M "\n",##__VA_ARGS__)
#define log_err(M, ...) fprintf(stderr, " [ERROR] [%s]: [%s]: (%d): errno: %s " M "\n", __FILE__, __func__, __LINE__, clean_errno(), ##__VA_ARGS__)

//#define debug(M, ...) fprintf(stderr, "[DEBUG] [%s]: [%s]: (%d): " M "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define debug(M, ...)
