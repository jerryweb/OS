#include "checkin.h"

CheckIn::CheckIn(int airline_, int id_, Airport* airport_) {
	airline = airline_;
	id = id_;
	airport = airport_;
	passengers = 0;
	luggage = 0;
	airport->RequestingCheckinData[id] = false;
}

CheckIn::~CheckIn() {
	airport = NULL;
}

void CheckIn::StartCheckInStaff() {
	int exec = airline * 6;
	//Passenger* p = NULL;
	//bool isExec = false;
	BoardingPass bp;

	while (true) {
		Passenger* p = NULL;
		bool isExec = false;
		//airport->checkinLock[id]->Acquire();
		airport->checkinLineLock[airline]->Acquire();
		airport->checkinLock[id]->Acquire();
		if (airport->checkinQueues[exec]->Size() > 0) {
			//airport->checkinLock[id]->Acquire();
			airport->checkinState[id] = CI_BUSY;
			p = (Passenger*) airport->checkinQueues[exec]->Remove();
			isExec = true;
			p->SetQueueIndex(id);
			airport->checkinLineCV[exec]->Signal(
					airport->checkinLineLock[airline]);
			airport->checkinLineLock[airline]->Release();
		} else if (airport->checkinQueues[id]->Size() > 0) {
			//airport->checkinLock[id]->Acquire();
			airport->checkinState[id] = CI_BUSY;
			p = (Passenger*) airport->checkinQueues[id]->Remove();
			isExec = false;
			p->SetQueueIndex(id);
			airport->checkinLineCV[id]->Signal(
					airport->checkinLineLock[airline]);
			airport->checkinLineLock[airline]->Release();
		} else {
			airport->checkinState[id] = CI_BREAK;
			airport->checkinLineLock[airline]->Release();
			airport->checkinManagerLock->Acquire();
			airport->checkinBreakCV[id]->Wait(airport->checkinManagerLock);
		}

		//handle passenger
		if (p != NULL) {
			airport->checkinCV[id]->Wait(airport->checkinLock[id]);

			//start check-in
			airport->checkinLock[id]->Acquire();

			airport->airlineLock[airline]->Acquire();
			passengers++;
			bp.seatNum = airport->airlines[airline]->seatsAssigned;
			airport->airlines[airline]->seatsAssigned++;
			bp.gate = airline;
			p->SetBoardingPass(bp);

			// Process luggage and add it to conveyor.
			airport->conveyorLock->Acquire();
			while (!p->getLuggage()->IsEmpty()) {
				Luggage* bag = (Luggage*) p->getLuggage()->Remove();
				bag->airlineCode = airline;
				airport->conveyor->Append(bag);
				luggage++;
				weight += bag->weight;
			}
			airport->conveyorLock->Release();

			airport->checkinCV[id]->Signal(airport->checkinLock[id]);
			airport->checkinCV[id]->Wait(airport->checkinLock[id]);
		}

		/*if (airport->RequestingCheckinData[id]) {
		 airport->checkinLock[id];
		 airport->checkinManagerLock->Acquire();

		 // Give manager data
		 airport->checkinLock[id]->Acquire();
		 airport->checkinManagerCV->Signal(airport->checkinManagerLock);

		 airport->checkinManagerLock->Release();

		 airport->checkinDataCV[id]->Wait(airport->checkinLock[id]);
		 //Wait for manager to signal that all the data has been collected
		 airport->RequestingCheckinData[id] = false;
		 }*/

		// Check if all passengers are processed, close if done.
		airport->airlineLock[airline]->Acquire();
		if (airport->airlines[airline]->seatsAssigned
				>= airport->airlines[airline]->ticketsIssued) {
			airport->checkinLock[id]->Acquire();
			airport->checkinState[id] = CI_CLOSED;
			airport->airlineLock[airline]->Release();
			if (airport->finalCheckin[id])
				currentThread->Finish();
			else {
				//airport->checkinLock[id]->Acquire();
				airport->checkinManagerLock->Acquire();
				airport->checkinBreakCV[id]->Wait(airport->checkinManagerLock);
			}
			printf("Airline check-in staff %d is closing the counter\n", id);
			airport->finalCheckin[id] = true;
		} else
			airport->airlineLock[airline]->Release();

		if (airport->allFinished) {
			currentThread->Finish();
		}
	}
}
