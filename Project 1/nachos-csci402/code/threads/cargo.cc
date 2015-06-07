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

void Cargo::doStuff()
{
    while (true) // need to change this
    {
        if(airport->conveyor->IsEmpty())
        {
            printf("Cargo Handler %d is going for a break", id);
            currentThread->Sleep();
        }
        // manager wakes it up
        Luggage* bag = (Luggage*)airport->conveyor->Remove();
        printf("Cargo Handler %d picked bag of airline %d with weighing %d lbs", id, bag->airlineCode, bag->weight);
        airport->airplanes[bag->airlineCode]->Append(bag);
        luggage[bag->airlineCode]++;
        weight[bag->airlineCode] += bag->weight;
    }
}