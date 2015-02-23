/*
 * CS 551 : Project 1: Shell Development
 * Team: Group 1: Nagaraj, Darshan, Sairam
 * myshell.c: main function. Entry point for shell program.
 */

#define _POSIX_SOURCE

#include "common.h"

/*
 * main() function. Entry point for the program.
 */
int main()
{
    char *home;
    char *promptsign;
    int retval=0;
    char buf[SIZE];

    struct sigaction new_act, old_act;
    do
    {
        log_info("Shell Start. PID %d", getpid());
        /* 
         * Read profile file.
         * Assume default promptsign and home directory if file is not
         * in expected format or file is not found.
         */
        if(readProfile("prompt") == NULL)
        {
            /* if NULL assign the prompt sign and home folder */
            promptsign="$";
            home=getcwd(NULL,0);
        }
        else
        {
            promptsign=readProfile("prompt");
            home=readProfile("home");
        }

        /* Signal handling. For ctrl + c */
        new_act.sa_handler = SIG_IGN;
        retval=sigaction(SIGINT, &new_act, &old_act);
        if(retval<0)
        {
            log_err("retval[%d]",retval);
            break;
        }

        setjmp(sjbuf);
        if (old_act.sa_handler != SIG_IGN)
        {
            new_act.sa_handler = signalhandler;
            retval=sigaction(SIGINT, &new_act, &old_act);
            if(retval<0)
            {
                log_err("retval[%d]",retval);
                break;
            }
        }

        /*
         * Init the command array. This will store the input command and the
         * level w.r.t it. Ex: if the command is (wc, (ls =l)) then ls-l will be
         * at level 2 and wc will be at level 1.
         */
        initializeCommandArray(cArray);

        /*
         * Read variables, if any exists for calculator application.
         */
        if(!readFileNew())
        {
            log_err("readFromFile() failed");
            //retval =-1;
        }
        chdir(home);

        /* Main loop */
        while(TRUE)
        {
            char cwd[255];

            /* print our custom prompt */
            getcwd(cwd,sizeof(cwd));
            printf("%s%s ",cwd,promptsign);

            /* Read command line input from user */
            if (!fgets(buf, 100, stdin))
                return 0;

            /* Process user input */
            if(buf!=NULL)
            {
                char *temp =buf;
                char *token;
                char finalString[1024];
                char *normal;
                normal = buf;

                memset(finalString,0,1024);

                if(buf[0]=='(') /* command with '(' */
                {
                    /*execute my code*/
                    int finalcount=0;

                    /* Tokenize user input and parse it */
                    token = strtok(temp,",\n");
                    retval = parseToken(token,cArray,1);
                    while(token!=NULL)
                    {
                        token= strtok(NULL,",\n");
                        if(token!=NULL)
                            retval = parseToken(token,cArray,0);
                    }

                    /*
                     * execute the command based on their level, as righmost input in
                     * '()' has higher priority.
                     */
                    retval=sortLevel(cArray);

                    debug("Final string %s", finalString);
                    while(cArray[finalcount].level != -1)
                    {
                        strcat(finalString,cArray[finalcount].input);

                        /* If their are multiple commands then insert pipe in
                         * between */
                        if(finalcount < CARRAY_SIZE && cArray[finalcount+1].level != -1)
                            strcat(finalString," | ");
                        finalcount++;
                    }
                    debug("Final string %s", finalString);
                    /* Execute command */
                    Execute(finalString);
                }
                else /* calculator */
                {
                       /**execute  code**/
                    char *token = buf;
                    char *temp ;
                    temp = strtok(token,":");
                    debug("temp contains %s",temp);

                    if(!strcmp(temp,"calc"))  /* calculator will be invoked with
                                              * 'calc' prefix. */
                    {
                        char *newBuf;
                        int i=0;

                        /* fetch operands and operation */
                        temp = strtok(NULL,"\n");
                        debug("temp contains %s buf len is %d",temp,strlen(buf));
                        newBuf = (char *)malloc(sizeof(char)*(strlen(temp)));
                        if (!newBuf) {
                            log_info("Malloc failed.");
                            continue;
                        }
                        debug("temp lenght is %d",strlen(temp));
                        strcpy(newBuf,temp);
                        char *token = newBuf;

                        if(!strcmp(token,"show"))
                        {
                            showOperands();
                        }
                        else if(strchr(token,' '))
                        {
                            printf("space not allowed. Type a command without space\n");
                        }
                        else if(strchr(token,'='))
                        {
                            debug("YOU STRING CONTAINS = %s",newBuf);
                            saveOperands(newBuf);
                        }
                        else
                        {
                            debug("calling calculateOperator",newBuf);
                            calculateOperator(newBuf);
                        }
                    }
                    else /* parse as normal command */
                    {
                        retval = Execute(normal);
                    }
                }
            }
        }
    } while (0);
    return 0;
}

