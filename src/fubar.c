// Simple C program to display "Hello World"
  
// Header file for input output functions
#include <stdio.h>
  
// main function -
// where the execution of program begins
int main()
{
  
    // prints hello world
    printf("Hello World\n");
	
    printf("char %lld\n", sizeof(char));
    printf("int %lld\n", sizeof(int));
    printf("float %lld\n", sizeof(float));
    printf("double %lld\n", sizeof(double));

double x=3.14;
double *ptr1=&x; 
double **ptr2=&ptr1;
printf("The size of pointer 1 is %lld bytes and pointer 2 is %lld bytes\n",sizeof(ptr1),sizeof(ptr2));	

    return 0;
}