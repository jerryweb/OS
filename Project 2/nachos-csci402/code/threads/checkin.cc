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

/*void CheckIn::StartCheckInStaff() {
 Passenger* pass = NULL;
 bool exec;
 bool talked = false;
 int execLine = airline * 6;
 BoardingPass bp;
 while (true) {
 airport->checkinLineLock[airline]->Acquire();
 if (airport->checkinState[id] != CI_CLOSED) {
 pass = FindPassenger(execLine);
 }

 airport->checkinLock[id]->Acquire();
 airport->checkinLineLock[airline]->Release();
 // Process ticket, create boarding pass.
 if (pass != NULL) {
 airport->airlineLock[airline]->Acquire();
 exec = pass->getTicket().executive;
 airport->checkinManagerLock->Acquire();
 passengers++;
 airport->checkinManagerLock->Release();
 bp.seatNum = airport->airlines[airline]->seatsAssigned;
 airport->airlines[airline]->seatsAssigned++;
 bp.gate = airline;
 pass->SetBoardingPass(bp);

 if (exec) {
 printf(
 "Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n",
 id, airline, pass->getID(),
 pass->GetBoardingPass().gate);
 airport->checkinLineCV[execLine]->Signal(
 airport->checkinLineLock[airline]);
 } else {
 printf(
 "Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n",
 id, airline, pass->getID(),
 pass->GetBoardingPass().gate);
 airport->checkinLineCV[id]->Signal(
 airport->checkinLineLock[airline]);
 }
 airport->airlineLock[airline]->Release();
 // Process luggage and add it to conveyor.
 airport->conveyorLock->Acquire();
 while (!pass->getLuggage()->IsEmpty()) {
 Luggage* bag = (Luggage*) pass->getLuggage()->Remove();
 bag->airlineCode = airline;
 airport->conveyor->Append(bag);
 airport->checkinManagerLock->Acquire();
 luggage++;
 airport->checkinManagerLock->Release();
 weight += bag->weight;
 }
 printf(
 "Airline check-in staff %d of airline %d dropped bags to the conveyor system\n",
 id, airline);
 pass = NULL;
 airport->conveyorLock->Release();
 }

 if (airport->RequestingCheckinData[id]) {
 airport->checkinManagerLock->Acquire();

 // Give manager data

 airport->checkinLock[id]->Acquire();
 airport->checkinManagerCV->Signal(airport->checkinManagerLock);

 airport->checkinManagerLock->Release();

 airport->checkinCV[id]->Wait(airport->checkinLock[id]);
 //Wait for manager to signal that all the data has been collected
 airport->RequestingCheckinData[id] = false;
 }

 // Check if all passengers are processed, close if done.
 airport->airlineLock[airline]->Acquire();
 if (airport->airlines[airline]->seatsAssigned
 >= airport->airlines[airline]->ticketsIssued) {
 airport->checkinLock[id]->Acquire();
 airport->checkinState[id] = CI_CLOSED;
 airport->airlineLock[airline]->Release();
 if (airport->finalCheckin[id])
 currentThread->Finish();
 else
 airport->checkinBreakCV[id]->Wait(airport->checkinLock[id]);
 printf("Airline check-in staff %d is closing the counter\n", id);
 airport->airlineLock[airline]->Acquire();
 airport->finalCheckin[id] = true;
 }
 airport->airlineLock[airline]->Release();

 }
 }*/

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
