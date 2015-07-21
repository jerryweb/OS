#include "liaison.h"

Liaison::Liaison(int id_, Airport* airport_) {
	id = id_;
	airport = airport_;
	passengers = new int[airport->numAirlines];
	luggageCount = new int[airport->numAirlines];
	totalLuggageWeight = new int[airport->numAirlines];     //May not be needed
	for (int i = 0; i < airport->numAirlines; i++) {
		passengers[i] = 0;
		luggageCount[i] = 0;
	}
	airport->liaisonState[id] = L_BUSY;
}

Liaison::~Liaison() {
	delete[] passengers;
	delete[] totalLuggageWeight;
}

/****this function is no longer being used in -manager test****/
// Finds the first passenger in the list (if applicable).
Passenger* Liaison::CheckForPassengers() {
	Passenger* p;
	if (airport->liaisonQueues[id]->Size() > 0) { // If line is not empty, signal next passenger.
		airport->liaisonState[id] = L_BUSY;
		airport->liaisonLineLock->Acquire();
		p = (Passenger*) airport->liaisonQueues[id]->Remove();
		airport->liaisonLineLock->Release();
		printf("Airport Liaison %d directed passenger %d of airline %d\n", id,
				p->getID(), p->getTicket().airline);
		p->SetAirline(p->getTicket().airline);
		airport->liaisonLineCV[id]->Signal(airport->liaisonLock[id]);
	} else { // If line is empty, do nothing;
		p = NULL;
		airport->liaisonState[id] = L_FREE;
	}
	return p;
}

//----------------------------------------------------------------------
//  This Function handles the interaction between the Liaison and the Passengers
//  First the Liaison will acquire the lock and see if there are any Passengers 
//  waiting in his respective queue. The liaison will tell the passenger which 
// airline check-in counter to go to, then remove that passenger from the liaison's 
// queue
//----------------------------------------------------------------------

void Liaison::DirectPassengers() {
	while (true) {
		Passenger* p = NULL;
		//start first C.S.
		airport->liaisonLineLock->Acquire();
		if (airport->liaisonQueues[id]->Size() > 0) {
			p = (Passenger*) airport->liaisonQueues[id]->Remove();
			airport->liaisonLock[id]->Acquire();
			airport->liaisonState[id] = L_BUSY;
			airport->liaisonLineCV[id]->Signal(airport->liaisonLineLock);
			airport->liaisonLineLock->Release();
			//wait passenger to start service
			airport->liaisonCV[id]->Wait(airport->liaisonLock[id]); //ending first C.S.

			//starting 2nd C.S.
			airport->liaisonLock[id]->Acquire();

			//acquire data lock to update data
			airport->liaisonManagerLock->Acquire();
			p->SetAirline(p->getTicket().airline);

			passengers[p->getTicket().airline]++;

			List *bags = p->getLuggage(); //Temp list for iterating through luggage
			for (int j = bags->Size(); j > 0; j--) { //This calculates the weights of each of the bags
													 //and puts it into a temp array to be read
				luggageCount[p->getTicket().airline]++;
			}
			airport->liaisonManagerLock->Release();

			airport->liaisonCV[id]->Signal(airport->liaisonLock[id]);

			printf("Airport Liaison %d directed passenger %d of airline %d\n",
					id, p->getID(), p->getTicket().airline);

			airport->liaisonCV[id]->Wait(airport->liaisonLock[id]); //ending 2nd C.S.

		//allFinished only be called when all passenger has boarded
		} else if (airport->allFinished) {
			currentThread->Finish();

		//if line is empty
		} else {
			airport->liaisonState[id] = L_FREE;
			airport->liaisonLineLock->Release();
			currentThread->Yield();
		}
	}
}
