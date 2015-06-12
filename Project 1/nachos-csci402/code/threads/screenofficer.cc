#include "screenofficer.h"
#include "stdlib.h"
#include "time.h"

ScreenOfficer::ScreenOfficer(int ID, Airport* AIRPORT) {
	id = ID;
	airport = AIRPORT;
}

ScreenOfficer::~ScreenOfficer() {

}

void ScreenOfficer::Screen() {
	//initialize random seed
	srand(time(NULL));
	//keep checking his own line
	while (true) {
		//the index in C.S. parenthesis is for this file only

		//start C.S.(1) between passenger
		//assuming officer ID is the same as his index in screenLocks array
		int myLine = id;
		airport->screenLocks[myLine]->Acquire();
		Passenger* currentPassenger;

		if ((airport->screenQueues[myLine]->Size()) != 0) {
			currentPassenger = (Passenger*) airport->screenQueues[id]->First();

			//assume 20% chance fail the screening
			int randNum = rand() % 100 + 1;
			if (randNum > 0 && randNum < 21) {
				currentPassenger->SetSecurityPass(false);
				printf(
						"Screening officer %d is suspicious of the hand luggage of passenger %d\n",
						id, currentPassenger->getID());
			} else {
				printf(
						"Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
						id, currentPassenger->getID());
			}

			//remove the passenger in current line

			//starting C.S.(2) to remove passenger from current line
			airport->screenQueuesLock->Acquire();
			airport->screenQueues[myLine]->Remove();
			airport->screenQueuesLock->Release();
			//ending C.S.(2) to remove passenger from current line
			//}

			//starting C.S.(3) to find shortest security line and append
			airport->securityQueuesLock->Acquire();

			int shortest = 0;		//shortest line's id
			int minimumSize = -1;		//for comparsion in the following loop

		//find the shortest line
		for (int i = 0; i < airport->securityInspectorList->Size(); i++) {
			if (minimumSize < 0
					|| minimumSize > airport->securityQueues[i]->Size()) {
				minimumSize = airport->securityQueues[i]->Size();
				shortest = i;
			}
        }

			airport->securityQueues[shortest]->Append(currentPassenger);
			//also update the queueIndex for current passenger
			currentPassenger->SetQueueIndex(shortest);

			airport->securityQueuesLock->Release();
			//ending C.S.(3)

			//signal the passenger
			airport->passengerWaitOfficerCV[myLine]->Signal(
					airport->screenLocks[myLine]);
			//printf("SO Release 77\n");
			airport->screenLocks[myLine]->Release();

			//wait for passenger thread
			airport->screenLocks[myLine]->Acquire();
			airport->officerWaitPassengerCV[myLine]->Wait(
					airport->screenLocks[myLine]);
			//ending C.S.(1)

			//airport->screenLocks[myLine]->Acquire();
			//Assuming the index in securityQueues array is the same as security inspector's id
			printf(
					"Screening officer %d directs passenger %d to security inspector %d\n",
					id, currentPassenger->getID(), shortest);
			//airport->screenLocks[myLine]->Release();
		}
	}
}
