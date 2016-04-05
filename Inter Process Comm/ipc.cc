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


/*Define Global Variables*/

pid_t   childpid , parentpid;
timeval t1, t2, start_Time, stop_Time ;

//counters
int numtests;
int tests=0;

//for time calculation
double elapsedTime;
double travelTime;
double totaltravelTime=0;
double maxTime;
double minTime;

//flags to check states
bool finish = false;
bool ready = true;
bool sig_call = false;
bool initial_run = true;




void sigusr1_handler(int sig){

    sig_call=true;

}

void sigusr2_handler(int sig){

    sig_call=true;
}

void sigstp_handler(int sig){

    finish=true;

}



//to calculate and update max time
void cal_Max(){

    if(tests == 0)
    {
        maxTime = travelTime;

    }
    else if(maxTime < travelTime)

    {
        maxTime = travelTime;

    }

}


//to calculate and update min time
void cal_Min(){


    if(tests == 0)
    {
        minTime = travelTime;
    }
    else if(minTime > travelTime)

    {
        minTime = travelTime;
    }

}

//print time statics
void print_info()
{

    printf("Process ID is %i, Group ID is %i\n", getpid(), getgid());
    printf("Round trip times\n");
    printf("Average %f\n", totaltravelTime / numtests);
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
	char    read_buf[100];


    /*Three Signal Handlers You Might Need
     *
     *I'd recommend using one signal to signal parent from child
     *and a different SIGUSR to signal child from parent
     */


    Signal(SIGUSR1,  sigusr1_handler); //User Defined Signal 1
    Signal(SIGUSR2,  sigusr2_handler); //User Defined Signal 2
    Signal(SIGTSTP, sigstp_handler); // stop signal

    //Default Value of Num Tests
    numtests=10000;


    //Determine the number of messages was passed in from command line arguments
    //Replace default numtests w/ the commandline argument if applicable


    if(argc<2){
		printf("Not enough arguments\n");
		exit(0);
	}
	else if(argc == 3)
    {
        numtests = atoi(argv[2]);
    }


    printf("Number of Tests %d\n", numtests);


    // start timer
	gettimeofday(&start_Time, NULL);


/*////////////////////////////FOR PIPE//////////////////////////////////////

                 initailread        while(msg != "q")             exit();
                 while(!quit)             read
                 start write
 child  ________________________________________________________________
         |            ^     \             ^
         |           /       \           /
         |         PtoC     C to P     P to C
         |         /           \       /
 parent _|________/_____________\_____/__________wait(NULL);____________
               write           read      write
                        (for all numtest)


/////////////////////////////////////////////////////////////////////////////*/

	if(strcmp(argv[1],"-p")==0){
    //code for benchmarking pipes over numtests

       int j = pipe(fd1);
       int k = pipe(fd2);


        if(j < 0 || k < 0)
   		{
   			printf("%s\n", strerror(errno));
   			exit(1);
   		}



        if((childpid = fork()) == -1)
   		{
   			perror("Fork Error");
   			exit(1);
   		}


        if(childpid==0){

        //child process

            ready=true;

            close(fd1[0]);
            close(fd2[1]);

            //initial read
            nbytes = read(fd2[0], read_buf, 100);


            while(ready==true)
            {
                //check for msg from parent
                if(!strcmp(read_buf, parentmsg) && nbytes > 0)
                {
                    gettimeofday(&t1, NULL);

                    write(fd1[1], childmsg, strlen(childmsg)+1);

                    nbytes = read(fd2[0], read_buf, 100);

                    gettimeofday(&t2, NULL);


                    travelTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
                    travelTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
                    totaltravelTime += travelTime;
                    cal_Min();
                    cal_Max();
                    tests++;


                }

                else if(!strcmp(read_buf, quitmsg))

                 {

                 ready = false;

                 }

            }

        printf("Child's Results for Pipe IPC mechanisms\n");
        print_info();


        }
        else{


            //parent process
            int i=0;
            close(fd1[1]);
            close(fd2[0]);

            while( i<numtests)

            {

                gettimeofday(&t1, NULL);

                write(fd2[1], parentmsg, strlen(parentmsg)+1);

                nbytes = read(fd1[0], read_buf, 100);

                if(!strcmp(read_buf, childmsg) && nbytes > 0)
                {
                    gettimeofday(&t2, NULL);

                    travelTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
                    travelTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
                    totaltravelTime += travelTime;
                    cal_Min();
                    cal_Max();
                    tests++;
                }

                i++;
            }

                write(fd2[1], quitmsg, strlen(quitmsg)+1); // Tell child to stop
                wait(NULL);


            printf("Parent's Results for Pipe IPC mechanisms\n");
            print_info();

        }


		// stop timer
		gettimeofday(&stop_Time, NULL);

		// compute and print the elapsed time in millisec
		elapsedTime = (stop_Time.tv_sec - start_Time.tv_sec) * 1000.0;      // sec to ms
		elapsedTime += (stop_Time.tv_usec - start_Time.tv_usec) / 1000.0;   // us to ms
		printf("Elapsed Time %f\n", elapsedTime);


	}


/*///////////////////////////////////////////////////////////////////////////

     for(;;);                  SU1...continues until stop   exit();
  while(firstrun)
     ________________SU1________________________________________________
      |            ^<--->\            ^
      |           /       \          /
      |         su1       SU2       SS
      |         /           \      /
     _|________/_____________\<SU2>/___________________________________
                    wait()    if(numtests>tests)

/////////////////////////////////////////////////////////////////////////////*/



	if(strcmp(argv[1],"-s")==0){
		//code for benchmarking signals over numtests

        if((childpid = fork()) == -1)
   		{
   			perror("Fork Error");
   			exit(1);
   		}



         if(childpid == 0) // Child Process
            {
            parentpid = getppid();
            while (!finish)//wait until child get sigstp signal
            {

                //this will execute only once
                //chech for SIGUSER1 signal from parent

                if (initial_run && sig_call)
                {
                    gettimeofday(&t1, NULL);

                   //set flags
                    initial_run = false;
                    sig_call = false;

                    kill(parentpid, SIGUSR2);//send SIGUSER2 to parent

                }

                //whenever SIGUSER1 by from parent it sets sig_call to true
                else if(!initial_run && sig_call)

                {

                    gettimeofday(&t2, NULL);

                    travelTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
                    travelTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
                    totaltravelTime += travelTime;
                    cal_Min();
                    cal_Max();
                    tests++;

                    gettimeofday(&t1, NULL);
                    sig_call = false;

                    kill(parentpid, SIGUSR2);
                }
            }


                printf("Child's Results for Signal IPC mechanisms\n");
                print_info();
        }
        else
        {

            gettimeofday(&t1, NULL);

            kill(childpid, SIGUSR1); //signal at the initial_run to child

            while (tests < numtests)
            {
                if (sig_call)
                {
                    gettimeofday(&t2, NULL);
                    travelTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
                    travelTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
                    totaltravelTime += travelTime;
                    cal_Min();
                    cal_Max();
                    tests++;



                    gettimeofday(&t1, NULL);
                    sig_call = false;

                    //replt to child
                    kill(childpid, SIGUSR1);
                }
            }

            kill(childpid, SIGTSTP);


            wait(NULL);//wait for child to


            printf("Parents's Results for Signal IPC mechanisms\n");
            print_info();



        }


            // stop timer
            gettimeofday(&stop_Time, NULL);


		// compute and print the elapsed time in millisec
		elapsedTime = (stop_Time.tv_sec - start_Time.tv_sec) * 1000.0;      // sec to ms
		elapsedTime += (stop_Time.tv_usec - start_Time.tv_usec) / 1000.0;   // us to ms
		printf("Elapsed Time %f\n", elapsedTime);
	}

}
