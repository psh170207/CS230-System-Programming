/* 
 * tsh - A tiny shell program with job control
 * 
 * <Put your name and login ID here>
 * cs20150326 ParkSiHwan
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; // argument list execve()
	int bg; // job run in bg or fg? // 0->fg, 1->bg
	int state; //state of child process
	int i; // iterator for for-loop
	pid_t pid; //process id of child process
	sigset_t mask,prev_mask; //signal mask for block and restore

	bg = parseline(cmdline,argv);
    
	if(argv[0]==NULL) return; // ignore empty lines
	
	if(!builtin_cmd(argv)){ //NOT a builtin command // if it is a builtin command, do immediately
		if(!bg)
			state=FG; // if bg=0, foreground job
		else
			state=BG; // if bg=1, background job

		sigemptyset(&mask);
		sigaddset(&mask,SIGCHLD);
		sigprocmask(SIG_BLOCK,&mask,&prev_mask); //block SIGCHLD before fork() : to avoid race between child and parent
		
		if((pid = fork())==0){ /* Child */
			sigprocmask(SIG_SETMASK,&prev_mask,NULL); //unblock SIGCHLD before execs the new program
			setpgid(0,0); // puts the child in a new process group whose group PID is identical to the child's PID
			if(execve(argv[0],argv,environ)<0){
				printf("%s: Command not found\n", argv[0]);
				exit(1);
			}
		}

		/* Parent */
		addjob(jobs,pid,state,cmdline);
		sigprocmask(SIG_SETMASK,&prev_mask,NULL);//unblock SIGCHLD after addjob
		
		if(!bg) 
			waitfg(pid); // if bg=0, foreground job and parent(shell) wait until foreground process is terminated or stopped
		else{
			for(i=0; i<MAXJOBS; i++)
				if(jobs[i].pid == pid) break;
			printf("[%d] (%d) %s",jobs[i].jid, jobs[i].pid,jobs[i].cmdline); // if bg=1, background job and shell don't need to wait and print added job's info
		}	
	}
} //45 lines

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{
	if(!strcmp(argv[0],"quit"))
		exit(0); // quit command
	if(!strcmp(argv[0],"jobs")) {
		listjobs(jobs); // jobs command
		return 1;
	}	
	if(!strcmp(argv[0],"bg") || !strcmp(argv[0],"fg")) {
		do_bgfg(argv);
		return 1;
	}
	if(!strcmp(argv[0],"&"))
		return 1;
	return 0; // Not a builtin command.

}//15 lines

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{
	pid_t cfpid; //current foreground job's pid
	pid_t pid; //pid of <job>
	int jid; // jid of <job>
	struct job_t *target;

	//determine pid of <job> and check whether it is vaild input

	if(argv[1]==NULL){ // if second argument is NULL(only "bg" or "fg")
		printf("%s command requires PID or %sjobid argument\n",argv[0],"%");
	}
	else if(argv[1][0]!='%' && atoi(argv[1])==0 && strcmp(argv[1],"0")){ // argv[1]'s first char is not a % and argv[1] is start with not an integer and argv[1]!="0"( ex) bg a or fg @!&$...)
		printf("%s: argument must be a PID or %sjobid\n",argv[0],"%");
	}
	else{
		//otherwise, valid input

		//get corresponding job(target)
		if(argv[1][0]=='%'){ // second input argument is <job> and if it starts with '%' => jid
			jid = atoi(&argv[1][1]);
			target = getjobjid(jobs,jid);
			if(target == NULL)
				printf("%s%d: No such job\n","%",jid); // if input jid's corresponding job(target) is NULL, there's no such job
			else
				pid = target->pid; //pid is pid of corresponding job
		} // second input argument is <pid>
		else{
			pid = atoi(argv[1]); // otherwise, pid
			target = getjobpid(jobs,pid);
			if(target == NULL)
				printf("(%d): No such job\n",pid);
		}
		//do bg and fg command using found target
		if(target!=NULL && (target->state==ST || target->state==BG )){ //check the target is vaild if not, do nothing
			//send a signal SIGCONT to target <job>
			if(!strcmp(argv[0],"bg")){
				kill(-pid,SIGCONT);
				target->state = BG; // set to background job
				printf("[%d] (%d) %s",target->jid,target->pid,target->cmdline);
			}
			else if(!strcmp(argv[0],"fg")){
				cfpid = fgpid(jobs);
				if(cfpid!=0) //if there's foreground job, wait for current foreground job
					waitfg(cfpid);

				//No current foreground job
				kill(-pid,SIGCONT);
				target->state = FG; // set to foreground job
				waitfg(pid); // wait until foreground job is terminated or stopped
			}
		}
	}
}//54 lines

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
	int status;
	struct job_t *target;
	target = getjobpid(jobs,pid);
	
	//check whether it is vaild input
	if(target == NULL){
		printf("%d: No such job\n",pid);
		exit(1);
	}
	else if(target->state != FG){
		printf("%d: Not a foreground job\n",pid);
		exit(1);
	}
	//use waitpid to waiting until current foreground job is terminated
	else{
		if(waitpid(pid,&status,WNOHANG|WUNTRACED)<0)
			unix_error("waitfg: waitpid error");
		
		//use busy loop to wait
		while(target->state == FG){
			sleep(1);
		}
	}
}//26 lines

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
	int old_errno = errno; //save previous errno
	sigset_t mask,prev_mask;
	pid_t pid;
	int status;
	struct job_t *target;

	sigfillset(&mask);
	while((pid = waitpid(-1,&status,WNOHANG | WUNTRACED))>0){ //Because pending signals NOT queued, reap all zombie childs when once SIGCHLD received(use WNOHANG|WUNTRACED for both terminated and stopped)
		sigprocmask(SIG_BLOCK,&mask,&prev_mask); //block all signal because it access to global variable(signal handler manual G3)
		
		if(WIFEXITED(status)) //child terminated normally
			deletejob(jobs,pid);

		else if(WIFSTOPPED(status)){ //child stopped by signal
			target = getjobpid(jobs,pid);
			target->state = ST;
			fprintf(stdout,"Job [%d] (%d) stopped by signal %d\n",target->jid,target->pid,WSTOPSIG(status));	
		}
		else if(WIFSIGNALED(status)){ //child terminated by signal
			target = getjobpid(jobs,pid);
			fprintf(stdout,"Job [%d] (%d) terminated by signal %d\n",target->jid,pid,WTERMSIG(status));
			deletejob(jobs,pid);
		}
		sigprocmask(SIG_SETMASK,&prev_mask,NULL); //restore signal mask
	}

	if(pid==-1 && errno!=ECHILD)
		unix_error("waitpid error\n");

	errno = old_errno; //restore errno
} //38 lines

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    int old_errno=errno;
    sigset_t mask, prev_mask;
    pid_t cfpid;
    
    sigfillset(&mask); //Initialize the signal mask to fill set
    sigprocmask(SIG_BLOCK,&mask,&prev_mask); //block all signals and save previous blocked set
    
    cfpid = fgpid(jobs); //cfpid is a current foreground process's id
	if(cfpid!=0) //if there are foreground job
		kill(-cfpid,SIGINT);//send SIGINT to foreground job //since foreground job is a process from execve, SIGINT's action is Default
	//otherwise, do nothing

	sigprocmask(SIG_SETMASK,&prev_mask,NULL); //restore previous blocked set
	errno = old_errno;//restore errno
} //17 lines

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
	int old_errno=errno;
	sigset_t mask, prev_mask;
	pid_t cfpid; // current foreground pid

	sigfillset(&mask); // Initialize the signal mask to fill set
	sigprocmask(SIG_BLOCK,&mask,&prev_mask); // block SIGTSTP signal and save previous blocked set
	
	cfpid = fgpid(jobs);
	if(cfpid!=0)
		kill(-cfpid,SIGTSTP); // send SIGTSTP signal to foreground job
	
	sigprocmask(SIG_SETMASK,&prev_mask,NULL); // restore previous blocked set
	errno = old_errno;
} //17 lines

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/
 
/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
	exit(0);
}
