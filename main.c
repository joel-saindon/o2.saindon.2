#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define TIMER_MSG "Alarm received, killing processes, freeing memory\n"
//#define _POSIX_C_SOURCE >= 199309L


static volatile sig_atomic_t doneflag = 0;

/*ARGSUSED */
static void setdoneflag(int signo){
	doneflag = 1;
}

key_t shmkey1 = 640640; 
key_t shmkey2 = 464064;
key_t shmkey3 = 444767;

int main (int argc, char * argv[]){
	
	alarm(20); //init timeout for process
	int count = 5;
	int i = 0;

	//sig handler taken from Robbins book Program 8.5
	struct sigaction act; //init signal handler for timeout process
	act.sa_handler = setdoneflag;
	act.sa_flags = 0;
	if((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGALRM, &act, NULL) == -1)){
		perror("SIGINT Handler failed");
		return 1;
	}

	//create shared memory segment structures
	int shm_idArray;
	int shm_idMaxProc;
	int shm_idTurn;
	shm_idArray = shmget(shmkey1, 500,IPC_CREAT | 0666);
	shm_idMaxProc = shmget(shmkey2, 10, IPC_CREAT | 0666);
	shm_idTurn = shmget(shmkey3, 10, IPC_CREAT | 0666);
	if(shm_idArray < 0 || shm_idMaxProc < 0 || shm_idTurn < 0) {
		perror("shmget failed");
		exit(1);
	}	
	printf("before array init\n");
	//attach array to shared memory
	int* flagarray = (int *)shmat(shm_idArray, NULL,0);
	int * maxproc = (int *)shmat(shm_idMaxProc, NULL,0);
	int * turn = (int *) shmat(shm_idTurn, NULL, 0);
	printf("%d\n", *maxproc);
	

	for(i=0; i<count; i++){
		flagarray[i] = i;
	} 
	
	for(i=0; i< count; i++)
		printf("\n%d---\n", flagarray[i]);
	//create producer and consumer children
	int opt;
	int status;
	pid_t pids[2];
	pid_t childpid = 0;
	pid_t waitreturn;
	pid_t producerpid;
	pid_t consumerpid;

	for(i=0; i <=1; i++){
		if ((pids[i] = fork()) < 0){
			perror("fork");
			return 1;
		}
		else if(pids[i] == 0){
			pids[i] = getpid();
			printf("child created %d\n", *maxproc);
			*maxproc++;
			sleep(3);
			exit(0);
		}
			
		}
	while(!doneflag){
		printf("Waiting...\n");
		sleep(1);
		(void)waitpid(pids[0], &status,0);
	}
	printf("signal caught, proceeding...\n");
	printf("%d\n", *maxproc);
	*maxproc++;
	printf("%d\n", *maxproc);
	

/*
	if((producerpid == -1) || (consumerpid == -1)){
		perror(argv[0]);
	}
	else if (producerpid == 0){
		printf("producer process created.\n");
		printf("printed from producer\n");
		exit(0);
	}
	else if (consumerpid == 0){
		printf("consumer created.\n");
		printf("printed from consumer");
		exit(0);
	}
	else {
		int status;
		int status2;
		printf("master waiting...\n");
		(void)waitpid(producerpid, &status, 0);
		printf("producer finished\n");
		(void)waitpid(consumerpid, &status2, 0); 
		printf("consumer finished\n");
		printf("master proceeding\n");
	}
*/

	//detach and remove shared memory
	int ArrayDelError = 0;
	int MaxProcError = 0;
	int TurnError = 0;

	ArrayDelError = shmdt(flagarray);
	if(ArrayDelError == -1)
		perror("shmdt fail on flagarray");

	MaxProcError = shmdt(maxproc);
	if(MaxProcError == -1)
		perror("shmdt fail on maxproc");

	TurnError = shmdt(turn);
	if(TurnError == -1);
		perror("shmdt fail on turn");

	int arrayRemoveError = shmctl(shm_idArray, IPC_RMID, NULL);
	int maxProcRemoveError = shmctl(shm_idMaxProc, IPC_RMID, NULL);
	int turnRemoveError = shmctl(shm_idTurn, IPC_RMID, NULL);
	if ((arrayRemoveError == -1) || (maxProcRemoveError == -1) || (turnRemoveError == -1))
		perror("shmctl fail");

	
	

/*	//create shared memory segment
	int shm_id;
	shm_id = shmget(IPC_PRIVATE, count*sizeof(int), 0666);
	if(shm_id < 0) {
		perror("shmget failed");
		exit(1);
	}	
	printf("before array init\n");
	int* flagarray = (int *)shmat(shm_id, NULL,0);

	for(i=0; i<count; i++){
		flagarray[i] = i;
	} 
	
	for(i=0; i< count; i++)
		printf("\n%d---\n", flagarray[i]);

	//detach and remove shared memory
	int delerror = shmdt(flagarray);
	if(delerror == -1){
		perror("shmdt fail");
	}
	int rmerror = shmctl(shm_id, IPC_RMID, NULL);
	if (rmerror == -1)
		perror("shmctl fail");
	

	if (setinterrupt() == -1){
		perror("Failed to setup SIGALRM handler");
		return 1;
	}
	if (setperiodic(10.0) == -1){
		perror("Failed to setup periodic interrupt");
		return 1;
	}
	
	int opt;
	pid_t childpid = 0;
	pid_t waitreturn;
	pid_t producerpid = fork();

	if(producerpid == -1){
		perror(argv[0]);
	}
	else if (producerpid == 0){
		printf("producer process created.\n");
		printf("printed from producer\n");
		exit(0);
	}
	else {
		int status;
		printf("master waiting...\n");
		(void)waitpid(producerpid, &status, 0);
		printf("master proceeding\n");
	}
*/

return 0;

}

int setupitimer(void){ //pulled from pg318 in the Robbins book
	struct itimerval value;
	value.it_interval.tv_sec = 2;
	value.it_interval.tv_usec = 0;
	value.it_value = value.it_interval;
	return (setitimer(ITIMER_PROF, &value, NULL));
}


void interrupt(int signo, siginfo_t *info, void *context){
	int errsave;
	
	errsave = errno;
	write(STDOUT_FILENO, TIMER_MSG, sizeof(TIMER_MSG -1));
	errno = errsave;
}

int setinterrupt(){
	struct sigaction act;

	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = interrupt;
	if((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGALRM, &act, NULL) == -1))
		return -1;
	return 0;
}

/*int setperiodic(double sec) {
	timer_t timerid;
	struct itimerspec value;

	if(timer_create(CLOCK_REALTIME, NULL, &timerid) == -1)
		return -1;

	value.it_interval.tv_sec = (long)sec;
	value.it_interval.tv_nsec = (sec- value.it_interval.tv_sec) * BILLION;
	if (value.it_interval.tv_nsec >= BILLION) {
		value.it_interval.tv_sec++;
		value.it_interval.tv_nsec -= BILLION;
	}
	value.it_value = value.it_interval;
	return timer_settime(timerid, 0, &value, NULL);
}
*/
