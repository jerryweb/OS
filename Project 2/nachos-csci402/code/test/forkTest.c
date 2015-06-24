/* forkTest.c
	This test creates and forks two threads within a process. The two threads
	are threads A and B, and will print the corresponding statements once the 
	thread is forked
 */

#include "syscall.h"


void ThreadAfunc(){
	int var = 0;
	Printf("Fort Test Fucntion for thread A\n", 32, 0,0);
	Exit(0);
}


void ThreadBfunc(){
	int var2 = 0;
	Printf("Fort Test Fucntion for thread B\n", 32, 0,0);
	Exit(0);
}


int
main()
{
	
    Fork(ThreadAfunc, "Thread A", 8);

    Fork(ThreadBfunc, "Thread B", 8);

}
