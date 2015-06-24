/// threadtest.cc 
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
#include "stdlib.h"
#include "liaison.h"
#include "checkin.h"
#include "cargo.h"
#include "manager.h"
#include "screenofficer.h"
#include "securityinspector.h"
#include <iostream>

using namespace std;

extern void TestSuite();

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void SimpleThread(int which) {
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

void ThreadTest() {
	DEBUG('t', "Entering SimpleTest");

	Thread *t = new Thread("forked thread");

	t->Fork(SimpleThread, 1);
	SimpleThread(0);
}

//----------------------------------------------------------------------
// The passenger should find the shortest liaison line 
//----------------------------------------------------------------------
void StartFindShortestLiaisonLine(int arg) {
	Passenger* p = (Passenger*) arg;
	p->findShortestLiaisonLine();
}

void StartFindCorrectCISLine(int arg) {
	Passenger* p = (Passenger*) arg;
	p->CheckIn();
}

void StartLiaisonTest(int arg) {
	Liaison* l = (Liaison*) arg;
	Passenger* p = l->CheckForPassengers();
}

void StartLiaisonThread(int arg) {
	Liaison* L = (Liaison*) arg;
	L->DirectPassengers();
}

void StartCargo(int arg) {
	Cargo* c = (Cargo*) arg;
	c->StartCargo();
}

void StartManager(int arg) {
	Manager* M = (Manager*) arg;
	M->MakeRounds();
}

void StartCheckInTest(int arg) {
	CheckIn* ci = (CheckIn*) arg;
	Passenger* p = ci->FindPassenger(0);
}

void StartCheckInStaff(int arg) {
	CheckIn* ci = (CheckIn*) arg;
	ci->StartCheckInStaff();
}

/********************Screen&Security************************/
//call screen officer
void StartScreening(int arg) {
	ScreenOfficer* s = (ScreenOfficer*) arg;
	s->Screen();
}

void FindScreeningOfficer(int arg) {
	Passenger* p = (Passenger*) arg;
	p->Screening();
}

//call passenger
void StartScreeningTest(int arg) {
	Passenger* p = (Passenger*) arg;
	p->Screening();
}

//call inspector
void StartInspecting(int arg) {
	SecurityInspector* s = (SecurityInspector*) arg;
	s->Inspect();
}

//call passenger
void StartInspectingTest(int arg) {
	Passenger* p = (Passenger*) arg;
	p->Inspecting();
}
/********************************************/

//----------------------------------------------------------------------
//These are the initial print statements needed at the beginning of each simulation
//The parameters should all be lists of each thread. Assumption that [Count] in the 
// output guidelines do not include the brackets in the output.
//----------------------------------------------------------------------
void StartupOutput(Airport* airport) {
	printf("Number of airport liaisons = %d\n", airport->liaisonList->Size());
	printf("Number of airlines = %d\n", airport->numAirlines);
	printf("Number of check-in staff = %d\n",
			airport->checkInStaffList->Size());
	printf("Number of cargo handlers = %d\n",
			airport->cargoHandlerList->Size());
	printf("Number of screening officers = %d\n",
			airport->screenOfficerList->Size());
	printf("Total number of passengers = %d\n", airport->passengerList->Size());

	for (int h = 0; h < airport->numAirlines; h++) {
		printf("Number of passengers for airline %d = %d\n",
				airport->airlines[h]->id, airport->airlines[h]->ticketsIssued);
	}

	//This prints out the number bags and their weights for each passenger 
	for (int i = 0; i < airport->passengerList->Size(); i++) {
		Passenger *P = (Passenger*) airport->passengerList->Remove();
		airport->passengerList->Append((void *) P);	//Prevent destruction of global passenger list
		List *bags = P->getLuggage();//Temp list for iterating through luggage
		int numBags = bags->Size();

		printf("Passenger %d belongs to airline %d\n", P->getID(),
				P->getTicket().airline);
		printf("Passenger %d : Number of bags = %d\n", P->getID(), numBags);
		printf("Passenger %d : Weight of bags = ", P->getID());
		for (int j = 0; j < numBags; j++) {
			if (j != 0)
				printf(", ");
			Luggage* l = (Luggage*) bags->Remove();
			bags->Append(l);
			printf("%d", l->weight);
		}
		printf("\n");
	}

	//Prints Check-in Staff information
	int CISsize = airport->checkInStaffList->Size();
	for (int k = 0; k < CISsize; k++) {
		CheckIn *CIS = (CheckIn*) airport->checkInStaffList->Remove();
		airport->checkInStaffList->Append((void *) CIS);
		printf("Airline check-in staff %d belongs to airline %d\n",
				CIS->getID(), CIS->getAirline());
	}
}

//----------------------------------------------------------------------
// This runs a full simulation of the airport interactions in order to test the manager
// 10 passengers, 7 liaisons, 3 airlines each with 5 check-in staff, and 10 cargo handlers
// are created at the beginning of the test.
//----------------------------------------------------------------------
void ManagerTest() {
	Airport *airport = new Airport();
	//These are the arrays that will hold the thread pointers, which are to be forked 
	List* PassengerThreadArray = new List();
	List* LiaisonThreadArray = new List();
	List* CheckInStaffThreadArray = new List();
	List* CargoHandlerTreadArray = new List();
	List* SecurityInspectorThreadArray = new List();
	List* ScreenOfficerThreadArray = new List();
    
    int* passengersPerAirline = new int[3];
	int* bagsPerAirline = new int[3];
	int* bagWeightPerAirline = new int[3];
    
    // Generate passengers, luggage, and airlines.
	for (int a = 0; a < 3; a++) {
		passengersPerAirline[a] = 0;
		bagsPerAirline[a] = 0;
		bagWeightPerAirline[a] = 0;
	}
	for (int i = 0; i < 10; i++) {
		Ticket ticket;
		int airline = rand() % 3;
		passengersPerAirline[airline]++;
		ticket.airline = airline;
		ticket.executive = (rand() % 100 < 50);
		int numBags = rand() % 2 + 2;
		List* bags = new List();
		for (int j = 0; j < numBags; j++) {
			Luggage* l = new Luggage;
			l->airlineCode = -1;
			l->weight = rand() % 30 + 30;
            		bagsPerAirline[airline]++;
            		bagWeightPerAirline[airline] += l->weight;
			bags->Append((void*) l);
		}
		Passenger* P = new Passenger(i, bags, ticket, airport);
		airport->passengerList->Append((void*) P);
		Thread* tP = new Thread("Passenger");
		PassengerThreadArray->Append((void*) tP);
	}
	for (int i = 0; i < 3; i++) {
		airport->airlines[i] = new Airline(i, passengersPerAirline[i],
				bagsPerAirline[i], bagWeightPerAirline[i]);
	}

	//Generates Liaisons
	for (int k = 0; k < 7; k++) {
		Liaison *L = new Liaison(k, airport);
		airport->liaisonList->Append((void *) L);
		Thread *tL = new Thread("Liaison");
		LiaisonThreadArray->Append((void *) tL);
	}

	//Generates Check-in Staff; there are 5 for each airline
	for (int m = 0; m < 3; m++) {
		for (int n = 0; n < 5; n++) {
			CheckIn *C = new CheckIn(m, m * 6 + n + 1, airport);
			airport->checkInStaffList->Append((void *) C);
			Thread *tC = new Thread("CheckInStaff");
			CheckInStaffThreadArray->Append((void *) tC);
		}
	}

	/*/Generates Cargo Handlers; there should be 10 in total
	for (int q = 0; q < 10; q++) {
		Cargo* cargo = new Cargo(q, airport);
		airport->cargoHandlerList->Append((void *) cargo);
		Thread *tCH = new Thread("Cargo_Handler");
		CargoHandlerTreadArray->Append((void *) tCH);
	}*/

	//generates Screening Officers
	for (int s = 0; s < 3; s++) {
		ScreenOfficer* SO = new ScreenOfficer(s, airport);
		airport->screenOfficerList->Append((void *) SO);
		Thread* tSO = new Thread("Screening Officer");
		ScreenOfficerThreadArray->Append((void *) tSO);
	}

	//generates security inspectors
	for (int i = 0; i < 3; i++) {
		SecurityInspector* SI = new SecurityInspector(i, airport);
		airport->securityInspectorList->Append(SI);
		Thread* tSI = new Thread("Security Inspector");
		SecurityInspectorThreadArray->Append(tSI);
	}

	//Generates an Airport Manager
	Manager* manager = new Manager(airport);

	//Display initial airport data
	StartupOutput(airport);

	//Fork all of the Passenger Threads from the array of passenger threads
	for (int i = PassengerThreadArray->Size(); i > 0; i--) {
		Passenger *P = (Passenger*) airport->passengerList->First();
		airport->passengerList->Remove();
		airport->passengerList->Append((void *) P);
		Thread *t = (Thread*) PassengerThreadArray->First();
		PassengerThreadArray->Remove();
		t->Fork(StartFindShortestLiaisonLine, (int(P)));
	}

	//Fork all of the Liaison Threads from the array of liaison threads
	for (int j = LiaisonThreadArray->Size(); j > 0; j--) {
		Liaison *L = (Liaison*) airport->liaisonList->First();
		airport->liaisonList->Remove();
		airport->liaisonList->Append((void *) L);
		Thread *tL = (Thread*) LiaisonThreadArray->First();
		LiaisonThreadArray->Remove();
		tL->Fork(StartLiaisonThread, (int(L)));
	}

	//Fork all of the Check-in Staff Threads from the array of Check-in Staff threads
	for (int k = CheckInStaffThreadArray->Size(); k > 0; k--) {
		CheckIn *CIS = (CheckIn*) airport->checkInStaffList->First();
		airport->checkInStaffList->Remove();
		airport->checkInStaffList->Append((void *) CIS);
		Thread *tCIS = (Thread*) CheckInStaffThreadArray->First();
		CheckInStaffThreadArray->Remove();
		tCIS->Fork(StartCheckInStaff, (int(CIS)));
	}

	/*Fork all of the Cargo Handlers Threads from the array of Cargo Handlers threads
	for (int m = CargoHandlerTreadArray->Size(); m > 0; m--) {
		Cargo *CH = (Cargo*) airport->cargoHandlerList->First();
		airport->cargoHandlerList->Remove();
		airport->cargoHandlerList->Append((void *) CH);
		Thread *tCH = (Thread*) CargoHandlerTreadArray->First();
		CargoHandlerTreadArray->Remove();
		tCH->Fork(StartCargo, (int(CH)));
	}*/

	for (int p = ScreenOfficerThreadArray->Size(); p > 0; p--) {
		ScreenOfficer *SO =
				(ScreenOfficer*) airport->screenOfficerList->Remove();
		airport->screenOfficerList->Append((void *) SO);
		Thread* tSO = (Thread*) ScreenOfficerThreadArray->Remove();
		tSO->Fork(StartScreening, (int(SO)));
	}

	for (int i = SecurityInspectorThreadArray->Size(); i > 0; i--) {
		SecurityInspector *SI =
				(SecurityInspector*) airport->securityInspectorList->Remove();
		airport->securityInspectorList->Append(SI);
		Thread* tSI = (Thread*) SecurityInspectorThreadArray->Remove();
		tSI->Fork(StartInspecting, (int(SI)));
	}

	Thread* tM = new Thread("Manager");
	tM->Fork(StartManager, (int) manager);
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
// Sets liaison to busy
//   Initializes 1 passenger (id 0) and runs it.
//   Intended result:
//    Passenger 0 will go to line 6 (length 0).
//----------------------------------------------------------------------
void PassengerFindsShortestLiaisonLine() {
	Airport *airport = new Airport(); //This creates a new airpost object with all of the
									  //global variables listed here

	// This fills the liaison queues with dummy int variables to simulate line lengths
	for (int i = 5; i >= 0; i--) {
		int tempVariable = 5 - i;
		for (int j = 0; j < 6 - i; j++) {
			airport->liaisonQueues[i]->Append((void *) tempVariable);
		}
	}

	Passenger *p = new Passenger(0, airport);

	//Beginning of shortest line test and start of critical section for finding shortest line
	Thread *t = new Thread("Passenger");

	t->Fork(StartFindShortestLiaisonLine, (int(p)));
}

//----------------------------------------------------------------------
//	LiaisonTest
// 	 Adds 1 passengers into liaison 0's queue:
//    id 0, ticket.airline 2
//   Initializes 1 Liaison thread (id 0) and runs it.
//   Intended result:
//    The Liaison will direct the passenger to airline 2.
//----------------------------------------------------------------------
void LiaisonTest() {
	Airport* airport = new Airport(); // 3 airlines

	// Create ticket.
	Ticket ticket;
	ticket.airline = 2;
	ticket.executive = true; // irrelevant

	// Create passenger shell.
	Passenger* p0 = new Passenger(0, ticket);

	// Add passengers to queue.
	airport->liaisonQueues[0]->Append(p0);

	// Create Liaison class.
	Liaison* l = new Liaison(0, airport);

	// Create thread.
	Thread* t = new Thread("Liaison");

	// Fork thread and pass CIS class.
	t->Fork(StartLiaisonTest, (int) l);
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
//   Sets CIS 4's state to closed.
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
void PassengerFindsCorrectCISLine() {
	Airport* airport = new Airport(); // 3 airlines

	// Populate the check-in list.
	int cisLineLengths[6] = { 0, 3, 4, 2, 0, 7 };
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < cisLineLengths[i]; j++) {
			Passenger* p = new Passenger();
			airport->checkinQueues[i]->Append(p);
			airport->passengerList->Append(p);
		}
	}

	// Close the 4th CIS stand.
	airport->checkinState[4] = CI_CLOSED;

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
	t0->Fork(StartFindCorrectCISLine, (int) p0);
	t1->Fork(StartFindCorrectCISLine, (int) p1);
	t2->Fork(StartFindCorrectCISLine, (int) p2);
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
void CheckInTest() {
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

	// Create thread.
	Thread* t = new Thread("CheckIn");

	// Fork thread and pass CIS class.
	t->Fork(StartCheckInTest, (int) ci);
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
void CargoTest() {
	Airport* airport = new Airport(); // 3 airlines

	int weight, i, j;

	// Create luggage and add to conveyor.
	for (i = 0; i < 3; i++) {
		Luggage* bag = new Luggage;
		bag->airlineCode = i;
		bag->weight = 30 + 12 * i;
		airport->conveyor->Append((void*) bag);
	}
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			Luggage* bag = new Luggage;
			bag->airlineCode = j;
			bag->weight = 30 + 15 * (i + j);
			airport->conveyor->Append((void*) bag);
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
	airport->cargoHandlerList->Append((void *) cargo0);
	airport->cargoHandlerList->Append((void *) cargo1);
	airport->cargoHandlerList->Append((void *) cargo2);
	airport->cargoHandlerList->Append((void *) cargo3);
	airport->cargoHandlerList->Append((void *) cargo4);
	airport->cargoHandlerList->Append((void *) cargo5);

	// Create threads.
	Thread* t0 = new Thread("Cargo0");
	Thread* t1 = new Thread("Cargo1");
	Thread* t2 = new Thread("Cargo2");
	Thread* t3 = new Thread("Cargo3");
	Thread* t4 = new Thread("Cargo4");
	Thread* t5 = new Thread("Cargo5");

	// Fork threads and pass cargo handler classes.
	t0->Fork(StartCargo, (int) cargo0);
	t1->Fork(StartCargo, (int) cargo1);
	t2->Fork(StartCargo, (int) cargo2);
	t3->Fork(StartCargo, (int) cargo3);
	t4->Fork(StartCargo, (int) cargo4);
	t5->Fork(StartCargo, (int) cargo5);
}

void ScreenTest() {
	Airport* airport = new Airport();

	//declare passenger array
	Passenger** screenPassenger = new Passenger*[5];

	//declare a single officer
	ScreenOfficer** sOfficer = new ScreenOfficer*[3];
	SecurityInspector** sInspector = new SecurityInspector*[3];
	for (int i = 0; i < 3; i++) {
		sOfficer[i] = new ScreenOfficer(i, airport);
		airport->screenOfficerList->Append((void *) sOfficer[i]);
		sInspector[i] = new SecurityInspector(i, airport);
		airport->securityInspectorList->Append(sInspector[i]);
	}

	for (int i = 0; i < 5; i++) {
		screenPassenger[i] = new Passenger(i, airport);
	}
    
//spawning all the passenger and officer threads for the test,then fork them
	Thread** passengerThreads = new Thread*[5];
	for (int i = 0; i < 5; i++) {
		passengerThreads[i] = new Thread("Passenger");
		passengerThreads[i]->Fork(StartScreeningTest,
				(int(screenPassenger[i])));
	}

	Thread** officerThreads = new Thread*[3];
	Thread** inspectorThreads = new Thread*[3];

	for (int i = 0; i < 3; i++) {
		officerThreads[i] = new Thread("Officer");
		officerThreads[i]->Fork(StartScreening, (int(sOfficer[i])));
		inspectorThreads[i] = new Thread("Inspector");
		inspectorThreads[i]->Fork(StartInspecting, (int(sInspector[i])));
	}

}

void InspectTest() {
	Airport* airport = new Airport();

    //declare passenger array
	Passenger** inspectPassenger = new Passenger*[3];

    //declare a single inspector
	SecurityInspector** sInspector = new SecurityInspector*[1];
	sInspector[0] = new SecurityInspector(0, airport);
	airport->securityInspectorList->Append((void *) sInspector[0]);

	for (int i = 0; i < 3; i++) {
		BoardingPass bP;
		bP.gate = i;
		bP.seatNum = i * 5;
		inspectPassenger[i] = new Passenger(i, airport, bP);
	}

    //spwaning all the passenger and inspector threads for the test,then fork them
	Thread** passengerThreads = new Thread*[3];
	for (int i = 0; i < 3; i++) {
		passengerThreads[i] = new Thread("Passenger");
		passengerThreads[i]->Fork(StartInspectingTest,
				(int(inspectPassenger[i])));
	}

	Thread* inspectorThread = new Thread("Inspector");
	inspectorThread->Fork(StartInspecting, (int(sInspector[0])));
}
void MTest() {
	Airport* airport = new Airport(); // 3 airlines

	int weight, i, j;

// Create luggage and add to conveyor.
	for (i = 0; i < 3; i++) {
		Luggage* bag = new Luggage;
		bag->airlineCode = i;
		bag->weight = 30 + 12 * i;
		airport->conveyor->Append((void*) bag);
	}
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			Luggage* bag = new Luggage;
			bag->airlineCode = j;
			bag->weight = 30 + 15 * (i + j);
			airport->conveyor->Append((void*) bag);
		}
	}

// Create cargo handler classes.
	Cargo* cargo0 = new Cargo(0, airport);
	Cargo* cargo1 = new Cargo(1, airport);
	Cargo* cargo2 = new Cargo(2, airport);
	Cargo* cargo3 = new Cargo(3, airport);
	Cargo* cargo4 = new Cargo(4, airport);
	Cargo* cargo5 = new Cargo(5, airport);

//Generates an Airport Manager
	Manager* manager = new Manager(airport);

//	Add all of the cargo handlers to a main list for the use of the manager
	airport->cargoHandlerList->Append((void *) cargo0);
	airport->cargoHandlerList->Append((void *) cargo1);
	airport->cargoHandlerList->Append((void *) cargo2);
	airport->cargoHandlerList->Append((void *) cargo3);
	airport->cargoHandlerList->Append((void *) cargo4);
	airport->cargoHandlerList->Append((void *) cargo5);

// Create threads.
	Thread* t0 = new Thread("Cargo0");
	Thread* t1 = new Thread("Cargo1");
	Thread* t2 = new Thread("Cargo2");
	Thread* t3 = new Thread("Cargo3");
	Thread* t4 = new Thread("Cargo4");
	Thread* t5 = new Thread("Cargo5");
	Thread* tM = new Thread("Manager");

// StartupOutput(airport);

// Fork threads and pass cargo handler classes.
	t0->Fork(StartCargo, (int) cargo0);
	t1->Fork(StartCargo, (int) cargo1);
	t2->Fork(StartCargo, (int) cargo2);
	t3->Fork(StartCargo, (int) cargo3);
	t4->Fork(StartCargo, (int) cargo4);
	t5->Fork(StartCargo, (int) cargo5);
	tM->Fork(StartManager, (int) manager);
}

void AirportSim() {
	srand(time(NULL));

	int i;

	int airlines = 0;
	int passengers = 0;
	int liaisons = 0;
	int checkins = 0;
	int cargos = 0;
	int security = 0;
	List* passengerThreadList = new List();
	List* liaisonThreadList = new List();
	List* checkinThreadList = new List();
	List* cargoThreadList = new List();
	List* screenerThreadList = new List();
	List* securityThreadList = new List();

// Menu asking for numbers:
	printf("============WELCOME TO AIRPORT SIMULATOR============\n");
	printf("ENTERING ANYTHING BUT NUMBERS IS STRICTLY FORBIDDEN!\n");
	while (airlines < 3 || airlines > 6) { // 3-6 airlines (arbitrary upper limit).
		printf("Please enter the number of airlines (3-6): ");

		cin >> airlines;
		cin.get();
	}
	while (liaisons < 5 || liaisons > 7) {   // 5-7 liaisons.
		printf("Please enter the number of liaisons (5-7): ");

		cin >> liaisons;
		cin.get();
	}
	while (checkins < 3 || checkins > 5) {   // 3-5 check-in staff per airline.
		printf("Please enter the number of check-in staff per airline (3-5): ");

		cin >> checkins;
		cin.get();
	}
	while (cargos < 6 || cargos > 10) {   // 6-10 cargo handlers.
		printf("Please enter the number of cargo handlers (6-10): ");

		cin >> cargos;
		cin.get();
	}
	while (security < 1 || security > 10) { // 1-10 screeners/inspectors (arbitrary limits).
		printf(
				"Please enter the number of screening officers and security inspectors (1-10): ");

		cin >> security;
		cin.get();
	}
	while (passengers < 20 || passengers > 100) { // 20-100 passengers (arbitrary upper limit).
		printf("Please enter the number of passengers (20-100): ");

		cin >> passengers;
		cin.get();
	}

// Create a new airport using given numbers.
	Airport* airport = new Airport(airlines, passengers, liaisons, checkins, security, cargos);
    printf("Airport created.\n");

// Initialize data classes and threads.
	int* passengersPerAirline = new int[airlines];
	int* bagsPerAirline = new int[airlines];
	int* bagWeightPerAirline = new int[airlines];
	for (i = 0; i < airlines; i++) {
		passengersPerAirline[i] = 0;
		bagsPerAirline[i] = 0;
		bagWeightPerAirline[i] = 0;
	}
	for (i = 0; i < passengers; i++) {
		Ticket ticket;
		int airline = rand() % airlines;
		passengersPerAirline[airline]++;
		ticket.airline = airline;
		ticket.executive = (rand() % 100 < 50);
		int numBags = rand() % 2 + 2;
		List* bags = new List();
		for (int j = 0; j < numBags; j++) {
			Luggage* l = new Luggage;
			l->airlineCode = -1;
			l->weight = rand() % 30 + 30;
            bagsPerAirline[airline]++;
            bagWeightPerAirline[airline] += l->weight;
			bags->Append((void*) l);
		}
		Passenger* p = new Passenger(i, bags, ticket, airport);
		airport->passengerList->Append((void*) p);
		Thread* t = new Thread("Passenger");
		passengerThreadList->Append((void*) t);
	}
    printf("Passengers and luggage created.\n");
	for (i = 0; i < airlines; i++) {
		airport->airlines[i] = new Airline(i, passengersPerAirline[i],
				bagsPerAirline[i], bagWeightPerAirline[i]);
	}
    printf("Airlines created.\n");
	for (i = 0; i < liaisons; i++) {
		Liaison* l = new Liaison(i, airport);
		airport->liaisonList->Append((void*) l);
		Thread* t = new Thread("Liaison");
		liaisonThreadList->Append((void*) t);
	}
    printf("Liaisons created.\n");
	for (i = 0; i < airlines * 6; i++) {
		if (i % 6 != 0 && i % 6 <= checkins) {
			CheckIn* ci = new CheckIn(i / 6, i, airport);
			airport->checkInStaffList->Append((void*) ci);
			Thread* t = new Thread("CheckIn");
			checkinThreadList->Append((void*) t);
		}
	}
    printf("Check-in staff created.\n");
	for (i = 0; i < security; i++) {
		// Screeners
		ScreenOfficer* so = new ScreenOfficer(i, airport);
		airport->screenOfficerList->Append((void*) so);
		Thread* to = new Thread("ScreenOfficer");
		screenerThreadList->Append((void*) to);
		// Security
		SecurityInspector* si = new SecurityInspector(i, airport);
		airport->securityInspectorList->Append((void*) si);
		Thread* ti = new Thread("SecurityInspector");
		securityThreadList->Append((void*) ti);
	}
    printf("Screeners and security created.\n");
	for (i = 0; i < cargos; i++) {
		Cargo* c = new Cargo(i, airport);
		airport->cargoHandlerList->Append((void*) c);
		Thread* t = new Thread("Cargo");
		cargoThreadList->Append((void*) t);
	}
    printf("Cargo handlers created.\n");
	Manager* m = new Manager(airport);
	Thread* tm = new Thread("Manager");
    printf("Manager created.\n");

// Display initial airport data.
	StartupOutput(airport);
// Fork the threads.
	while (!passengerThreadList->IsEmpty()) {
		Passenger* p = (Passenger*) airport->passengerList->Remove();
		airport->passengerList->Append((void*) p);
		Thread* t = (Thread*) passengerThreadList->Remove();
		t->Fork(StartFindShortestLiaisonLine, (int) p);
	}
	while (!liaisonThreadList->IsEmpty()) {
		Liaison* l = (Liaison*) airport->liaisonList->Remove();
		airport->liaisonList->Append((void*) l);
		Thread* t = (Thread*) liaisonThreadList->Remove();
		t->Fork(StartLiaisonThread, (int) l);
	}
	while (!checkinThreadList->IsEmpty()) {
		CheckIn* ci = (CheckIn*) airport->checkInStaffList->Remove();
		airport->checkInStaffList->Append((void*) ci);
		Thread* t = (Thread*) checkinThreadList->Remove();
		t->Fork(StartCheckInStaff, (int) ci);
	}
	while (!cargoThreadList->IsEmpty()) {
		Cargo* c = (Cargo*) airport->cargoHandlerList->Remove();
		airport->cargoHandlerList->Append((void*) c);
		Thread* t = (Thread*) cargoThreadList->Remove();
		t->Fork(StartCargo, (int) c);
	}
	while (!screenerThreadList->IsEmpty()) {
		ScreenOfficer* so =
				(ScreenOfficer*) airport->screenOfficerList->Remove();
		airport->screenOfficerList->Append((void*) so);
		Thread* t = (Thread*) screenerThreadList->Remove();
		t->Fork(StartScreening, (int) so);
	}
	while (!securityThreadList->IsEmpty()) {
		SecurityInspector* si =
				(SecurityInspector*) airport->securityInspectorList->Remove();
		airport->securityInspectorList->Append((void*) si);
		Thread* t = (Thread*) securityThreadList->Remove();
		t->Fork(StartInspecting, (int) si);
	}
	tm->Fork(StartManager, (int) m);

// ???

// PROFIT
}

void MainMenu() {
	int choice = -1;
	while (choice < 0 || choice > 12) {
		printf("CHOOSE ONE (PLEASE ONLY ENTER A NUMBER):\n");
		printf("0:  Lock and CV test code.\n");
		printf(
				"1:  Passenger selects the shortest line for the airport liaison.\n");
		printf(
				"2:  Passenger is directed by the Liaison to the correct airline counters.\n");
		printf(
				"3:  Economy class passengers enter the shortest line while Executive class passengers enter their correct line.\n");
		printf(
				"4:  Executive class passengers are given priority over the economy class passengers at the check-in kiosks.\n");
		printf(
				"5:  Screening officer chooses an available security inspector each time a passenger comes in.\n");
		printf(
				"6:  Cargo handlers choose bags from the conveyor system each time and go on a break if there are no bags.\n");
		printf(
				"7:  Handing over of the hand luggage by the passenger to the screening officer.\n");
		printf(
				"8:  Passenger returns to the same security inspector after further questioning.\n");
		printf(
				"9:  Baggage weights of all the passengers of a particular airline should match the weights of the bags reported by the cargo handlers.\n");
		printf(
				"10: Handing over of boarding pass by the passenger to the security inspector.\n");
		printf("11: Hardcoded airport simulation.\n");
		printf("12: Full airport simulation.\n");

		cin >> choice;
	}
	switch (choice) {
	case 0:
		TestSuite();
		break;
	case 1:
		PassengerFindsShortestLiaisonLine();
		break;
	case 2:
		LiaisonTest();
		break;
	case 3:
		PassengerFindsCorrectCISLine();
		break;
	case 4:
		CheckInTest();
		break;
	case 5:
		ScreenTest();
		break;
	case 6:
		CargoTest();
		break;
	case 7:
		ScreenTest();
		break;
	case 8:
		InspectTest();
		break;
	case 9:
		MTest();
		break;
	case 10:
		InspectTest();
		break;
	case 11:
		ManagerTest();
		break;
	case 12:
		AirportSim();
		break;
	}
}
