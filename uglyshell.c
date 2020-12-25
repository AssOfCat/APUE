#include "apue.h"
#include <sys/wait.h>
#include <fcntl.h>

#define PID_MAX 32727

static void	sig_int(int);		/* our signal-catching function */
static void	promptShow();		/* prompt custom printf*/
static int	commandHandling(char *inCommand ,
				char **outComand ,
				int *isBg ,
				char *inRDFile ,
				char *outRDFile);	/* handle input command*/


int
main(void)
{
	char	buf[MAXLINE];	/* from apue.h */
	pid_t	pid;
	int	status;
	int 	backPID[PID_MAX] = {0};

	if (signal(SIGINT, sig_int) == SIG_ERR)
		err_sys("signal error");

	printf("This is my uglyshell!\n");
	promptShow();	
	while (fgets(buf, MAXLINE, stdin) != NULL) {

		if (strlen(buf) == 1){
			promptShow();
			continue;
		}
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; /* replace newline with null */

		char *allCommands[MAXLINE] = {0};/* all the commands(after commandHandling() )for execvp*/
		int isBackground = 0;		/*run in background or not*/
		char inputFilename[MAXLINE] = {0};
		char outputFilename[MAXLINE] = {0};/*redirection filename*/


		if(commandHandling(buf , allCommands , &isBackground , inputFilename , outputFilename)){
			printf("command handle error! \n");
			promptShow();
			continue;
		}


		if ((pid = fork()) < 0) {
			err_sys("fork error");
		} else if (pid == 0) {		/* child */

			/*redirection filename in*/
			if(inputFilename[0] != 0){
				int infd;
				if( (infd = open(inputFilename, O_RDONLY))< 0){
					printf("can't open file (input)");
					exit(127);
				}
				dup2(infd, fileno(stdin));
				close(infd);
			}
			/*redirection filename out*/
			if(outputFilename[0] != 0){
				int outfd;
				if( (outfd = open(outputFilename, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0){
					printf("can't open file (output)");
					exit(127);
				}
				dup2(outfd, fileno(stdout));
				close(outfd);
			}
			execvp(allCommands[0], allCommands);			

			err_ret("couldn't execute: %s", buf);
			exit(127);
		}

		/* parent */
		if(isBackground){
			backPID[pid] = 1;
			printf("background: %s [%d]\n", allCommands[0], pid);
			promptShow();
		}else{
		if ((pid = waitpid(pid, &status, 0)) < 0)
			err_sys("waitpid error");
		promptShow();
		}
	}
	exit(0);
}


/* our signal-catching function */
void
sig_int(int signo)
{
	printf("interrupt\n");
	promptShow();
}


/* prompt custom printf*/
void
promptShow(){
	char *promptEnv = getenv("PROTYPE");	/*Custom prompt*/
	if(promptEnv == NULL)
		printf("->");
	else{
		printf("%s",promptEnv);
	}
}

/*	handle input command
 *	inCommand:input command (string)
 *	outComand:output command (string[])
 *	isBg:backgroung ornot (int pointer)
 *	inRDFile,outRDFile:RedirectionFile (string)
 *	return:0 , if wrong return 1(int)
 */

int
commandHandling(char *inCommand , char **outComand , int *isBg , char * inRDFile , char *outRDFile){

	char *partCommand = NULL;
	char tmpBuffer[MAXLINE];
	int inRDTimes = 0;
	int outRDTimes = 0;
	int commandCount = 0;

	partCommand = strtok(inCommand, " ");
	while(partCommand != NULL){
		sscanf(partCommand, "%s", tmpBuffer);
		/*RedirectionFile in*/
		if(strcmp(tmpBuffer , "<") == 0){
			if(inRDTimes != 0){
				printf("you can only type < once");
				return -1;
			}
				
			++inRDTimes;
			partCommand = strtok(NULL , " ");
			if(partCommand == NULL){
				printf("< must be followed by filename \n");
				return -1;
			}else{
				sscanf(partCommand, "%s", inRDFile);
				partCommand = strtok(NULL, " ");
				continue;					
			}
		}
		/*RedirectionFile out*/
		if(strcmp(tmpBuffer , ">") == 0){
			if(outRDTimes != 0){
				printf("you can only type > once");
				return -1;
			}
				
			++outRDTimes;
			partCommand = strtok(NULL , " ");
			if(partCommand == NULL){
				printf("> must be followed by filename \n");
				return -1;
			}else{
				sscanf(partCommand, "%s", outRDFile);
				partCommand = strtok(NULL, " ");
				continue;					
			}
		}
		/*run in background*/
		if( strcmp(tmpBuffer, "&") == 0)
		{
			partCommand = strtok(NULL, " ");

			if(partCommand != NULL){
				printf("& must be the last character \n");
				return -1;
			}else{
				*isBg = 1;
				return 0;					
			}
		}
		/*handle outCommand*/
		int len = strlen(tmpBuffer) + 1;
		char *temp = (char *) malloc( len * sizeof(char));
		strncpy(temp, tmpBuffer, len);
		outComand[commandCount++] = temp;


		partCommand = strtok(NULL, " ");
	}
	return 0;
}


