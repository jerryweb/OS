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

void Liaison::DirectPassengers() {
	while (true) {
		Passenger* p = NULL;
		airport->liaisonLineLock->Acquire();
		if (airport->liaisonQueues[id]->Size() > 0) {
			p = (Passenger*) airport->liaisonQueues[id]->Remove();
			airport->liaisonLock[id]->Acquire();
			airport->liaisonState[id] = L_BUSY;
			airport->liaisonLineCV[id]->Signal(airport->liaisonLineLock);
			airport->liaisonLineLock->Release();
			//wait passenger to start service
			airport->liaisonCV[id]->Wait(airport->liaisonLock[id]);
			airport->liaisonLock[id]->Acquire();

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

			airport->liaisonCV[id]->Wait(airport->liaisonLock[id]);
		} else if (airport->allFinished) {
			currentThread->Finish();
		} else {
			airport->liaisonState[id] = L_FREE;
			airport->liaisonLineLock->Release();
			currentThread->Yield();
		}
	}
}
