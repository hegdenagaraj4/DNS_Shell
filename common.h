/*
 * CS 551 : Project 1: Shell Development
 * Team: Group 1: Nagaraj, Darshan, Sairam
 * common.h: Header file containing various standard header files, Macros,
 * Global Variables and function definitions.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>

#include "debug.h"

#define PROFILE "PROFILE"
#define FILENAME "./alias.txt"
#define CALCULATOR "./calculator.txt"

#define TRUE 1
#define FALSE 0
#define MAX_LEN  1024
#define ARG_COUNT 64
#define PIPE_READ 0
#define PIPE_WRITE 1
#define SIZE 1024
#define CARRAY_SIZE 20
#define MAX 100

typedef struct _command_
{
    char input[SIZE];
    int level;
}COMMAND_ARRAY;

typedef struct hashtable{
    char *key;
    char *value;
    struct hashtable *next;
}hash;

typedef struct _keyvalue_
{
    char key[1024];
    int value;
    struct _keyvalue_ *next;
}KEYVALUE;

struct Stack
{
    int top;
    unsigned capacity;
    int* array;
};

jmp_buf         sjbuf;
hash*           hasht[SIZE];
COMMAND_ARRAY   cArray[CARRAY_SIZE];
int             valid_oldpwd;
char            oldpwd[255];

char* readProfile(char *type);
void signalhandler(int signo);
void initializeCommandArray(COMMAND_ARRAY *cArray);
int readFile();
int parseToken(char *buf,COMMAND_ARRAY *cArray, int clear);
int sortLevel(COMMAND_ARRAY *cArray);
int Execute(char *buf);
char* duplicate(char *command);
int calculateOperator(char *buf);
int saveOperands(char *buf);
void showOperands();

#endif
