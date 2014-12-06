#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80
#define ARR_SIZE 128
#define HISTORY_COUNT 10

/* Function history lists all the last 10 commands that were stored in the history buffer, which has been defined as a circular buffer*/
int history(char *hist[], int current)
{
        int i = current; 
        int hist_id = 1;
        int len = 0;
        do {
                if (hist[i]) {
						len = strlen(hist[i]);
						if (hist[i][len] == '\0' && hist[i][len-1]=='\n')
							printf("%1d  %s", hist_id, hist[i]);
						else
						{
							hist[i][len]='\n';
							hist[i][len+1]='\0';
							printf("%1d  %s", hist_id, hist[i]);
						}
                        hist_id++;
                }
                i = (i + 1) % HISTORY_COUNT;
        } while (i != current);
        return 0;
}

/* history_recall recalls the last command that was executed, returning as char * command */
// returns index of appropriate history buffer; -1 sentinel
int history_recall(char *hist[], int current)
{
        int i = current;
        int hist_id = 1;
        do {
                if (hist[i]) {
                        hist_id++;
                }
                i = (i + 1) % HISTORY_COUNT;
        } while (i != current-1);
        printf("	executing %1d  %s", current, hist[i]);
        return i;
}

/* args_parse parses the arguments passed in the shell and stores them in args[] under the specific index, in the order in which they appear. This is a one time call function, used to process any additional arguments that maybe specified*/
int args_parse(char *buffer, char** args, 
                size_t size_argument, size_t *narguments)
{
    char *buffer_argument[size_argument]; 
    char **cp;
    char *wbuf;
    size_t i, j;
    int result = false;
   
    wbuf=buffer;
    buffer_argument[0]=buffer; 
    args[0] =buffer;
    
    for(cp=buffer_argument; (*cp=strsep(&wbuf, " \n\t")) != NULL ;){
        if ((*cp != '\0') && (++cp >= &buffer_argument[size_argument]))
            break;
    }
    for (j=i=0; buffer_argument[i]!=NULL; i++){
        //"&" does not count as an argument; it does not write into the arguments
        if(strlen(buffer_argument[i])==1 && buffer_argument[i][0] == '&' )
        {
            printf("   Running command as concurrent child process\n");
            result = true;
        }
        else if(strlen(buffer_argument[i])>0)
        {
            args[j++]=buffer_argument[i];
		}
        else if(strlen(buffer_argument[i]) == j)
			args[j] = NULL;
    }  
    *narguments=j;
    args[j]=NULL;
    return result;
}

/*
void omegaParser(char *buffer, char** args, 
                size_t size_argument, size_t *narguments){

    int whitey = false;  //whitey = true; I am in whitespace, not a string    
    int i=0;
    int j=1;
    args[0] = buffer;
    while (buffer[i] != '\0'){
        switch(buffer[i]){
            case ' ':
            case '\n':
            case '\t':
                whitey = true;
                buffer[i]='\0';

            default:
                if (whitey){
                    whitey=false;
                    args[j++]=buffer + i;        
                }
        }
        i++;
    }
    *narguments = (size_t) j;
    args[j]=NULL;
}
 */
 
 
// forkCommand executes the command represented by args in a child and waits for the child to return
// unless "&" argument was passed
void forkCommand( char ** args,     //represents the command to execute
                size_t narguments,  //represents size of args
                int childFlag )     //indicates '&' argument was given
{
    int *ret_status; 
	pid_t pid;
	pid = fork(); // fork 
	if (pid == 0) //if in the child thread
	{
		if ((strcmp(args[0], "exit" ) != 0 ) && (strcmp(args[0], "history") != 0)){
			if (execvp(args[0], args)){ // execute the command
					puts(strerror(errno)); //return error if command not found/invalid
					exit(127);				
		}}
	}
	else if(pid<0)
	{
		printf("No child process created\n"); 
	}
	else
	{
        if (!childFlag)			
		    pid = wait(ret_status);		//process waits if in the parent process		
	}
    return;
}

/* Main Function */

int main(int argc, char *argv[], char *envp[]){
    char buffer[MAX_LINE];
    char buffer2[MAX_LINE];
    char *args[ARR_SIZE];
    char *hist[HISTORY_COUNT];
	int should_run = 1;
	int i, current = 0;
    size_t narguments;
    size_t ln;

    //v2
    int childFlag = false;          // true if & was entered to run in background 
    int history_size = 0;   // tracks size of history to check for out of bounds
    int h;                  //temporary used during command "history" interpretation

	for (i = 0; i < HISTORY_COUNT; i++) // initialising the buffer to be empty
	{
		hist[i] = NULL;
	}
                
    while(should_run){
        printf("osh> ");                // defining the prompt
        fgets(buffer, MAX_LINE, stdin); // accept the value/command from the user
        free(hist[current]);            // release resourcein circular buffer to be overwritten
		hist[current] = strdup(buffer); //update the history buffer	
		current = (current + 1) % HISTORY_COUNT; // update the position of current in the history buffer
        childFlag = args_parse(buffer, args, ARR_SIZE, &narguments); // to parse the arguments passed
        if (narguments==0) continue; // To avoid a segmentation fault, incase the user just hits an enter without supplying any command(s)
		
        if ((strcmp(args[0], "history") == 0) && (strcmp(args[0], "exit") != 0))
        {
			if(narguments>1)
			{
                // for command "history !n"
                // for command "history !!"
                if (strcmp(args[1], "!!") == 0) 
                {
                    h = (current + 9) % HISTORY_COUNT; //go to previous history number
				}
                else    
                {
                    h = atoi( args[1] + 1 );    //pointer math, skip the first char (i.e. skip '!')
				}
                //check for appropriate input and execute
                if ( h > history_size || h < 1){
                    printf("No such command in history.\n");
                    printf("%1d is not in valid history range of 1 to %1d\n",h, history_size);
                } else {
                    //h is converted from user command # to circular array command #
				    h =	history_recall(hist, h); 

                    //v4 changed the logic here so that history !n to history works
                    if (h != -1){
                        //causes the history !!   
                        //copy into buffer, let buffer get parsed instead
                        strcpy(buffer, hist[h]);                          
                        childFlag = args_parse(buffer, args, ARR_SIZE, &narguments);
                        //omegaParser(buffer, args, ARR_SIZE, &narguments);
                        //printf("debug 193: narguments is %1d: %s\n",(int)narguments, args[0]);
                        //childFlag = false;

                        //execute hist[h] again
                        if (strcmp(args[0],"history") == 0){
                            if(narguments > 1){
                                strcpy(buffer2, hist[h]); 
                                buffer2[strlen(buffer2)-1] = '\0';
                                printf("        **> Referenced command %s does nothing via history recall\n", buffer2);
                            }
                            else 
                                history(hist, current);	
                        } else {
                            forkCommand( args, narguments, childFlag );
                        }
                    } //end if h != -1
				} //end else from if check on bounds for h
            } // end "history X" commands
    		else 
                // command "history" (no extra arguments)
    			history(hist, current);	 // for history
		} //end all command "history" logic

        else if (strcmp( args[0], "exit" ) == 0) // test for exit
			should_run = 0;          //check for exit command                    
        //if not history or exit command, pass command to kernel

		else
		{  
            forkCommand( args, narguments, childFlag );
		}

        if ( ++history_size > HISTORY_COUNT ) history_size = HISTORY_COUNT;   //track history size; maximum is 10

    }   //end while
    return 0;
}

     
