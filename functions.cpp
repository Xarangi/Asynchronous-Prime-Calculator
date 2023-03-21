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
int partition(int array[], int left, int right)
{
	int pivot=array[right];
	int temp;
	int p=left-1;
	for (int i=left;i<right;i++)
	{
		if(array[i]<pivot)
		{
			p++;
			temp= array[p];
			array[p]=array[i];
			array[i]=temp;

		}
	}
	temp=array[p+1];
	array[p+1]=pivot;
	array[right]=temp;
	return p+1;
}
void quick_sort(int array[], int left, int right)
{
	if(left<right)
	{int p=partition(array,left,right);
	quick_sort(array,left,p-1);
	quick_sort(array,p+1,right);}
}
