#include "cargo.h"

Cargo::Cargo(int id_, Airport* airport_)
{
	id = id_;
    airport = airport_;
    luggage = new int[airport->numAirlines];
    weight = new int[airport->numAirlines];
    for(int i = 0; i < airport->numAirlines; i++){
        luggage[i] = 0;
        weight[i] = 0;
    }
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
            airport->cargoLock[id]->Acquire();
            airport->cargoState[id] = C_BREAK;
            printf("Cargo Handler %d is going for a break\n", id);
            airport->conveyorLock->Release();
            // airport->cargoCV->Wait(airport->cargoLock);
            airport->cargoDataCV[id]->Wait(airport->cargoLock[id]);
            printf("Cargo Handler %d returned from break\n", id);
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
        if(airport->RequestingCargoData[id]){
            airport->CargoHandlerManagerLock->Acquire();
            //Give manager data
           
            airport->cargoDataLock[id]->Acquire();
            airport->cargoManagerCV[id]->Signal(airport->CargoHandlerManagerLock);

            airport->CargoHandlerManagerLock->Release();
            airport->cargoDataCV[id]->Wait(airport->cargoDataLock[id]);
            //Wait for manager to signal that all the data has been collected
            airport->RequestingCargoData[id] = false;
        }
    }
}
