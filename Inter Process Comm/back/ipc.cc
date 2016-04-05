//
//
//
// Name: Tejas Patil CSU-ID: 006949762
//

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>
#include <limits.h>
#include <sys/time.h>

#include "helper-routines.h"

void update_time(timeval t1, timeval t2);

/*Define Global Variables*/
pid_t   childpid , parentpid;
timeval t1, t2, start_Time, stop_Time ;
int numtests;
int tests=0;
double elapsedTime;
double travelTime;
double travelTimeSum;
double maxTime;
double minTime;
int returnStatus;
bool quit = false;
bool sig_call = false;
bool first_run = true;


void sigusr1_handler(int sig){

    sig_call=true;

}

void sigusr2_handler(int sig){

    sig_call=true;
}

void sigint_handler(int sig){

    quit=true;

}

void update_time(timeval t1, timeval t2){

    travelTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
    travelTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
    if(tests == 0)
    {
        travelTimeSum = travelTime;
        maxTime = travelTime;
        minTime = travelTime;
    }
    else
    {
        travelTimeSum += travelTime;

        if(maxTime < travelTime)
        { maxTime = travelTime; }

        if(minTime > travelTime)
        { minTime = travelTime; }
    }
    tests++;

}


double cal_average(){

    return travelTimeSum / numtests ;

}


void print_info(char* argv1){

    char test_type[10] = "Pipe";
    if(strcmp(argv1,"-s")==0)
    { strcpy(test_type, "Signal"); }

    char process_name[10] = "Parent";
    if(childpid == 0)
    { strcpy(process_name, "Child"); }

    printf("Number of Tests %d\n", numtests);
    printf("%s's Results for %s IPC mechanisms\n", process_name, test_type);
    printf("Process ID is %i, Group ID is %i\n", getpid(), getpgid(getpid()));
    printf("Round trip times\n");
    printf("Average %f\n", cal_average());
    printf("Maximum %f\n", maxTime);
    printf("Minimum %f\n", minTime);

}
//
// main - The main routine
//
int main(int argc, char **argv){
	//Initialize Constants here

    //variables for Pipe
	int fd1[2],fd2[2], nbytes;
	//byte size messages to be passed through pipes
	char    childmsg[] = "1";
	char    parentmsg[] = "2";
	char    quitmsg[] = "q";
	char readbuffer[80]; ///upd

    /*Three Signal Handlers You Might Need
     *
     *I'd recommend using one signal to signal parent from child
     *and a different SIGUSR to signal child from parent
     */
    Signal(SIGUSR1,  sigusr1_handler); //User Defined Signal 1
    Signal(SIGUSR2,  sigusr2_handler); //User Defined Signal 2
    Signal(SIGINT, sigint_handler);

    //Default Value of Num Tests
    numtests=10000;
    //Determine the number of messages was passed in from command line arguments
    //Replace default numtests w/ the commandline argument if applicable
    if(argc<2){
		printf("Not enough arguments\n");
		exit(0);
	}
	else if(argc == 3)
    { numtests = atoi(argv[2]); }

    //printf("Number of Tests %d\n", numtests);
    // start timer
	gettimeofday(&start_Time, NULL);

	if(strcmp(argv[1],"-p")==0){
		//code for benchmarking pipes over numtests
        pipe(fd1);
        pipe(fd2);

        childpid=fork();

        if(childpid==0){

        //child process

            quit=false;
            close(fd1[0]);
            close(fd2[1]);

            nbytes = read(fd2[0], readbuffer, sizeof(readbuffer));

            while(quit==false){



                if(strcmp(readbuffer, parentmsg) == 0 && nbytes > 0)
                {
                    gettimeofday(&t1, NULL);
                    write(fd1[1], childmsg, strlen(childmsg)+1);
                    nbytes = read(fd2[0], readbuffer, sizeof(readbuffer));
                    gettimeofday(&t2, NULL);
                    update_time(t1, t2);
                }
                else if(strcmp(readbuffer, quitmsg) == 0)
                { quit = true; }
                else
                { printf("Error Reading"); }///upd

            }

            print_info(argv[1]);


        }
        else{
            //parent process

            close(fd1[1]);
            close(fd2[0]);

            for (int i=0; i<numtests; i++){

                gettimeofday(&t1, NULL);
                write(fd2[1], parentmsg, strlen(parentmsg)+1);

                nbytes = read(fd1[0], readbuffer, sizeof(readbuffer));

                if(strcmp(readbuffer, childmsg) == 0 && nbytes > 0)
                {
                    gettimeofday(&t2, NULL);
                    update_time(t1, t2);
                }


            }

                write(fd2[1], quitmsg, strlen(quitmsg)+1); // Tell child to stop

                waitpid(childpid, &returnStatus, 0); // Wait for child to finish
                print_info(argv[1]);

        }


		// stop timer
		gettimeofday(&stop_Time, NULL);

		// compute and print the elapsed time in millisec
		elapsedTime = (stop_Time.tv_sec - start_Time.tv_sec) * 1000.0;      // sec to ms
		elapsedTime += (stop_Time.tv_usec - start_Time.tv_usec) / 1000.0;   // us to ms
		printf("Elapsed Time %f\n", elapsedTime);
	}


	if(strcmp(argv[1],"-s")==0){
		//code for benchmarking signals over numtests

          childpid = fork();


         if(childpid == 0) // Child Process
        {
            parentpid = getppid();
            while (quit == false)
            {
                if (first_run == true && sig_call == true)
                {
                    gettimeofday(&t1, NULL);
                    first_run = false;
                    sig_call = false;
                    kill(parentpid, SIGUSR2);
                }
                else if(first_run == false && sig_call == true)
                {
                    gettimeofday(&t2, NULL);
                    update_time(t1, t2);
                    gettimeofday(&t1, NULL);
                    sig_call = false;
                    kill(parentpid, SIGUSR2);
                }
            }
            print_info(argv[1]);
        }
        else
        {

            gettimeofday(&t1, NULL);
            kill(childpid, SIGUSR1); // Send initial signal to child
            while (tests < numtests)
            {
                if (sig_call == true)
                {
                    gettimeofday(&t2, NULL);
                    update_time(t1, t2);
                    gettimeofday(&t1, NULL);
                    sig_call = false;
                    kill(childpid, SIGUSR1);
                }
            }
            kill(childpid, SIGINT);

            waitpid(childpid, &returnStatus, 0); // Wait for child to terminate

            print_info(argv[1]);



        }



		// stop timer
		gettimeofday(&stop_Time, NULL);

		// compute and print the elapsed time in millisec
		elapsedTime = (stop_Time.tv_sec - start_Time.tv_sec) * 1000.0;      // sec to ms
		elapsedTime += (stop_Time.tv_usec - start_Time.tv_usec) / 1000.0;   // us to ms
		printf("Elapsed Time %f\n", elapsedTime);
	}

}
