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

void StartManagerTest(int arg){

}

//----------------------------------------------------------------------
// The passenger should find the shortest liaison line 
//----------------------------------------------------------------------
void StartFindShortestLiaisonLine(int arg){
	Passenger* p = (Passenger*)arg;
	p->findShortestLiaisonLine();
}

void StartFindCorrectCISLine(int arg){
	Passenger* p = (Passenger*)arg;
	p->CheckIn();
}

void StartLiaisonThread(int arg){
	Liaison* L = (Liaison*)arg;
	L->DirectPassengers();
}

void StartCargo(int arg){
	Cargo* c = (Cargo*)arg;
	c->StartCargo();
}

void StartCheckInTest(int arg){
	CheckIn* ci = (CheckIn*)arg;
	Passenger* p = ci->FindPassenger(0);
}

void StartCheckInStaff(int arg){
	CheckIn* ci = (CheckIn*)arg;
	ci->StartCheckInStaff();
}

//----------------------------------------------------------------------
//These are the initial print statements needed at the beginning of each simulation
//The parameters should all be lists of each thread. Assumption that [Count] in the 
// output guidelines do not include the brackets in the output.
//----------------------------------------------------------------------
void StartupOutput(Airport* airport){
	printf("Number of airport liaisons = %d\n", airport->liaisonList->Size());
	printf("Number of airlines = %d\n", airport->numAirlines);
	printf("Number of check-in staff = %d\n", airport->checkInStaffList->Size());
	printf("Number of cargo handlers = %d\n", airport->cargoHandlerList->Size());
	//printf("Number of screening officers = %d\n", screeningOfficersList.Size());
	printf("Total number of passengers = %d\n", airport->passengerList->Size());

	int passengersPerAirline[3] = {0,0,0};
	//Static counter for number of passengers per airline for 3 airlines
	for(int i = 0; i < airport->passengerList->Size(); i++){
		Passenger *P = (Passenger*)airport->passengerList->First();
		airport->passengerList->Remove();
		airport->passengerList->Append((void *)P);	

		if(P->getTicket().airline == 0){
			passengersPerAirline[0]++;
		}
		else if(P->getTicket().airline == 1){
			passengersPerAirline[1]++;
		}
		else if(P->getTicket().airline == 2){
			passengersPerAirline[2]++;
		}
	}
	
	for(int h = 0; h < airport->numAirlines; h++){
			printf("Number of passengers for airline %d = %d\n", 
				airport->airlines[h]->id, passengersPerAirline[h]);
	}

	//This prints out the number bags and their weights for each passenger 
	for(int i = 0; i < airport->passengerList->Size(); i++){
		Passenger *P = (Passenger*)airport->passengerList->First();
		airport->passengerList->Remove();
		airport->passengerList->Append((void *)P);			//Prevent destruction of global passenger list	
		List *bags = P->getLuggage();						//Temp list for iterating through luggage
		int tempBagWeights[3] = {0,0,0};					//This helps list the weights of the bags
		
		printf("Passenger %d belongs to airline %d\n", P->getID(),P->getTicket().airline);

		for(int j = bags->Size(); j > 0; j--){				//This calculates the weights of each of the bags 
			Luggage *l = (Luggage*)bags->First();			//and puts it into a temp array to be read
			tempBagWeights[3 - j] = l->weight;
			bags->Remove();
			bags->Append((void *)l);					//Prevent destruction of local bags list
		}
		
		printf("Passenger %d : Number of bags = %d\n", P->getID(), P->getLuggage()->Size());
		printf("Passenger %d : Weight of bags = %d, %d, %d\n", P->getID(), tempBagWeights[0],
		 tempBagWeights[1], tempBagWeights[2]);	
	}

	//Prints Check-in Staff information
	for(int k = 0; k < airport->checkInStaffList->Size(); k++){
		CheckIn *CIS = (CheckIn*)airport->checkInStaffList->First();
		airport->checkInStaffList->Remove();
		airport->checkInStaffList->Append((void *)CIS);
		printf("Airline check-in staff %d belongs to airline %d\n", CIS->getID(), CIS->getAirline());
	}
}

//----------------------------------------------------------------------
// This runs a full simulation of the airport interactions in order to test the manager
// 10 passengers, 7 liaisons, 3 airlines each with 5 check-in staff, and 10 cargo handlers
// are created at the beginning of the test.
//----------------------------------------------------------------------
void ManagerTest(){
	Airport *airport = new Airport();
	//These are the arrays that will hold the thread pointers, which are to be forked 
	List* PassengerThreadArray = new List();
	List* LiaisonThreadArray = new List();
	List* CheckInStaffThreadArray =  new List();
	List* CargoHandlerTreadArray = new List();

	//Generate Passengers each with seperate luggage and tickets 
	for(int i = 0; i < 10; i++){
		List* bagList = new List();

		for(int j =0; j <3; j++){
			Luggage *bag = new Luggage;	
			bag->airlineCode = 2;
			bag->weight = (30 + i*2 + j) % 60;				//weight ranges from 30 to 60lbs
			bagList->Append((void *)bag);
		}

		//Varies the airline codes and executive class status for the passengers
		Ticket ticket;
		ticket.airline = i % 3 ;
		if(i % 3 == 2)
			ticket.executive = true;
		else
			ticket.executive = false;

		Passenger *p = new Passenger(i, bagList, ticket, airport);
		airport->passengerList->Append((void *)p);
		Thread *t = new Thread("Passenger");
		PassengerThreadArray->Append((void *)t);
	}

	//Generates Liaisons
	for(int k = 0; k < 7; k++){
		Liaison *L = new Liaison(k,airport);
		airport->liaisonList->Append((void *)L);
		Thread *tL = new Thread("Liaison");
		LiaisonThreadArray->Append((void *)tL);
	}

	//Generates Check-in Staff; there are 5 for each airline
	for(int m = 0; m < 3; m++){
		for(int n = 0; n <5; n++){
			CheckIn *C = new CheckIn(m, n, airport);
    		airport->checkInStaffList->Append((void *)C);
    		Thread  *tC =  new Thread("CheckInStaff");
    		CheckInStaffThreadArray->Append((void *)tC);
		}
	}

	//Generates Cargo Handlers; there should be 10 in total 
	for(int q = 0; q < 10; q++){
		Cargo* cargo = new Cargo(q, airport);
		airport->cargoHandlerList->Append((void *)cargo);
		Thread *tCH = new Thread("Cargo_Handler");
		CargoHandlerTreadArray->Append((void *)tCH);
	}
    
    //Display initial airport data
	StartupOutput(airport);

	//Fork all of the Passenger Threads from the array of passenger threads
	for(int i = PassengerThreadArray->Size(); i > 0 ; i--){
		Passenger *P = (Passenger*)airport->passengerList->First();
		airport->passengerList->Remove();
		airport->passengerList->Append((void *)P);
		Thread *t = (Thread*)PassengerThreadArray->First();
		PassengerThreadArray->Remove();
		t->Fork(StartFindShortestLiaisonLine,(int(P)));
	}

	//Fork all of the Liaison Threads from the array of liaison threads
	for(int j = LiaisonThreadArray->Size(); j > 0 ; j--){
		Liaison *L = (Liaison*)airport->liaisonList->First();
		airport->liaisonList->Remove();
		airport->liaisonList->Append((void *)L);
		Thread *tL = (Thread*)LiaisonThreadArray->First();
		LiaisonThreadArray->Remove();
		tL->Fork(StartLiaisonThread,(int(L)));
	}

	//Fork all of the Check-in Staff Threads from the array of Check-in Staff threads
	for(int k = CheckInStaffThreadArray->Size(); k > 0; k--){
		CheckIn *CIS = (CheckIn*)airport->checkInStaffList->First();
		airport->checkInStaffList->Remove();
		airport->checkInStaffList->Append((void *)CIS);
		Thread *tCIS = (Thread*)CheckInStaffThreadArray->First();
		CheckInStaffThreadArray->Remove();
		tCIS->Fork(StartCheckInStaff,(int(CIS)));
	}

	//Fork all of the Cargo Handlers Threads from the array of Cargo Handlers threads
	for(int m = CargoHandlerTreadArray->Size(); m > 0; m--){
		Cargo *CH = (Cargo*)airport->cargoHandlerList->First();
		airport->cargoHandlerList->Remove();
		airport->cargoHandlerList->Append((void *)CH);
		Thread *tCH = (Thread*)CargoHandlerTreadArray->First();
		CargoHandlerTreadArray->Remove();
		tCH->Fork(StartCargo,(int(CH)));
	}
}


//----------------------------------------------------------------------
//Passenger Find Shortest Liaison Line Test
// This is the test to show that the passenger chooses the correct line.
// Liaison Queues 0 through 5 are filled with dummy sizes all greater
// than queue 6 to demonstrate passenger finding shortest queue (size 0)
// Adds dummy passengers into the liaison queues:
//    line 0: 6
//    line 1: 5
//    line 2: 4
//    line 3: 3
//    line 4: 2
//    line 5: 1
// 	  line 6: 0
// Only 1 passenger and 1 liaison (for queue 6) are created to simulate.
// Sets liaison to busy
//   Initializes 1 passenger and 1 liaison thread and runs them:
//    id 0, airline 2 
//	  id 6 
//   Intended result:
//    Passenger 0 will go to line 6 (length 0).
// 	  The Liaison will then direct the passenger to airline 2 checkin 
//    counter.
//----------------------------------------------------------------------
void PassengerFindsShortestLiaisonLine(){
	Airport *airport = new Airport();					//This creates a new airpost object with all of the 
														//global variables listed here
	List* bagList = new List();							//List of passenger's luggage

	for(int i =0; i <3; i++){
		Luggage *bag = new Luggage;	
		bag->airlineCode = 2;
		bag->weight = 45 + i;							 //weight ranges from 45 -47lbs
		bagList->Append((void *)bag);
	}

	// This fills the liaison queues with dummy int variables to simulate line lengths
	for(int i = 5; i >= 0; i--){
		int tempVariable = 5 - i;
		for(int j = 0; j < 6 - i; j++){
			airport->liaisonQueues[i]->Append((void *)tempVariable);
			//printf("Size: %d\n", airport->liaisonQueues[i]->Size());
		}
	}

	Ticket ticket;
	ticket.airline = 2;
	ticket.executive = false;

	// Liaison *L0 = new Liaison(0,airport);
	Liaison *L6 = new Liaison(6,airport);
	// airport->liaisonList->Append((void *)L0);
	airport->liaisonList->Append((void *)L6);

	Passenger *p = new Passenger(0, bagList, ticket, airport);
	//passengerList->Append((void *)p);
	airport->passengerList->Append((void *)p);

	StartupOutput(airport);								//Prints the initial output
	//Beginning of shortest line test and start of critical section for finding shortest line
	Thread *t = new Thread("Passenger");
	// Thread *tL0 = new Thread("Liaison_0");
	Thread *tL6 = new Thread("Liaison_6");

	t->Fork(StartFindShortestLiaisonLine,(int(p)));
	// tL0->Fork(StartLiaisonThread,(int(L0)));
	tL6->Fork(StartLiaisonThread,(int(L6)));
}


//----------------------------------------------------------------------
//	PassengerFindsCorrectCISLine
//   This test only uses airline 0's check-in area.
// 	 Adds dummy passengers into the check-in queue:
//    line 0: 0 (executive line)
//    line 1: 3
//    line 2: 4
//    line 3: 2
//    line 4: 0
//    line 5: 7
//   Sets CIS 4's state to break.
//   Initializes 3 passenger threads and runs them:
//    id 0, economy ticket
//    id 1, executive ticket
//    id 2, economy ticket
//   Intended result:
//    Passenger 0 will go to line 3 (length 2).
//    Passenger 1 will go to line 0 (the executive line).
//    Passenger 2 will go to line 1 (length 3).
//    (note: depending on order, Passengers 0 and 2 may be
//      switched, although I haven't found a value of -rs
//      for which this is true yet.)
//----------------------------------------------------------------------
void PassengerFindsCorrectCISLine()
{
	Airport* airport = new Airport(); // 3 airlines
    
    // Populate the check-in list.
    int cisLineLengths[6] = {0, 3, 4, 2, 0, 7};
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < cisLineLengths[i]; j++)
        {
            Passenger* p = new Passenger();
            airport->checkinQueues[i]->Append(p);
            airport->passengerList->Append(p);
        }
    }
    
    // Put the 4th CIS on break.
    airport->checkinState[4] = CI_BREAK;
    
    // Create tickets.
    Ticket ticket0;
    ticket0.airline = 0;
    ticket0.executive = false;
    Ticket ticket1;
    ticket1.airline = 0;
    ticket1.executive = true;
    Ticket ticket2;
    ticket2.airline = 0;
    ticket2.executive = false;
    
    // Create passenger classes.

    Passenger* p0 = new Passenger(0, ticket0, 0, airport);
    airport->passengerList->Append(p0);
    Passenger* p1 = new Passenger(1, ticket1, 0, airport);
    airport->passengerList->Append(p1);
    Passenger* p2 = new Passenger(2, ticket2, 0, airport);
    airport->passengerList->Append(p2);
    
    // Create threads.
    Thread* t0 = new Thread("Passenger0");
    Thread* t1 = new Thread("Passenger1");
    Thread* t2 = new Thread("Passenger2");
    
    // Fork threads and pass passenger classes.
	t0->Fork(StartFindCorrectCISLine, (int)p0);
	t1->Fork(StartFindCorrectCISLine, (int)p1);
    t2->Fork(StartFindCorrectCISLine, (int)p2);
}

//----------------------------------------------------------------------
//	CheckInTest
// 	 Adds 3 passengers into the airline 0 check-in queue:
//    id 0, executive (line 0)
//    id 1, economy (line 1)
//    id 2, economy (line 1)
//   Initializes 1 CIS thread (id 1; airline 0) and runs it.
//   Intended result:
//    The CIS will choose to help the executive passenger.
//    The economy line will stay at length 2.
//----------------------------------------------------------------------
void CheckInTest()
{
    Airport* airport = new Airport(); // 3 airlines
    
    // Create passenger shells.
    Passenger* p0 = new Passenger(0);
    Passenger* p1 = new Passenger(1);
    Passenger* p2 = new Passenger(2);

    // Add passengers to queues.
    airport->checkinQueues[0]->Append(p0);
    airport->checkinQueues[1]->Append(p1);
    airport->checkinQueues[1]->Append(p2);
    
    // Create CIS class.
    CheckIn* ci = new CheckIn(0, 1, airport);
    airport->checkInStaffList->Append((void *)ci);

    // Create thread.
    Thread* t = new Thread("CheckIn");

    // Fork thread and pass CIS class.
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
//   Intended result:
//    The bags will be added to the proper airplane in the
//     order in which they were added to the conveyor.
//     After all bags are finished, the cargo handlers
//     will all go on break.
//----------------------------------------------------------------------
void CargoTest()
{
    Airport* airport = new Airport(); // 3 airlines
    
    int weight, i, j;
    
    // Create luggage and add to conveyor.
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
    
    // Create cargo handler classes.
    Cargo* cargo0 = new Cargo(0, airport);
    Cargo* cargo1 = new Cargo(1, airport);
    Cargo* cargo2 = new Cargo(2, airport);
    Cargo* cargo3 = new Cargo(3, airport);
    Cargo* cargo4 = new Cargo(4, airport);
    Cargo* cargo5 = new Cargo(5, airport);

    //	Add all of the cargo handlers to a main list for the use of the manager
    airport->cargoHandlerList->Append((void *)cargo0);
    airport->cargoHandlerList->Append((void *)cargo1);
    airport->cargoHandlerList->Append((void *)cargo2);
    airport->cargoHandlerList->Append((void *)cargo3);
    airport->cargoHandlerList->Append((void *)cargo4);
    airport->cargoHandlerList->Append((void *)cargo5);    
    
    // Create threads.
	Thread* t0 = new Thread("Cargo0");
	Thread* t1 = new Thread("Cargo1");
	Thread* t2 = new Thread("Cargo2");
	Thread* t3 = new Thread("Cargo3");
	Thread* t4 = new Thread("Cargo4");
	Thread* t5 = new Thread("Cargo5");
    
    // Fork threads and pass cargo handler classes.
	t0->Fork(StartCargo, (int)cargo0);
	t1->Fork(StartCargo, (int)cargo1);
	t2->Fork(StartCargo, (int)cargo2);
	t3->Fork(StartCargo, (int)cargo3);
	t4->Fork(StartCargo, (int)cargo4);
	t5->Fork(StartCargo, (int)cargo5);
}


/*void AirportSim()
{
    // MENU ASKING FOR NUMBERS:
    //  - >=3 AIRLINES
    //  - 5-7 LIAISONS
    //  - 3-5 CHECK-IN / AIRLINE (all equal)
    //  - 6-10 CARGO
    //  - # SCREENERS/SECURITY
    //  - # PASSENGERS
    
    // CREATE NEW AIRPORT USING THOSE NUMBERS
    
    // INITIALIZE THREAD CLASSES:
    //  USE 
    
    // FORK
    
    // ???
    
    // PROFIT
    
}*/
