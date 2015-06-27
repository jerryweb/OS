#include "screenofficer.h"
#include "stdlib.h"
#include "time.h"

ScreenOfficer::ScreenOfficer(int ID, Airport* AIRPORT) {
	id = ID;
	airport = AIRPORT;
	airport->screenState[id] = SO_BUSY;
}

ScreenOfficer::~ScreenOfficer() {

}

Passenger* ScreenOfficer::CheckForPassengers() {
	Passenger* P;
	bool luggageTest = true;      //for storing hand luggage test result

	//if my line has passenger waiting
	if (!airport->screenQueues[id]->IsEmpty()) {
		airport->screenState[id] = SO_BUSY;
		//start handling the current passenger
		airport->screenQueuesLock->Acquire();
		P = (Passenger*) airport->screenQueues[id]->Remove();
		airport->screenQueuesLock->Release();

		// Give bag to officer
		printf("Passenger %d gives the hand-luggage to screening officer %d\n",
				P->getID(), id);

		//do hand luggage test, assume 20% chance fail
		int randNum = rand() % 100 + 1;
		if (randNum > 0 && randNum < 21) {
			P->SetSecurityPass(false);
			luggageTest = false;
		}

		//make decision based on test result
		if (luggageTest) {
			printf(
					"Screening officer %d is suspicious of the hand luggage of passenger %d\n",
					id, P->getID());
		} else {
			printf(
					"Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
					id, P->getID());
		}

		//find the available secuirty inspector for the passenger
		airport->securityQueuesLock->Acquire();
		int securityIndex = P->findShortestLine(airport->securityQueues, false,
				false, true);
		//inform passenger the index of next secuirty inspector
		P->SetQueueIndex(securityIndex);
		P->SetSecurityPass(luggageTest);
		printf(
				"Screening officer %d directs passenger %d to security inspector %d\n",
				id, P->getID(), securityIndex);
		airport->securityQueuesLock->Release();

		//signal passenger to proceed
		airport->screenQueuesCV[id]->Signal(airport->screenLocks[id]);

		//if no passenger in line set state to free
	} else {
		airport->screenState[id] = SO_FREE;
		P = NULL;
	}

	return P;

}

/*void ScreenOfficer::Screen() {
 Passenger* p = NULL;
 srand(time(NULL));

 while (true) {
 airport->screenLocks[id]->Acquire();

 //this function do the checking and return the pointer
 //points to current passenger if available
 p = CheckForPassengers();

 if (p != NULL) {
 //wait for passenger's acknowledgement
 airport->screenCV[id]->Wait(airport->screenLocks[id]);
 //after confirmation from passenger procced to next loop
 }

 //if p = NULL then wait on free condition
 else {
 airport->screenQueuesCV[id]->Signal(airport->screenLocks[id]);
 airport->screenFreeCV[id]->Wait(airport->screenLocks[id]);
 }
 }
 }*/

void ScreenOfficer::Screen() {
	srand(time(NULL));

	while (true) {
		Passenger* p = NULL;
		airport->screenQueuesLock->Acquire();
		//airport->screenLocks[id]->Acquire();
		if (!airport->screenQueues[id]->IsEmpty()) {
			airport->screenLocks[id]->Acquire();
			airport->screenState[id] = SO_BUSY;
			p = (Passenger*) airport->screenQueues[id]->Remove();
			airport->screenQueuesCV[id]->Signal(airport->screenQueuesLock);
			airport->screenQueuesLock->Release();

			airport->screenCV[id]->Wait(airport->screenLocks[id]);
			airport->screenLocks[id]->Acquire();

			bool luggageTest;
			//do hand luggage test, assume 20% chance fail
			int randNum = rand() % 100 + 1;
			if (randNum > 0 && randNum < 21) {
				p->SetSecurityPass(false);
				luggageTest = false;
			}

			//make decision based on test result
			if (luggageTest) {
				printf(
						"Screening officer %d is suspicious of the hand luggage of passenger %d\n",
						id, p->getID());
			} else {
				printf(
						"Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
						id, p->getID());
			}

			//find the available secuirty inspector for the passenger
			airport->securityQueuesLock->Acquire();
			int securityIndex = p->findShortestLine(airport->securityQueues,
					false, false, true);
			//inform passenger the index of next secuirty inspector
			p->SetQueueIndex(securityIndex);
			p->SetSecurityPass(luggageTest);
			printf(
					"Screening officer %d directs passenger %d to security inspector %d\n",
					id, p->getID(), securityIndex);
			airport->securityQueuesLock->Release();

			airport->screenCV[id]->Signal(airport->screenLocks[id]);
			airport->screenCV[id]->Wait(airport->screenLocks[id]);

		} else if (airport->allFinished) {
			currentThread->Finish();
		} else {
			airport->screenState[id] = SO_FREE;
			airport->screenQueuesLock->Release();
			currentThread->Yield();
		}
	}
}

