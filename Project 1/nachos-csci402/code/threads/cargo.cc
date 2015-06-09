#include "cargo.h"

Cargo::Cargo(int id_, Airport* airport_)
{
	id = id_;
    airport = airport_;
    luggage = {0};
}

Cargo::~Cargo()
{
    airport = NULL;
}

void Cargo::Run()
{
    while (true)
    {
        airport->conveyorLock->Acquire();
        if(airport->conveyor->IsEmpty())
        {   // Conveyor is empty, go on break (sleep).
            printf("Cargo Handler %d is going for a break", id);
            airport->conveyorLock->Release();
            airport->cargoCV->Wait(/*lock*/);
        }
        else
        {   // Process bag and load onto airplane.
            Luggage* bag = (Luggage*)airport->conveyor->Remove();
            printf("Cargo Handler %d picked bag of airline %d with weighing %d lbs",
                    id, bag->airlineCode, bag->weight);
            airport->airplanes[bag->airlineCode]->Append(bag);
            luggage[bag->airlineCode]++;
            weight[bag->airlineCode] += bag->weight;
            airport->conveyorLock->Release();
        }
    }
}