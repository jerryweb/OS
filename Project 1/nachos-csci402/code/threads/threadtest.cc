// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "passenger.h"
#include "liaison.h"
#include "checkin.h"
#include "cargo.h"

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

//----------------------------------------------------------------------
//----------------------------------------------------------------------
void StartFindShortestLiaisonLine(int arg){
	Passenger* p = (Passenger*)arg;
	p->findShortestLiaisonLine();
}

//----------------------------------------------------------------------
// The passenger should find the shortest economy class line because he is in 
// econ class.
//----------------------------------------------------------------------
void StartFindShortestCISLine(int arg){
	Passenger* p = (Passenger*)arg;
	p->findShortestCheckinLine();
}

void StartCargoTest(int arg){
	Cargo* c = (Cargo*)arg;
	c->Run();
}

void StartCheckInTest(int arg){
	CheckIn* ci = (CheckIn*)arg;
	Passenger* p = ci->FindPassenger(0);
}

//----------------------------------------------------------------------
//These are the initial print statements needed at the beginning of each simulation
//The parameters should all be lists of each thread. 
//----------------------------------------------------------------------
void StartupOutput(List* pList){
	//printf("Number of airport liaisons = %d", liaisonList.Size());
	//printf("Number of airlines = %d", airlineList.Size());
	//printf("Number of check-in staff = %d", checkInStaffList.Size());
	//printf("Number of cargo handlers = %d", cargoHandlersList.Size());
	//printf("Number of screening officers = %d", screeningOfficersList.Size());
	printf("Total number of passengers = %d\n", pList->Size());

	//printf("Numner of passengers for airline[code goes here] = %d", );
	for(int i = 0; i < pList->Size(); i++){
		Passenger *P = (Passenger*)pList->First();
		List *bags = P->getLuggage();
		int tempBagWeights[3] = {0,0,0};				//This helps list the weights of the bags
		printf("Passenger %d belongs to airline %d\n", P->getID(),P->getTicket().airline);

		for(int j = 0; j < 3; j++){						//This calculates the weights of each of the bags 
			Luggage *l = (Luggage*)bags->First();		//and puts it into a temp array to be read
			tempBagWeights[j] = l->weight;
			bags->Remove();
		}
		printf("Passenger %d : Number of bags = %d\n", P->getID(), P->getLuggage()->Size());
		printf("Passenger %d : Weight of bags = %d, %d, %d\n", P->getID(), tempBagWeights[0],
		 tempBagWeights[1], tempBagWeights[2]);
		
	}

}

//----------------------------------------------------------------------
//Passenger Find Shortest Line Test
// This is the test to show that the passenger chooses the correct line. If the ticket is 
// economy class, then the passenger will pick line 0, otherwise, he will pick the shortest
// of the other 6 lines. The values below are hard coded and can be changed here.
//----------------------------------------------------------------------
void PassengerFindsShortestLiaisonLine(){
	//This is the initialization of a passenger with the following secifications
	int checkInStaffList[5] = {1,4,0,2,0};
	int liasionList[7] = {3, 2, 5, 8, 1, 6, 9};  			//there are 7 airport liaisons
	
	Airport *airport = new Airport();					//This creates a new airpost object with all of the 
														//global variables listed here
	List* bagList = new List();							//List of passenger's luggage
	// List* passengerList = new List();

	for(int i =0; i <3; i++){
		Luggage *bag = new Luggage;	
		bag->airlineCode = 1;
		bag->weight = 45 + i;							 //weight ranges from 45 -47lbs
		bagList->Append((void *)bag);
	}

	Ticket ticket;
	ticket.airline = 1;
	ticket.executive = false;

	Liaison *L = new Liaison(0,airport);

	Passenger *p = new Passenger(0, bagList, ticket, airport->liaisonQueues, liasionList, checkInStaffList);
	//passengerList->Append((void *)p);
	airport->passengerList->Append((void *)p);

	StartupOutput(airport->passengerList);
	//Beginning of shortest line test and start of critical section for finding shortest line
	Thread *t = new Thread("Passenger");
	Thread *tLiaison = new Thread("Liaison");
	t->Fork(StartFindShortestLiaisonLine,(int(p)));
	//tLiaison->Fork(directPassengers,(int(L)));
}


//----------------------------------------------------------------------
//	This test will check to make sure that the passenger enters the shortest economy class line
// 	This function initializes the data and forks the new thread 
//----------------------------------------------------------------------
void PassengerFindsShortestCISEconomyLine(){
	int checkInStaffList[5] = {3,7,2,1,5};
	int liasionList[7] = {3, 2, 5, 8, 1, 6, 9};  			//there are 7 airport
	
	List* newLList[7];

	for(int i =0; i <7; i++){
		List* liaisonLine = new List();
		newLList[i] = liaisonLine;
	}	

	Airport *airport = new Airport();					//This creates a new airpost object with all of the 
														//global variables listed here
	List* bagList = new List();							//List of passenger's luggage
	// List* passengerList = new List();

	for(int i =0; i <3; i++){
		Luggage *bag = new Luggage;	
		bag->airlineCode = 1;
		bag->weight = 45 + i;							 //weight ranges from 45 -47lbs
		bagList->Append((void *)bag);
	}

	Ticket ticket;
	ticket.airline = 2;
	ticket.executive = false;								//this makes the passenger economy class

	Passenger *p = new Passenger(1, bagList, ticket, airport->liaisonQueues, liasionList, checkInStaffList);
	airport->passengerList->Append((void *)p);

	StartupOutput(airport->passengerList);
	Thread *t = new Thread("Passenger");
	t->Fork(StartFindShortestCISLine,(int(p)));
}

//----------------------------------------------------------------------
//	CheckInTest
// 	 Adds 3 passengers into the check-in queue:
//    id 0, executive (line 0)
//    id 1, economy (line 1)
//    id 2, economy (line 1)
//   Initializes 1 check-in thread (id 1; airline 0) and runs it.
//----------------------------------------------------------------------
void CheckInTest()
{
    Airport* airport = new Airport(); // 3 airlines
    Passenger* p0 = new Passenger(0);
    Passenger* p1 = new Passenger(1);
    Passenger* p2 = new Passenger(2);
    airport->checkinQueues[0]->Append(p0);
    airport->checkinQueues[1]->Append(p1);
    airport->checkinQueues[1]->Append(p2);
    CheckIn* ci = new CheckIn(0, 1, airport);
    Thread* t = new Thread("CheckIn");
    t->Fork(StartCheckInTest, (int)ci);
}

//----------------------------------------------------------------------
//	CargoTest
// 	 Adds 7 bags onto the conveyor:
//    airline 0, weight 30
//    airline 1, weight 42
//    airline 2, weight 54
//    airline 0, weight 30
//    airline 1, weight 45
//    airline 0, weight 45
//    airline 1, weight 60
//   Initializes 6 cargo (0-5) threads and runs them.
//----------------------------------------------------------------------
void CargoTest()
{
    Airport* airport = new Airport(); // 3 airlines
    int weight, i, j;
    for (i = 0; i < 3; i++)
    {
        Luggage* bag = new Luggage;
        bag->airlineCode = i;
        bag->weight = 30+12*i;
        airport->conveyor->Append((void*)bag);
    }
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            Luggage* bag = new Luggage;
            bag->airlineCode = j;
            bag->weight = 30+15*(i+j);
            airport->conveyor->Append((void*)bag);
        }
    }
    
    Cargo* cargo0 = new Cargo(0, airport);
    Cargo* cargo1 = new Cargo(1, airport);
    Cargo* cargo2 = new Cargo(2, airport);
    Cargo* cargo3 = new Cargo(3, airport);
    Cargo* cargo4 = new Cargo(4, airport);
    Cargo* cargo5 = new Cargo(5, airport);
    
	Thread* t0 = new Thread("Cargo0");
	Thread* t1 = new Thread("Cargo1");
	Thread* t2 = new Thread("Cargo2");
	Thread* t3 = new Thread("Cargo3");
	Thread* t4 = new Thread("Cargo4");
	Thread* t5 = new Thread("Cargo5");
    
	t0->Fork(StartCargoTest, (int)cargo0);
	t1->Fork(StartCargoTest, (int)cargo1);
	t2->Fork(StartCargoTest, (int)cargo2);
	t3->Fork(StartCargoTest, (int)cargo3);
	t4->Fork(StartCargoTest, (int)cargo4);
	t5->Fork(StartCargoTest, (int)cargo5);
}
