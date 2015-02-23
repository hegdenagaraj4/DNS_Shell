/*
 * CS 551 : Project 1: Shell Development
 * Team: Group 1: Nagaraj, Darshan, Sairam
 * myfunctions.c: Helper Routines used by myshell.c for various operations.
 */

#define _POSIX_SOURCE
#include "common.h"

KEYVALUE        *keyValue=NULL;
int             s[SIZE];
int             top=-1;

/*
 * Create a copy of the given command.
 */
char* duplicate(char *command)
{
    int len;
    char *hs;

    if(command==NULL)
    {
        return NULL;
    }

    len=strlen(command)+1;
    hs=(char*)malloc(len*sizeof(char));

    if (hs==NULL) { /* Malloc failed. Handle gracefully */
        log_err("Malloc failed");
        return NULL;
    } else {
        strcpy(hs,command);
        return hs;
    }
}

/*
 * Handler to handle signals like ctrl + c.
 */
void signalhandler(int signo)
{
    switch(signo)
    {
        case SIGINT:
        {
            char str;
            printf("\nAre you sure?[Y/N]");
            str=getchar();
            if(str=='y' || str=='Y')
                exit(0);
            else
                longjmp(sjbuf,1);
            break;
        }
        case SIGCHLD:
        {
            int pid;
            pid=wait(NULL);
            printf("PID:%d completed job.\n",pid);
            longjmp(sjbuf,1);
        }
    }
}

/*
 * Read profile file and return the value of the requested type
 */
char* readProfile(char *type)
{
    FILE *fp;
    char *home,*promptsign, *temp,*temp1,* pos;
    char command[20];
    int len;

    do
    {
        fp=fopen("PROFILE","r");
        if(fp==NULL)
        {
            log_err("Cannot open PROFILE file");
            return NULL;
        }

        while(fgets(command, sizeof(command), fp)!=NULL)
        {
            temp=strtok(command,"=");

            if(strchr(temp,'#')) {
                continue;
            }

            /* ftech prompt specified by user in profile file */
            if(!strcmp(temp,"prompt")){
                temp1=strtok(NULL,"=");
                promptsign=duplicate(temp1);
                if (!promptsign) {
                    return NULL;
                }
                len=strlen(promptsign);
                pos=promptsign+len-1;
                *pos='\0';
            }
            /* ftech path specified by user in profile file and set env */
            else if(!strcmp(temp,"path"))
            {
                char *path;
                temp1=strtok(NULL,"=");
                pos=temp1+strlen(temp1)-1;
                *pos='\0';
                debug("Path - %s", temp1);
                setenv("PATH", temp1, 1);
            }
            /* ftech home directory specified by user in profile file */
            else if(!strcmp(temp,"home"))
            {
                temp1=strtok(NULL,"=");
                home=duplicate(temp1);
                if (!home) {
                    return NULL;
                }
                len=strlen(home);
                pos=home+len-1;
                *pos='\0';
            }
        }

        fclose(fp);

        if(!strcmp(type,"home"))
            return home;
        else if (!strcmp(type,"prompt"))
            return promptsign;
        else
            return NULL;
    } while(0);
}

/*
 * Initialize/reset given command array
 */
void initializeCommandArray(COMMAND_ARRAY *cArray)
{
    int i=0;

    if (cArray==NULL)
    {
        return;
    }

    for(i=0;i<CARRAY_SIZE;i++)
    {
        memset(&cArray[i].input,0,sizeof(SIZE));
        cArray[i].level = -1;
    }
}

/*
 * Parse command line input given by user
 */
int parseToken(char *buf,COMMAND_ARRAY *cArray, int clear)
{
    int retval=1;
    int length=0;
    int index=0,count=0;
    static int slevel=0;
    static int array_count;

    if(clear)
    {
        slevel = 0;
        array_count = 0;
    }

    if(buf==NULL)
    {
        return 0;
    }

    debug("ENTRY %s",buf);

    do
    {
        length = strlen(buf);
        debug("length is %d",length);

        while(length)
        {
            char *c =  (char *)&buf[index];

            /* raise our level if/when '(' is encountered in input */
            if(*c=='(')
            {
                ++slevel;
            }

            /* lower our level if/when '(' is encountered in input */
            if(*c==')')
            {
                --slevel;
            }

            if(isalpha(buf[index]) || isspace(buf[index]) || (*c=='-'))
            {
                cArray[array_count].level = slevel;
                cArray[array_count].input[count] =  buf[index];
                debug("cArray[array_count++].input[count++] is %c",cArray[array_count].input[count]);
                count++;
            }
            index++;
            length--;
        }
        cArray[array_count].input[count] = '\0';
        debug("cArray[%d].input[count] %s",array_count,cArray[array_count].input);
        debug("cArray[%d].level is %d",array_count,cArray[array_count].level);
        array_count++;
    }while(0);

    debug("EXIT");
    return retval;
}

/*
 * sort command array as per the commands level.
 */
int sortLevel(COMMAND_ARRAY *cArray)
{
    int retval=0;
    int c=0,d=0,n=0,t=0	;

    while(cArray[n].level!=-1)
    {
        n++;
    }

    debug("n value is %d",n);

    for (c = 0 ; c < ( n - 1 ); c++)
    {
        for (d = 0 ; d < n - c - 1; d++)
        {
            if (cArray[d].level < cArray[d+1].level) /* For decreasing order use < */
            {
                t                   = cArray[d].level;
                cArray[d].level     = cArray[d+1].level;
                cArray[d+1].level   = t;

                char temp[1024];
                strcpy(temp,cArray[d].input);
                strcpy(cArray[d].input,cArray[d+1].input);
                strcpy(cArray[d+1].input,temp);

            }
        }
    }
    return retval;
}

/*
 * Execute the command
 */
int Execute(char *buf)
{

    char *arg_list[30] = {0};
    int status;
    int counter = 0;
    int counter2 = 0;
    int isrun =0;
    int pid    =-1;
    int retval  = 1;

    do
    {
        if(buf)
        {
            char first_argument[64] = {0};

            if( strchr(buf,'&') || strchr(buf,'<') || strchr(buf,'>'))
            {
                printf("Command with &,< and > is not supported\n");
                break;
            }

            arg_list[counter] = strtok(buf, " \n");

            if( arg_list[counter] != NULL )
            {
                strcpy( first_argument, arg_list[0]);

                while(arg_list[counter] != NULL)
                {
                    debug("%s***",arg_list[counter]);
                    counter++;
                    arg_list[counter] = strtok(NULL, " \n");
                }

                /* Handle cd command */
                if( strcmp( first_argument, "cd" ) == 0 )
                {
                    char cwd[255];
                    debug("Cd command ");

                    if( arg_list[1] != NULL )
                    {
                        /* If command is cd -; go back to old directory if
                         * exists */
                        if( strcmp(arg_list[1], "-") == 0 )
                        {
                            if (valid_oldpwd) 
                            {
                                char cwd[255];
                                getcwd(cwd, sizeof(cwd));
                                chdir( oldpwd );
                                stpncpy(oldpwd, cwd, sizeof(oldpwd));
                                debug("Oldpwd %s.. ", oldpwd);
                            }
                        }
                        else
                        {
                            /* cache current directory value before changing the
                             * directory */
                            valid_oldpwd = 1;
                            stpncpy(oldpwd, getcwd(NULL, 0), sizeof(oldpwd));
                            debug("Oldpwd %s.. %s", oldpwd, getcwd(NULL, 0));
                            chdir( arg_list[1] );
                        }
                    }
                    return retval;
                }
                /* Handle 'exit' or 'quit' */
                else if( (strcmp( first_argument, "exit" ) == 0) || 
                         (strcmp( first_argument, "quit" ) == 0) )
                {
                    log_info("Shell Exit: pid %d", getpid());
                    exit(0);
                }

                while(arg_list[counter2] != NULL)
                {
                    if(strchr(arg_list[counter2],'|'))
                        debug("Pipe command entered");
					
                    if( !strcmp( arg_list[counter2], "|"))
                    {
                        int num_pcmds = 0;
                        int tmp = 0;
                        while (arg_list[tmp] != NULL) {
                            debug("arg_list[%d]: %s", tmp, arg_list[tmp]);
                            if (!strcmp( arg_list[tmp], "|")) {
                                num_pcmds++;
                            }
                            tmp++;
                        } 
                       
                        {
                            int fds[num_pcmds][2];
                            int arg[num_pcmds + 1][2];
                            int cmd = 1; /* atleast 1 command */
                            pid_t pids[num_pcmds + 1];

                            tmp = 0;
                            arg[cmd - 1][0] = 0;

                            /* Mark when each command is starting and ending */
                            while (arg_list[tmp] != NULL) {
                                if (!strcmp( arg_list[tmp], "|")) {
                                    arg[cmd - 1][1] = tmp;
                                    pipe(&fds[cmd - 1][0]);
                                    debug("cmd %d.. fds[%d][0] %d fds[%d][1] %d", 
                                           cmd -1, cmd -1, fds[cmd - 1][0],
                                           cmd -1, fds[cmd - 1][1]);
                                    cmd++;
                                    if (cmd <= num_pcmds + 1) {
                                        arg[cmd - 1][0] = tmp + 1;
                                    }
                                }
                                tmp++;
                            }
                            arg[cmd -1][1] = tmp;

                            for (tmp = 0; tmp <= num_pcmds; tmp++) {
                                debug("num_pcmds %d: arg[%d][0].. %d; arg[%d][1].. %d", 
                                       num_pcmds, tmp, arg[tmp][0], tmp, arg[tmp][1]);
                            }

                            for (tmp = 0; tmp <= num_pcmds; tmp++) {

                                int k;
                                pids[tmp] = fork();
    
                                if (pids[tmp] == 0) { // child

                                    if (tmp == 0) {
                                        dup2(fds[tmp][PIPE_WRITE], STDOUT_FILENO);
                                    } else if (tmp == num_pcmds) {
                                        dup2(fds[tmp - 1][PIPE_READ], STDIN_FILENO );
                                    } else {
                                        dup2(fds[tmp - 1][PIPE_READ], STDIN_FILENO );
                                        dup2(fds[tmp][PIPE_WRITE], STDOUT_FILENO);
                                    }

                                    arg_list[arg[tmp][1]] = 0;

                                    for (k = 0; k < num_pcmds; k++) {
                                        debug("k %d: 0 %d, 1 %d", k, fds[k][0],
                                                fds[k][1]);
                                        close(fds[k][0]);
                                        close(fds[k][1]);
                                    }

                                    for (k = arg[tmp][0]; k <= arg[tmp][1]; k++)
                                    {
                                        debug("tmp %d: arg_list[%d] %s", tmp, k,
                                                arg_list[k]);
                                    }

                                    /* execute the command */
                                    execvp(arg_list[arg[tmp][0]], &arg_list[arg[tmp][0]]);
                                    exit(0);
                                }
                            }
                            for (tmp = 0; tmp < num_pcmds; tmp++) {
                                close(fds[tmp][0]);
                                close(fds[tmp][1]);
                            }
                            for (tmp = 0; tmp <= num_pcmds; tmp++) {
                                waitpid(pids[tmp], NULL, 0);
                            }
                        } 
                        /* All is done */
                        isrun=1;
                        break;
                    }
                    counter2++;
                }
                
                /* Handle normal command.. No pipe */
                if (arg_list[0] !=NULL && isrun==0)
                {
                    debug("first_argument %s", first_argument);
                    pid = fork();
                    if(pid <0) return 1;
                    else if(pid ==0)
                    {
                        execvp( first_argument, arg_list );
                        exit(0);
                    }
                    else
                    {
                        waitpid( pid, &status, 0 );
                    }
                }
            }
        }
    }while(0);

    return retval;
}

/*******************************************/
/*      Calculator related routines        */
/*******************************************/

/* For given key search for its value */
int searchValueFromKey(char *key)
{
    int retval =1, value=-1;
    bool bFound = false;
    KEYVALUE *temp=keyValue;
    char ch;

    if(key==NULL)
    {
        return -1;
    }

    do
    {
    	while(temp != NULL)
        {
            ch = key[0];
            if(temp->key[0] ==ch)
            {
                value = temp->value;
                break;
            }
            temp = temp->next;
        }
    }while(0);

    return value;
}

/* Update the value of the given key */
int updateKeyValue(char *key, char *value)
{
    int retval=1;
    KEYVALUE *newKeyValue=NULL,*temp;

    debug("ENTRY key %s value %s",key,value);

    if(key!=NULL)
    {
        newKeyValue = (KEYVALUE *)malloc(sizeof(KEYVALUE));
        if (!newKeyValue) {
            log_err("Malloc failed");
            return 0;
        }
        memset(newKeyValue,0,sizeof(KEYVALUE));
        strcpy(newKeyValue->key,key);
        debug("ATOI of VALUE is %d",atoi(value));
        newKeyValue->value = atoi(value);
        debug("newKeyValue->value contains %d", newKeyValue->value);
        newKeyValue->next = NULL;
        if(keyValue==NULL)
        {
            keyValue = newKeyValue;
        }
        else
        {
            temp = keyValue;
            while(temp!=NULL)
            {
                if(strcmp(temp->key,key) == 0)
                {
                    debug("[t %p; t->n %p] temp->key %s, key %s", temp,
                            temp->next, temp->key, key);
                    temp->value = atoi(value);
                    free(newKeyValue);
                    break;
                }
                else if (temp->next == NULL)
                {
                    temp->next = newKeyValue;
                    debug("t %p, t->n %p, new %p, new->n %p", temp, temp->next,
                            newKeyValue, newKeyValue->next);
                    break;
                }
                temp = temp->next;
            }
        }
    }

    return retval;
}

/* print key and values */
void showOperands()
{
    char ch;
    KEYVALUE *temp=keyValue;

    log_info("Variable :: value");
    while(temp != NULL)
    {
        log_info(" %c     ::   %d",
                 temp->key[0], temp->value);
        temp = temp->next;
    }
}

/* read keys and their values from calculator file. */
int readFileNew()
{
    FILE *fp;
    char* filepath;
    char *key;
    char *value;
    char command[SIZE];
    int len;
    int val;
    char* pos;
    int res=1;

    do
    {
        filepath=CALCULATOR;
        fp=fopen(filepath,"r");
        if(fp==NULL)
        {
            log_err("Cannot open %s file", filepath);
            res =0;
            break;
        }

        while(fgets(command, sizeof(command), fp)!=NULL)
        {
            if(command == '\0' || command == " ")
                break;

            if(strlen(command) == 1)
            {
                continue;
            }
            key=(char *) malloc(sizeof(SIZE));
            value=(char *) malloc(sizeof(SIZE));
            if (!key || !value) {
                log_err("Malloc failed");
                res = 0;
                break;
            }
            key=strtok(command,"=");
            value=strtok(NULL,"=");
            len=strlen(value);
            pos=value+len-1;
            *pos='\0';

            val=updateKeyValue(key,value);
            if(val==0)
            {
                log_err("updateKeyValue failed");
                res =0;
                break;
            }
        }
    }while(0);

    if (fp != NULL) {
        fclose(fp);
    }
    return res;
}

/* reverse contents of given string */
void reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    char temp;

    while (start < end)
    {
    	temp = *(str + start);
        *(str + start) = *(str+end);
        *(str + end) = temp;
        start++;
        end--;
    }
}

/* Customized i to a */
char* custitoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;
 
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled only with 
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}

int saveOperands(char *buf)
{
	
    int retval=1;
    int resultofEval = 0;
    int evaldone = 0;
    char buffer[20];
    char *token,*evalBuf;
    FILE *fp;

    do
    {
        if(strchr(buf,'+') || strchr(buf,'-') || strchr(buf,'*') || strchr(buf,'/'))
        {
            evalBuf = strtok(buf,"=");
            evalBuf = strtok(NULL," \n");
            resultofEval = calculateOperator(evalBuf);

            if(resultofEval == -1)
            {
                retval = -1;
                printf("Sorry. Cannot store -ve values. please store +ve values.\n");
                break;
            }
            debug("cal result is %d",resultofEval);
            evaldone = 1;
        }

        fp = fopen(CALCULATOR,"a");
        if(fp == NULL)
        {
            printf("File not found\n");
            retval = -1;
            break;
        }

        token=strtok(buf," =");
        
        debug("token %s",token);
        fputs(token,fp);

        fputs("=",fp);
        debug("= written to file, evaldone = %d", evaldone);
        
        if(evaldone)
        {
            custitoa(resultofEval,buffer,10);   // here 10 means decimal
            token = buffer;
        }
        else
        {
            token=strtok(NULL," =\n");
        }

	debug("token %s",token);
	fputs(token,fp);
	printf("= %s\n",token);
        fputs("\n",fp);
        fclose(fp);

        if(!readFileNew())
        {
            log_err("readFromFile() failed");
            retval =-1;
        }
    }while(0);

    return retval;
}


/********* Stack *************/
/****** Stack Operations *****/

struct Stack* createStack( unsigned capacity )
{
    struct Stack* stack = (struct Stack*) malloc(sizeof(struct Stack));

    if (!stack)
        return NULL;

    stack->top = -1;
    stack->capacity = capacity;

    stack->array = (int*) malloc(stack->capacity * sizeof(int));

    if (!stack->array)
        return NULL;
    return stack;
}

int isEmpty(struct Stack* stack)
{
    return stack->top == -1 ;
}

char peek(struct Stack* stack)
{
    return stack->array[stack->top];
}

char pop(struct Stack* stack)
{
    if (!isEmpty(stack))
        return stack->array[stack->top--] ;
    return '$';
}

void push(struct Stack* stack, char op)
{
    stack->array[++stack->top] = op;
}


// A utility function to check if the given character is operand
int isOperand(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

// A utility function to return precedence of a given operator
// Higher returned value means higher precedence
int Prec(char ch)
{
    switch (ch)
    {
    case '+':
    case '-':
        return 1;

    case '*':
    case '/':
        return 2;

    case '^':
        return 3;
    }
    return -1;
}

// The main function that converts given infix expression
// to postfix expression.
int infixToPostfix(char* exp)
{
    int i, k;

    // Create a stack of capacity equal to expression size
    struct Stack* stack = createStack(strlen(exp));
    if(!stack) // See if stack was created successfully
        return -1 ;

    for (i = 0, k = -1; exp[i]; ++i)
    {
         // If the scanned character is an operand, add it to output.
        if (isOperand(exp[i]))
            exp[++k] = exp[i];

        // If the scanned character is an ‘(‘, push it to the stack.
        else if (exp[i] == '(')
            push(stack, exp[i]);

        //  If the scanned character is an ‘)’, pop and output from the stack
        // until an ‘(‘ is encountered.
        else if (exp[i] == ')')
        {
            while (!isEmpty(stack) && peek(stack) != '(')
                exp[++k] = pop(stack);
            if (!isEmpty(stack) && peek(stack) != '(')
                return -1; // invalid expression
            else
                pop(stack);
        }
        else // an operator is encountered
        {
            while (!isEmpty(stack) && Prec(exp[i]) <= Prec(peek(stack)))
                exp[++k] = pop(stack);
            push(stack, exp[i]);
        }

    }

    // pop all the operators from the stack
    while (!isEmpty(stack))
        exp[++k] = pop(stack );

    exp[++k] = '\0';
    printf( "%s\n", exp );
}

/* Function for PUSH operation */
pushInt(int elem)
{
    debug("elem is %d, top = %d",elem, top);       
    s[++top]=elem;
}

/* Function for POP operation */
int popInt()
{
    debug("pop now.current top: %d ",top);
    return(s[top--]);
}

int evaluatePostfixExpression(char *buf)
{                         /* Main Program */
    char ch;
    int i=0,op1,op2;
    int value = 0;

    bzero(s, SIZE);
    top = -1;
    debug("ENTRY is %s",buf);
    while( (ch=buf[i++]) != '\0')
    {
        debug("character is %c",ch);
        if(isalpha(ch))
        {
            value = searchValueFromKey(&ch);
            if(value == -1)
            {
                printf("variable not saved before. save it and then perform operation \n");
                break;
            }

            debug("value is %d",value);
            pushInt(value); /* Push the operand */
        }
        else
        {   /* Operator,pop two  operands */
            debug("ENTERED HERE... %c, %d", ch, ch);
            if(ch == ' ')
                continue;
            op2=popInt();
            op1=popInt();
            switch(ch)
            {
               case '+':
                    debug("%d",op1+op2);
                    pushInt(op1+op2);
                break;
               case '-':pushInt(op1-op2);
                break;
               case '*':pushInt(op1*op2);
                break;
               case '/':
                    if(op2 == 0)
                    {
                        printf("ERROR: Divide by zero\n");
                        return -1;
                    }
                    pushInt(op1/op2);
                break;
            }
        }
    }
    log_info("Result: %d",s[top]);
	return s[top];
}

bool checkIfDigitExist(char *buf)
{
    bool bRet = false;
    int i=0;
    for(i=0;i<strlen(buf);i++)
    {
        if(isdigit(buf[i]))
        {
            debug("digit is %c.. %d",buf[i],buf[i]);
            bRet = true;
        }
    }
    return bRet;
}

/* Calculates given expression */
int calculateOperator(char *buf)
{
    int retval=-1;
    do
    {
        // convertKeyToValue(value, buf);
        if(checkIfDigitExist(buf))
        {
            printf("Assign value to a variable and then perform the operation.\n");
            printf("Calculator does not support direct arthimetic operations on numbers\n");
            retval = -1;
            break;
        }
        infixToPostfix(buf);
        debug("after infixToPostfix buf has %s", buf);
        retval = evaluatePostfixExpression(buf);
    }while(0);
    return retval;
}
