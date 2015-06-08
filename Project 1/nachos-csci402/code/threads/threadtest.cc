// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
// #include "passenger.h"
// #include "liaison.h"
// #include "cargo.h"
// #include "airportobjects.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 2; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

/*
//----------------------------------------------------------------------
//Passenger Find Shortest Line Test
// This is the test to show that the passenger chooses the correct line. If the ticket is 
// economy class, then the passenger will pick line 0, otherwise, he will pick the shortest
// of the other 6 lines. The values below are hard coded and can be changed here
//----------------------------------------------------------------------
void PassengerFindsShortestLiaisonLine(){
	//This is the initialization of a passenger with the following secifications
	Lock *LineLock = new Lock("line lock");
	Condition* lineCV[7];

	int liasionList[7] = {3, 2, 5, 8, 1, 6, 9};  			//there are 7 airport
															//liasions including 
															//executive
	Luggage luggage[3];										//3 bags 

	for(int i =0; i <3; i++){
		luggage[i].airlineCode = 1;
		luggage[i].weight = 45 + i;							 //weight ranges from 45 -47lbs
	}

	Ticket ticket;
	ticket.airline = 1;
	ticket.executive = false;
	int passengerID = 0;

	Passenger *passenger = new Passenger(0, luggage, ticket);

	//Beginning of shortest line test and start of critical section for finding shortest line
	LineLock->Aquire();
		
		int myLine, lineSize = 0; 				//These are the line and its size that the passenger enters
		// int lineSize =0// = LineCount[0];
		myLine = passenger->findShortestLiaisonLine(liasionList);		//function to find shortest line
		lineSize = liasionList[myLine];

		//Should be the first print statment 
		printf("Passenger %d chose liaison %d with a line length of %d\n", Passenger.getID(), myLine, lineSize);

		if(lineSize > 0)
			lineCV[myLine]->Wait(LineLock);
		

	LineLock->Release();
	//end of passenger crtitical section 

	//CreatePassenger(liasionList);

}
*/