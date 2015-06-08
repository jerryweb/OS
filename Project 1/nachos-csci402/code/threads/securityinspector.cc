#include "securityinspector.h"
#include "stdlib.h"
#include "time.h"

Securityinspector::Securityinspector(int id)
{
	this.id = id;
}

Securityinspector::~Securityinspector() {

}

void Securityinspector::SecurityCheck(int* liaisonList[7]) {
	while (true) {
		//starting C.S.

		Passenger* currentPassenger;

		//handle return line first
		if (returnLine.Size() != 0) {
			currentPassenger = (Passenger*) returnLine.First();
			currentPassenger->SetSecurityPass(true);
			returnLine.Remove();
			printf(
					"Security inspector %d permits returning passenger %d to board\n",
					this.id, currentPassenger->GetID());
			//signal the passenger thread here

			//if return line is empty handle the security line
		} else if (securityLine.Size() != 0) {
			currentPassenger = (Passenger*) securityLine.First();

			//this flag use locally in this function
			bool passFlag = currentPassenger->GetSecurityPass();

			//if it's orginally clean from screening do wand-hand test
			if (passFlag) {
				//assume 10% chance fail the screening
				srand(time(NULL));
				int randNum = rand() % 100 + 1;
				if (randNum > 0 && randNum < 11) {
					currentPassenger->SetSecurityPass(false);
					passFlag = false;
				}
			}

			//make decision based on the result above
			if (!passFlag) {
				securityLine.Remove();
				returnLine.Append(currentPassenger);
				currentPassenger->Questioning();

				printf(
						"Security inspector %d is suspicious of the passenger %d\n",
						this.id, currentPassenger->GetID());
				printf(
						"Security inpector %d asks passenger %d to go for further examination\n",
						this.id, currentPassenger->GetID());
			} else {
				securityLine.Remove();
				//signal the passenger thread here

				printf(
						"Security inspector %d is not suspicious of the passenger %d\n",
						this.id, currentPassenger->GetID());
				printf("Security inspector %d allows passenger %d to board\n",
						this.id, currentPassenger->GetID());
			}

		}
	}
}

int Securityinspector::GetID() {
	return this.id;
}
