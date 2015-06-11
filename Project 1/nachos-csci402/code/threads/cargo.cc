#include "cargo.h"

Cargo::Cargo(int id_, Airport* airport_)
{
	id = id_;
    airport = airport_;
    luggage = new int[airport->numAirlines];
    weight = new int[airport->numAirlines];
    airport->RequestingCargoData[id] = false;
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
            airport->cargoLock->Acquire();
            airport->cargoState[id] = C_BREAK;
            printf("Cargo Handler %d is going for a break\n", id);
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
        if(airport->RequestingCargoData[id]){
            printf("heyd\n");
            airport->CargoHandlerManagerLock->Acquire();
            printf("Cargo Handler %d is sending data.\n", id);
            
            //Give manager data
            airport->cargoManagerCV[id]->Signal(airport->CargoHandlerManagerLock);
            airport->cargoDataLock[id]->Acquire();
            airport->CargoHandlerManagerLock->Release();
            airport->cargoDataCV[id]->Wait(airport->cargoLock);

            //Wait for manager to signal that all the data has been collected
            airport->cargoDataLock[id]->Acquire();
            printf("Cargo Handler %d has finished reporting data to manager.\n", id);
            airport->cargoDataLock[id]->Release();
            airport->RequestingCargoData[id] = false;
        }
    }
}
