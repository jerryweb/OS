/* forkTest.c

 */

#include "syscall.h"


void testFuction(){
	int hello = 0;
	Printf("Fort Test Fucntion\n", 19, 0,0);
}

int
main()
{
	
    Fork(testFuction, "funk", 4);

}
