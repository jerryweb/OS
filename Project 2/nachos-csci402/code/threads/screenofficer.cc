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
	bool luggageTest = true;
	if (!airport->screenQueues[id]->IsEmpty()) {
		P = (Passenger*) airport->screenQueues[id]->Remove();
        
		int randNum = rand() % 100 + 1;
		if (randNum > 0 && randNum < 21) {
			P->SetSecurityPass(false);
			luggageTest = false;
		}

		if (luggageTest) {
            printf("Screening officer %d is suspicious of the hand luggage of passenger %d\n",
                    id, P->getID());
		} else {
            printf("Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
					id, P->getID());
		}

		airport->securityQueuesLock->Acquire();
		int securityIndex = P->findShortestLine(airport->securityQueues, false,
				false, true);
		P->SetQueueIndex(securityIndex);
		printf("Screening officer %d directs passenger %d to security inspector %d\n",
				id, P->getID(), securityIndex);
		airport->securityQueuesLock->Release();

		airport->screenQueuesCV[id]->Signal(airport->screenQueuesLock);
		airport->screenState[id] = SO_BUSY;
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
		airport->screenQueuesLock->Acquire();

		p = CheckForPassengers();

		if (p != NULL) {
			airport->screenQueuesLock->Release();
			airport->screenLocks[id]->Acquire();
			airport->screenCV[id]->Wait(airport->screenLocks[id]);
		}

		else {
			airport->screenQueuesLock->Release();
			airport->screenLocks[id]->Acquire();
			airport->screenFreeCV[id]->Wait(airport->screenLocks[id]);
		}

	}
}

