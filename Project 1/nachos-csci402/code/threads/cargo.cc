#include "cargo.h"

Cargo::Cargo(int id, Airport* airport)
{
	this.id = id;
    this.airport = airport;
    this.luggage = {0};
}

Cargo::~Cargo()
{
    airport = NULL;
}

void Cargo::doStuff() // TODO: ADD SYNCHRONIZATION.
{
    while (true) // TODO: change this to avoid busy waiting.
    {
        // Check if the conveyor is empty. If yes, go on break (sleep).
        if(airport->conveyor->IsEmpty())
        {
            printf("Cargo Handler %d is going for a break", id);
            currentThread->Sleep();
        }
        // TODO: have manager wake it up.
        // Process bag and load onto airplane.
        Luggage* bag = (Luggage*)airport->conveyor->Remove();
        printf("Cargo Handler %d picked bag of airline %d with weighing %d lbs", id, bag->airlineCode, bag->weight);
        airport->airplanes[bag->airlineCode]->Append(bag);
        luggage[bag->airlineCode]++;
        weight[bag->airlineCode] += bag->weight;
    }
}