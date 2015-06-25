/* forkTest.c
	This test creates and forks two threads within a process. The two threads
	are threads A and B, and will print the corresponding statements once the 
	thread is forked. The ThreadAfunc and ThreadBfunc print out a statement to
	confirm that the threads were properly forked and then will call the exit
	syscall function. 
 */

#include "syscall.h"


void ThreadAfunc(){
	Printf("Fort Test Fucntion for thread A\n", 32, 0,0);
	Exit(0);
}

void ThreadBfunc(){
	Printf("Fort Test Fucntion for thread B\n", 32, 0,0);
	Exit(0);
}

int main(){
	
    Fork(ThreadAfunc, "Thread A", 8);
    Fork(ThreadBfunc, "Thread B", 8);

}
