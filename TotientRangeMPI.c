// TotientRanceMPI.c - Parallel Euler Totient Function (C Version with MPI)
// compile: mpicc -Wall -g -O2 -o test1 TotientRangeMPI.c
// run:     mpirun -N 2 ./test1 lower_num uppper_num

// Original Sequential C version: Greg Michaelson 14/10/2003
// Patrick Maier   29/01/2010 [enforced ANSI C compliance]

// This program calculates the sum of the totients between a lower and an 
// upper limit using C longs and MPI. It is based on earlier work by:
// Phil Trinder, Nathan Charles, Hans-Wolfgang Loidl and Colin Runciman

#include <stdio.h>
#include <mpi.h>

// hcf x 0 = x
// hcf x y = hcf y (rem x y)

long hcf(long x, long y)
{
  long t;

  while (y != 0) {
    t = x % y;
    x = y;
    y = t;
  }
  return x;
}


// relprime x y = hcf x y == 1

int relprime(long x, long y)
{
  return hcf(x, y) == 1;
}


// euler n = length (filter (relprime n) [1 .. n-1])

long euler(long n)
{
  long length, i;

  length = 0;
  for (i = 1; i < n; i++)
    if (relprime(n, i))
      length++;
  return length;
}


// sumTotient lower upper = sum (map euler [lower, lower+1 .. upper])

long sumTotient(long lower, long upper)
{
  long sum, i;

  sum = 0;
  for (i = lower; i <= upper; i++)
    sum = sum + euler(i);
  return sum;
    

}


int main(int argc, char *argv[])
{
  long lower = 1, upper,localSum,totSum,newl,newu;
    int my_rank, p, quotient, remainder;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    
    double start = MPI_Wtime();
    
    
    
  if (argc != 2) {
    printf("Only one argument needed\n");
    return 1;
  }
 
  sscanf(argv[1], "%ld", &upper);
  quotient = (upper - lower)/p;
  remainder = (upper - lower) % p;
   
  newl = quotient * my_rank + 1;
  newu = quotient * (my_rank+1);
  if(my_rank != p - 1){
    localSum = sumTotient(newl,newu);
    printf("%d C: Sum of Totients  between [%ld..%ld] is %ld\n",my_rank,
           newl, newu, localSum);
    MPI_Send(&localSum,1,MPI_LONG,0,0,MPI_COMM_WORLD);
  }
  else{
	localSum = sumTotient(newl,newu+remainder+1);
    printf("%d C: Sum of Totients  between [%ld..%ld] is %ld\n",my_rank,
               newl, newu, localSum);
    MPI_Send(&localSum,1,MPI_LONG,0,0,MPI_COMM_WORLD);
  }
  if(my_rank == 0){
        totSum = localSum;
        for (int i=1; i<p; i++){
            MPI_Recv(&localSum,1,MPI_LONG,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            totSum += localSum;
        }
        double end = MPI_Wtime();

        printf("C: Sum of Totients  between [%ld..%ld] is %ld, time is %.8f\n",
               lower, upper, totSum, end-start);
        

        
    }
    
  
    MPI_Finalize();
  return 0;
}
