#include "screenofficer.h"
#include "stdlib.h"
#include "time.h"

Screenofficer::ScreenOfficer(int id)
{
	this.id = id;
}

Screenofficer::~ScreenOfficer() {

}

void Screenofficer::Screen(int* liaisonList[7]) {
	//keep checking his own line
	while (true) {
		if (screenLine.size != 0) {
			//starting C.S. for screening
			Passenger* currentPassenger = (Passenger*) screenLine.First();

			//assume 10% chance fail the screening
			srand(time(NULL));
			int randNum = rand() % 100 + 1;
			if (randNum > 0 && randNum < 11) {
				currentPassenger->SetScreenPass(false);
				printf(
						"Screening officer %d is suspicious of the hand luggage of passenger %d\n",
						this.id, currentPassenger->GetID());
			} else {
				printf(
						"Screening officer %d is not suspicious of the hand luggage of passenger %d\n",
						this.id, currentPassenger->GetID());
			}

			//remove the Passenger in current line
			screenLine.remove();
		}

		//ending C.S. for screening

		//starting C.S. for assigning security inspector

		int shortest = 0; //shortest line's id
		int minimumSize = -1;  //for comparsion in the following loop

		for (int i = 0; i < 3; i++) {
			if (minimumSize < 0 || minimumSize > screenQueues[i].Size()) {
				minimumSize = screenQueues[i].Size();
				shrotest = i;
			}
		}

		securityQueues[shortest].Append(currentPassenger);

		//Assuming the index in securityQueues array is the same as security inspector's id
		printf(
				"Screening officer %d directs passenger %d to security inspector %d\n",
				this.id, currentPassenger->GetID(), shortest);

		//ending C.S. for assigning security inspector
	}
}
