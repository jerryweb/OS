/* matmultFork
	fork 2 matmults
*/

#include "syscall.h"

#define Dim 	20	/* sum total of the arrays doesn't fit in physical memory */

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];

int D[Dim][Dim];
int E[Dim][Dim];
int F[Dim][Dim];

int G[Dim][Dim];
int H[Dim][Dim];
int I[Dim][Dim];

void
matmult1(int n)
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A[i][j] = i;
	     B[i][j] = j;
	     C[i][j] = 0;
	}

    for (i = 0; i < Dim; i++)		/* then multiply them together */
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 C[i][j] += A[i][k] * B[k][j];

    Exit(C[Dim-1][Dim-1]);		/* and then we're done */
}
void
matmult2(int n)
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     D[i][j] = i;
	     E[i][j] = j;
	     F[i][j] = 0;
	}

    for (i = 0; i < Dim; i++)		/* then multiply them together */
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 F[i][j] += D[i][k] * E[k][j];

    Exit(F[Dim-1][Dim-1]);		/* and then we're done */
}
/*
void
matmult3(int n)
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices
	for (j = 0; j < Dim; j++) {
	     G[i][j] = i;
	     H[i][j] = j;
	     I[i][j] = 0;
	}

    for (i = 0; i < Dim; i++)		/* then multiply them together
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 I[i][j] += G[i][k] * H[k][j];

    Exit(I[Dim-1][Dim-1]);		/* and then we're done
}
*/

int main()
{
    Printf("Starting main\n", 14, 0, 0);
    Printf("Forking thread 1\n", 17, 0, 0);
    Fork(matmult1, "1", 1);
    Printf("Done forking thread 1\n", 22, 0, 0);
    Printf("Forking thread 2\n", 17, 0, 0);
    Fork(matmult2, "2", 1);
    Printf("Done forking thread 2\n", 22, 0, 0);
    Printf("Finishing main\n", 15, 0, 0);
    /*Fork(matmult3, "3", 1);*/
}
