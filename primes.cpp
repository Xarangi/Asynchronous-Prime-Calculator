#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include<string.h>
#include <sys/wait.h>
#include "headers.h"

using namespace std;

//global variables to store number of sig1s and sig2s received
int numsig1=0;
int numsig2=0;

//program to check if given input is a number
bool isnum(string s)
{
    for(int i=0;i<s.size();i++)
    {
        if(!isdigit(s[i]))
            return 0;
    }
    return 1;
}
//main function
int main (int argc, char* argv[]){

    signal(SIGUSR1 , handler); // callign the handler whenever we receive a SIGUSR1 signal
    signal(SIGUSR2 , handler); // callign the handler whenever we receive a SIGUSR2 signal

    //initializing variables we will use in the program
	int i;
    int j;
    //to check if a type has been assigned (-r/-e)
    int re=-1;
    //to store number of children of root adn delegators
	int nums=2;
    //store the type of worker node (primes1 or primes2)
	int type;
    //storing the bounds given by the user
	int lbound=0;
	int ubound=10;
    //storing calculated bounds for delegator
    int lb;
    int ub;
    //store the batch of worker node (to send SIGUSR1 or SIGUSR2) - assigned to delegator node
    int batch=0;

    //storing pid of root process to send signals
    pid_t root_pid=getpid();

    //interpreting command line input
    //if number of inline parameters are less than 7 the code won't run
	if(argc<7)
	{
        cerr<<"Please input all parameters\n";
        exit(EXIT_FAILURE);
	}

	for(int i=1;i<argc;i++)
    {   
        //setting the lower bound in the root node
        if(strcmp(argv[i],"-l")==0)
        {   i++;
            
            if (isnum(argv[i]))
            {
                lbound = atoi(argv[i]);
            }
            else
            {
                cerr<<"Enter an integer for the lower bound";
                exit(EXIT_FAILURE);
            }
        }
        //setting the upper bound in the root node
        else if (strcmp(argv[i],"-u")==0)
        {   
            i++; 
            if (isnum(argv[i]))
            {
                ubound = atoi(argv[i]);
            }
            else
            {
                cerr<<"Enter an integer for the upper bound\n";
                exit(EXIT_FAILURE);
            }
        }
        //setting distribution to equal
        else if (strcmp(argv[i],"-e")==0)
        {
            re=0;
        }
        //setting distribution to random
		else if (strcmp(argv[i],"-r")==0)
        {
            re=1;
        }
        //setting the number of child nodes the root/delegator nodes will have
		else if (strcmp(argv[i],"-n")==0)
        {
            i++;
            if(isnum(argv[i]))
                nums=atoi(argv[i]);
            else
            {    
                cerr<<"Give a number after -n\n";
                exit(EXIT_FAILURE);
            }
        }
    }

    //code to handle error checking

    if(lbound<1)
    {
        cout<<"Lower bound changed to 1\n";
        lbound=1;
    }
    if(lbound>ubound)
    {
        cerr<<"Lower bound was set greater than upper bound!\n";
        exit(0);
    }
    //case when the total number of worker nodes are more than toal number of integers in the range
    if(nums*nums>ubound-lbound)
    {
        cout<<"more worker nodes than total numbers in range: \nSome workers won't perform any computations\n";
    }
    ub=lbound-1;
    //arrays of pids to store pids of different children who have been forked (not necessary)
    //for delegators
    pid_t p[nums];
    //for workers
    pid_t pc[nums];

    //pipe to receive data from delegator nodes (delegator->root)
    int prdx[2];
    int prdxt[2];
    if(pipe(prdx)<0)
    {
        cerr<<"\npipe error\n";
        exit(EXIT_FAILURE);
    }

    pipe(prdxt);
    //array of pipes to send data to all delegator nodes
    int prdxs[nums][2];

    for(j=0; j<nums;j++)
    {
        pipe(prdxs[j]);
        //fork creates two processes
        p[j]=fork();
        //batch of the process is set
        batch=j%2;
        //delegator node
        if(p[j]==0)
        {   
            close(prdx[0]);
            close(prdxt[0]);
            //closing the write end of prdxs[j] pipe because we only have to read
            close(prdxs[j][1]);
            //reading bounds of delegators
            int llb = read(prdxs[j][0],&lb,sizeof(int));
            int uub = read(prdxs[j][0],&ub,sizeof(int));
            close(prdxs[j][0]);
            // cerr<<"\n\n"<<lb<<"\n\n"<<ub<<"\n\n";
            //initialising varaibles for bounds of worker proceses
            int wlb=lb;
            int wub=lb-1;
            int k;
            //creating pipe for transfer of primes and times of workers from workers to delegator
            int px[2];
            int pt[2];
            //array of pipes so delegator node can send bounds to all workers
            int pb[nums][2];
            //pipe for transfer of primes from worker to delegator
            if(pipe(px)<0)
            {
                cerr<<"Pipe error";
                exit(EXIT_FAILURE);
            }
            //pipe for transfer of times from worker to delegator
            if(pipe(pt)<0)
            {
                cerr<<"Pipe error";
                exit(EXIT_FAILURE);
            }
            //loop to fork all workers
            for(k=0; k<nums;k++)
            {   
                //creating pipe for delegator->worker transfer for the kth child node
                pipe(pb[k]);
                //forking the elegaotr to create a worker process
                pc[k]=fork();
                if(pc[k]<0)
                {
                    cerr<<"Fork error";
                    exit(EXIT_FAILURE);
                }
                //deciding type (primes1 or primes2) of worker in a circular manner
                type=((j*nums)+k)%2;
                //worker node
                if(pc[k]==0)
                {
                    //reading assigned lower bound and upper bound from delegator process
                    close(pb[k][1]);
                    //converting to string to pass to exec
                    char str1[30];
                    char str2[30];
                    char str3[30];
                    char str4[30];
                    int len1=read(pb[k][0],&lb,sizeof(lb));
                    int len2=read(pb[k][0],&ub,sizeof(ub));
                    snprintf(str1, sizeof(str1), "%d", lb);
                    snprintf(str2, sizeof(str2), "%d", ub);
                    snprintf(str3, sizeof(str3), "%d", batch);
                    snprintf(str4, sizeof(str4), "%d", root_pid);
                    close(pb[k][0]);

                    //cerr<<str1<<"\n\n\n"<<str2<<"\n";
                 
                    close(px[0]);
                    close(pt[0]);

                    //duplicating the fd to WRITE_FD
                    if(dup2(px[1],WRITE_FD)==-1)
                    {
                        cerr<<"dup2 error";
                        exit(EXIT_FAILURE);
                    }
                    if(dup2(pt[1],WRITE_FDX)==-1)
                    {
                        cerr<<"dup2 error";
                        exit(EXIT_FAILURE);
                    }

                    //calling relevant executable depending on type(primes1 or primes2)
                    //run primes1
                    if(type==0)
                    {
                        execlp("./primes1","primes1",str1,str2,str3,str4,NULL);
                    }
                    else //run primes2
                    {
                        execlp("./primes2","primes2",str1,str2,str3,str4,NULL);
                    }
                    break;
                }
                else
                {   
                    //writing to workers
                    close(pb[k][0]);
                    if(re==0) //dividing tasks equally
                    {
                        //finding ranges for all worker nodes and sending them through pipes
                        wlb=lb+k*((ub-lb+1)/nums);
                        wub=wlb+((ub-lb+1)/nums)-1;
                        if(wub>ub || k==nums-1)
                        {
                            write(pb[k][1],&wlb,sizeof(int));
                            write(pb[k][1],&ub,sizeof(int));
                        }
                        else
                       { write(pb[k][1],&wlb,sizeof(int));
                        write(pb[k][1],&wub,sizeof(int));}
                    }
                    else //dividing tasks randomly
                    {
                        //sets lower bound of worker to 1 more than upper bound of previous worker
                        wlb=wub+1;
                        //if its the last worker under given delegator we set upper bound of it to upper bound of delegator
                        if(k==nums-1)
                            wub=ub;
                        else
                            wub=wlb+rand()%(ub-wlb-(nums-k-1)+1);
                        
                        write(pb[k][1],&wlb,sizeof(int));
                        write(pb[k][1],&wub,sizeof(int));
                    }
                    close(pb[k][1]);
                }
            }
            //delegator node
            if(k==nums)
            {
                close(px[1]);
                close(pt[1]);
                pid_t pid;
                int status = 0;
                //loop to wait till everything has been read from the pipe which transfers primes
                while (1)
                {
                    int number;
                    //read from READ end 
                    int check=read(px[0], &number, sizeof(int));
                    if(check==0)
                        break;
                    // cout<<"HELLO\n\n"<<number<<"\n";
                    //write the prime to root
                    write(prdx[1],&number,sizeof(int));

                }
                close(px[0]);
                //loop to get all the times from the pipes
                while (1)
                {
                    long double time;
                    //read from READ end 
                    int check=read(pt[0], &time, sizeof(long double));
                    //cout<<time<<"\n";
                    if(check==0)
                        break;
                    // cout<<"HELLO\n\n"<<number<<"\n";
                    //write the time to root
                    write(prdxt[1],&time,sizeof(long double));

                }
                close(pt[0]);
                close(prdx[1]);
                close(prdxt[1]);
                while ((pid = wait(&status)) != -1);
                exit(0);
            }
            break;
        }
        else
        {
            //sending all delegator nodes their ranges to find primes in through a pipe
            close(prdxs[j][0]);
            if(re==0)
            {lb=lbound+j*((ubound-lbound+1)/nums);
            ub=lb+((ubound-lbound+1)/nums)-1;
            // cerr<<"\n\n\n"<<lb<<"\n"<<ub<<"\n\n";
            if(ub>ubound || j==nums-1)
            {
                write(prdxs[j][1],&lb,sizeof(lb));
                write(prdxs[j][1],&ubound,sizeof(ubound));
            }
            else
           { write(prdxs[j][1],&lb,sizeof(lb));
            write(prdxs[j][1],&ub,sizeof(ub));}
            }
            else //dividing tasks randomly
                    {   

                        //sets lower bound of delegator to 1 more than upper bound of previous delegator
                        lb=ub+1;
                        //if its the last worker under given delegator we set upper bound of it to upper bound of delegator
                        if(j==nums-1)
                            ub=ubound;
                        else
                            ub=lb+rand()%(ubound-lb-(nums-(j+1))+1);
                        write(prdxs[j][1],&lb,sizeof(int));
                        write(prdxs[j][1],&ub,sizeof(int));
                    }
            close(prdxs[j][1]);
        }
    }
    //root node
    if(j==nums)
    {
        close(prdx[1]);
        close(prdxt[1]);
        pid_t pid;
        int status = 0;
        int count=0;

        //variables to calculate statistics for time

        long double countt=0;
        long double min_time=1000;
        long double max_time=0;
        long double sum_time=0;

        //allocating an array on the heap to store primes that we get from delegators
        int primes[ubound-lbound+1];

        //loop to wait for all children
        //as long as parent still has children to wait for (wait not returning -1)
        //then we will do the following statements
        while (1)
            {
                int prime;
                //read primes from delegators and put them in an array
                int check= read(prdx[0], &prime, sizeof(prime));
                if(check==0)
                    break;
                primes[count]=prime;
                // cout<<prime<<" ";
                count++;
                //add just read prime to array of primes
            }

        while (1)
            {
                long double time;
                //read time from delegators
                int checkt= read(prdxt[0], &time, sizeof(long double));
                // cout<<time<<"\n";
                if(checkt==0)
                    break;
                //setting the minimum and maximum times
                if (time<min_time)
                    min_time=time;
                else if(time>max_time)
                    max_time=time;
                //adding up all the times reported thus far
                sum_time+=time;
                countt++;

            }
        long double avg_time=sum_time/countt;
        //closign reads ends of pipes used to get primes, and times from the delegators
        close(prdx[0]);
        close(prdxt[0]);
        //ensuring all child processes are finished
        while ((pid = wait(&status)) != -1);
        //sort the array of primes

        //outputting the required data
        cout<<"The number of SIGUSR1 received are:"<<numsig1<<"\n";
        cout<<"The number of SIGUSR2 received are:"<<numsig2<<"\n";
        cout<<"The minimum running time of a worker node:"<<min_time<<"\n";
        cout<<"The maximum running time of a worker node:"<<max_time<<"\n";
        cout<<"The average running time of a worker node:"<<avg_time<<"\n";
        cout<<"The primes are:\n";
        //sort primes
        quick_sort(primes,0,count-1);
        //print primes
        for(int c=0;c<count;c++)
            cout<<primes[c]<<" ";
        cout<<"\n";
        exit(0);

}
}
//function to handle the received SIGUSR1 and SIGUSR2 signals
void handler( int signum ){ 
    //if it is SIGUSR1, increase global variable numsig1 by 1, otherwise increase numsig2 by 1
    if(signum==SIGUSR1)
	{    
        numsig1++;
        signal(SIGUSR1,handler);
    }
    else if(signum==SIGUSR2)
	{    
        numsig2++;
        signal(SIGUSR2,handler);
    }
}