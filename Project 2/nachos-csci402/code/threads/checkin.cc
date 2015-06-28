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

/****this function is no longer being used in -manager test****/
Passenger* CheckIn::FindPassenger(int execLine) {
	Passenger* pass = NULL;
	if (!airport->checkinQueues[execLine]->IsEmpty()) { // Passenger in executive line.
		pass = (Passenger*) airport->checkinQueues[execLine]->Remove();
		printf(
				"Airline check-in staff %d of airline %d serves an executive class passenger and economy class line length = %d\n",
				id, airline, airport->checkinQueues[id]->Size());
	} else if (!airport->checkinQueues[id]->IsEmpty()) { // Passenger in economy line.
		pass = (Passenger*) airport->checkinQueues[id]->Remove();
		printf(
				"Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n",
				id, airline, airport->checkinQueues[execLine]->Size());
	} else {   // Both lines empty.
		airport->checkinLock[id]->Acquire();
		airport->checkinState[id] = CI_BREAK;
		airport->checkinLineLock[airline]->Release();
		airport->checkinBreakCV[id]->Wait(airport->checkinLock[id]);

		airport->checkinState[id] = CI_BUSY;
		airport->checkinLineLock[airline]->Acquire();
	}
	return pass;
}

void CheckIn::StartCheckInStaff() {
	int exec = airline * 6;
	//Passenger* p = NULL;
	//bool isExec = false;
	BoardingPass bp;

	while (true) {
		Passenger* p = NULL;
		bool isExec = false;

		//start 1st C.S.
		airport->checkinLineLock[airline]->Acquire();
		airport->checkinLock[id]->Acquire();
		//check executive line first
		if (airport->checkinQueues[exec]->Size() > 0) {
			airport->checkinState[id] = CI_BUSY;
			p = (Passenger*) airport->checkinQueues[exec]->Remove();
			isExec = true;
			p->SetQueueIndex(id);
			airport->checkinLineCV[exec]->Signal(
					airport->checkinLineLock[airline]);
			airport->checkinLineLock[airline]->Release();
		//then check my own line
		} else if (airport->checkinQueues[id]->Size() > 0) {
			airport->checkinState[id] = CI_BUSY;
			p = (Passenger*) airport->checkinQueues[id]->Remove();
			isExec = false;
			p->SetQueueIndex(id);
			airport->checkinLineCV[id]->Signal(
					airport->checkinLineLock[airline]);
			airport->checkinLineLock[airline]->Release();
		//if both empty wait on manager to wake me up
		} else {
			airport->checkinState[id] = CI_BREAK;
			airport->checkinLineLock[airline]->Release();
			airport->checkinManagerLock->Acquire();
			airport->checkinBreakCV[id]->Wait(airport->checkinManagerLock);
		}

		//handle passenger
		if (p != NULL) {
			airport->checkinCV[id]->Wait(airport->checkinLock[id]);  //end 1st C.S.

			//start check-in
			//start 2nd C.S.
			airport->checkinLock[id]->Acquire();

			//handle passenger info
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

			//tell passenger I'm done
			airport->checkinCV[id]->Signal(airport->checkinLock[id]);
			airport->checkinCV[id]->Wait(airport->checkinLock[id]);
		}//end 2nd C.S.

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
				airport->checkinManagerLock->Acquire();
				airport->checkinBreakCV[id]->Wait(airport->checkinManagerLock);
			}
			printf("Airline check-in staff %d is closing the counter\n", id);
			airport->finalCheckin[id] = true;
		} else
			airport->airlineLock[airline]->Release();

		//will only enter this after all passengers boarded
		if (airport->allFinished) {
			currentThread->Finish();
		}
	}
}
