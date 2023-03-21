#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include<string.h>
#include <sys/wait.h>

//making a file descriptor where on which we will duplicate so we can continue using it after exec
#define WRITE_FD 1310
#define WRITE_FDX 1311

//program to handle received signals
void handler(int);

//programs for sorting the primes
int partition(int array[], int left, int right);
void quick_sort(int array[], int left, int right);
