#include "screenofficer.h"
#include "stdlib.h"
#include "time.h"

ScreenOfficer::ScreenOfficer(int ID, Airport* AIRPORT) {
	id = ID;
	airport = AIRPORT;
}

ScreenOfficer::~ScreenOfficer() {

}

Passenger* ScreenOfficer::CheckForPassengers() {
	Passenger* P;

	//if my line has passenger waiting
	if (!airport->screenQueues[id]->IsEmpty()) {
		//start handling the current passenger
		airport->screenQueuesCV[id]->Signal(airport->screenQueuesLock);
		P = (Passenger*) airport->screenQueues[id]->Remove();
		airport->screenState[id] = SO_BUSY;
		// //do hand luggage test, assume 20% chance fail
		// int randNum = rand() % 100 + 1;
		// if (randNum > 0 && randNum < 50) {
		// 	P->SetSecurityPass(false);
		// 	luggageTest = false;
		// }

		// //make decision based on test result
		// if (luggageTest) {
  //           printf("Screening officer %d is suspicious of the hand luggage of passenger %d\n",
  //                   id, P->getID());
		// } else {
  //           printf("Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
		// 			id, P->getID());
		// }

		// //find the available secuirty inspector for the passenger
		// airport->securityQueuesLock->Acquire();
		// int securityIndex = P->findShortestLine(airport->securityQueues, false,
		// 		false, true);
		// //inform passenger the index of next secuirty inspector
		// P->SetQueueIndex(securityIndex);
		// printf("Screening officer %d directs passenger %d to security inspector %d\n",
		// 		id, P->getID(), securityIndex);
		// airport->securityQueuesLock->Release();

		// //signal passenger to proceed
		// airport->screenQueuesCV[id]->Signal(airport->screenQueuesLock);
		// airport->screenState[id] = SO_BUSY;

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
	bool luggageTest = true;      //for storing hand luggage test result
	while (true) {
		airport->screenQueuesLock->Acquire();
		// printf("size of my %d queue is %d\n", id, airport->screenQueues[id]->Size());
		//this function do the checking and return the pointer
		//points to current passenger if available
		p = CheckForPassengers();
		airport->screenLocks[id]->Acquire();
		airport->screenQueuesLock->Release();

		if (p != NULL) {
			//wait for passenger's bags

			airport->screenCV[id]->Wait(airport->screenLocks[id]);
			//wait for confirmation that passenger has given bags and moved to inspector
			airport->screenLocks[id]->Acquire();
			//do hand luggage test, assume 20% chance fail
			int randNum = rand() % 100 + 1;
			if (randNum > 0 && randNum < 21) {
				luggageTest = false;
			}
			//make decision based on test result
			if (luggageTest) {
	            printf("Screening officer %d is suspicious of the hand luggage of passenger %d\n",
	                    id, p->getID());
			} else {
	            printf("Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
						id, p->getID());
			}

			//find the available secuirty inspector for the passenger
			airport->securityQueuesLock->Acquire();
			int securityIndex = p->findShortestLine(airport->securityQueues, false,
					false, true);

			//inform passenger the index of next secuirty inspector
			p->SetQueueIndex(securityIndex);
			//this needs to be changed
			p->SetSecurityPass(luggageTest);
			
			printf("Screening officer %d directs passenger %d to security inspector %d\n",
					id, p->getID(), securityIndex);

			// //signal passenger to proceed
			// airport->screenQueuesCV[id]->Signal(airport->screenQueuesLock);
			//after confirmation from passenger procced to next loop
			airport->screenLocks[id]->Release();
			airport->securityQueuesLock->Release();
			
		}

		//if p = NULL then wait on free condition
		else {
			airport->screenQueuesLock->Release();
			//airport->screenLocks[id]->Acquire();
			airport->screenFreeCV[id]->Wait(airport->screenLocks[id]);
		}

	}
}

