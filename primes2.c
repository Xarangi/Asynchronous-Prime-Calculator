# include <stdio.h>
# include <stdlib.h>
# include <math.h>
#include <signal.h>
#include<sys/times.h>
#include<unistd.h>

# define YES 1
# define NO 0
#define WRITE_FD 1310
#define WRITE_FDX 1311

//function to determine if a given integer is prime
int prime (int n){
    int i=0 , limitup =0;
    limitup = (int)(sqrt((float)n));
    if (n ==1) return (NO);
        for (i=2 ; i <= limitup ; i ++)
            if ( n % i == 0) return (NO);
    return ( YES );
    }
//main function
int main ( int argc , char * argv []) {
    int lb =0 , ub =0 , i=0;
    //initialising variables to calculate time taken
    long double time;
    long double t1,t2,ticspersec;
    struct tms tb1,tb2;
    // printf("\n\n\n\n%s\n\n\n",argv[1]);
    ticspersec = (long double ) sysconf ( _SC_CLK_TCK );
    t1 = (long double ) times (& tb1);
    if ( ( argc< 3) ) {
        // printf (" usage : prime1 lb ub\n") ;
        exit (1) ; }
    //getting lower and upper bounds from the arguments passed in execlp
    lb= atoi ( argv [1]) ;
    ub= atoi ( argv [2]) ;
    if ( ( lb <0 ) || ( lb > ub ) ) {
        // printf (" usage : prime1 lb ub\n");
        // printf("\n%d %d\n",lb,ub);
        exit (1) ; }
    //loop going through all the numbers in range and checking if they are prime
    for (i=lb ; i <= ub ; i ++)
        if ( prime (i) == YES )
        {
            write(WRITE_FD,&i,sizeof(int));
            // printf("%d\n",i);
        }
    //closing file descriptor, sending signal
    close(WRITE_FD);
    int checker;
    if(atoi(argv[3])==0)
        checker=kill(atoi(argv[4]),SIGUSR1);
    else
        checker=kill(atoi(argv[4]),SIGUSR2);
    if(checker==-1)
    {
        // printf("Signal not sent");
        printf("%s",argv[4]);
    }
    //storing time after the process, writing it to the pipe
    t2 = (long double) times (& tb2) ;
    time=(t2-t1)/ticspersec;
    write(WRITE_FDX,&time,sizeof(long double));
    close(WRITE_FDX);
    exit(0);
    
}