#include "cargo.h"

Cargo::Cargo(int id_, Airport* airport_)
{
	id = id_;
    airport = airport_;
    luggage = new int[airport->numAirlines];
    weight = new int[airport->numAirlines];
}

Cargo::~Cargo()
{
    delete luggage;
    delete weight;
}

void Cargo::StartCargo()
{
    Luggage* bag;
    while (true)
    {
        airport->conveyorLock->Acquire();
        if(airport->conveyor->IsEmpty())
        {   // Conveyor is empty, go on break (sleep).
            printf("Cargo Handler %d is going for a break\n", id);
            airport->cargoState[id] = C_BREAK;
            airport->cargoLock->Acquire();
            airport->conveyorLock->Release();
            airport->cargoCV->Wait(airport->cargoLock);
            airport->cargoState[id] = C_BUSY;
        }
        else
        {   // Process bag and load onto airplane.
            bag = (Luggage*)airport->conveyor->Remove();
            printf("Cargo Handler %d picked bag of airline %d weighing %d lbs\n",
                    id, bag->airlineCode, bag->weight);
            airport->aircraft[bag->airlineCode]->Append(bag);
            luggage[bag->airlineCode]++;
            weight[bag->airlineCode] += bag->weight;
            airport->conveyorLock->Release();
        }
    }
}
