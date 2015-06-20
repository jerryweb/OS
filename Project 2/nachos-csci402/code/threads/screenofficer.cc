#include "screenofficer.h"
#include "stdlib.h"
#include "time.h"

ScreenOfficer::ScreenOfficer(int ID, Airport* AIRPORT) {
	id = ID;
	airport = AIRPORT;
	airport->screenState[id] = SO_BUSY;
	printf(" screen airport pointer: %p\n", airport);
}

ScreenOfficer::~ScreenOfficer() {

}

Passenger* ScreenOfficer::CheckForPassengers() {
	Passenger* P;
	bool luggageTest = true;      //for storing hand luggage test result

	//if my line has passenger waiting
	if (!airport->screenQueues[id]->IsEmpty()) {
		//start handling the current passenger
		P = (Passenger*) airport->screenQueues[id]->Remove();
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
		printf(
				"Screening officer %d directs passenger %d to security inspector %d\n",
				id, P->getID(), securityIndex);
		airport->securityQueuesLock->Release();

		//signal passenger to proceed
		airport->screenQueuesCV[id]->Signal(airport->screenQueuesLock);
		airport->screenState[id] = SO_BUSY;

	//if no passenger in line set state to free
	} else {
		P = NULL;
		airport->screenState[id] = SO_FREE;
	}

	return P;

}

void ScreenOfficer::Screen() {
	Passenger* p = NULL;
	srand(time(NULL));

	while (true) {
		//	printf(" screen airport pointer: %p\n", airport);

		airport->screenQueuesLock->Acquire();

		//this function do the checking and return the pointer
		//points to current passenger if available
		p = CheckForPassengers();

		if (p != NULL) {
			airport->screenQueuesLock->Release();
			airport->screenLocks[id]->Acquire();
			//wait for passenger's acknowledgement
			airport->screenCV[id]->Wait(airport->screenLocks[id]);
			//after confirmation from passenger procced to next loop
		}

		//if p = NULL then wait on free condition
		else {
			airport->screenQueuesLock->Release();
			airport->screenLocks[id]->Acquire();
			airport->screenFreeCV[id]->Wait(airport->screenLocks[id]);
		}

	}
}

