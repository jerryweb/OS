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

void ScreenOfficer::Screen() {
	srand(time(NULL));

	while (true) {
		Passenger* p = NULL;

		//start 1st C.S.
		airport->screenQueuesLock->Acquire();
		if (!airport->screenQueues[id]->IsEmpty()) {
			airport->screenLocks[id]->Acquire();
			airport->screenState[id] = SO_BUSY;
			p = (Passenger*) airport->screenQueues[id]->Remove();
			airport->screenQueuesCV[id]->Signal(airport->screenQueuesLock);
			airport->screenQueuesLock->Release();

			airport->screenCV[id]->Wait(airport->screenLocks[id]); //end 1st C.S.

			//start 2nd C.S.
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
			airport->screenCV[id]->Wait(airport->screenLocks[id]);   //end 2nd C.S.

		//enter here if all passengers are boarded
		} else if (airport->allFinished) {
			currentThread->Finish();

		//if my line is empty, yield
		} else {
			airport->screenState[id] = SO_FREE;
			airport->screenQueuesLock->Release();
			currentThread->Yield();
		}
	}
}

