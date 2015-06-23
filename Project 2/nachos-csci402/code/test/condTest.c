/* cond.c
 *	Simple program to test whether the Condition syscalls work.
 */

#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    int goodCondID, badCondID;
    /* Create with bad name string: ID -1 */
    badCondID = CreateCondition((char*)10, 10);
    Printf("Condition ID = %d\n", 13, 1, badCondID);
    /* Create with good name string */
    goodCondID = CreateCondition("Test", 4);
    Printf("Condition ID = %d\n", 13, 1, goodCondID);
    
    /* wait, signal, broadcast - need multithreading to do this */
    
    /* Destroy bad cond IDs - will print errors */
    DestroyCondition(badCondID);
    DestroyCondition(goodCondID + 1);
    /* Destroy good cond ID */
    DestroyCondition(goodCondID);
}
