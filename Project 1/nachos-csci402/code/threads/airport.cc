#include "airport.h"

Airport::Airport()
{
    // TODO: add everything and non-hardcode it
    numAirlines = 3;
    conveyorLock = new Lock("conveyorLock");
    conveyor = new List();
    cargoCV = new Condition("cargoCV");
    airplanes = new List*[numAirlines];
    for (int i = 0; i < numAirlines; i++)
    {
        airplanes[i] = new List();
    }
}

Airport::~Airport()
{
    delete conveyorLock;
    delete conveyor;
    delete cargoCV;
    delete[] airplanes;
}