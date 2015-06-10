#include "airport.h"

Airport::Airport()
{
    int i;
    // TODO: add everything and non-hardcode it
    //Passenger variables
    passengerList = new List();
    numAirlines = 3;
    // Liaison variables
    liaisonLineLock = new Lock("liaisonLineLock");
    liaisonList = new List();
    for (i = 0; i < 7; i++)
    {
        liaisonQueues[i] = new List();
        lineCV[i] = new Condition("lineCV" + i);
        liaisonCV[i] = new Condition("LiaisonCV" + i);
        liaisonLock[i] = new Lock("liaisonLock" + i);
        liaisonState[i] = L_BUSY;
    }
    // Cargo variables
    conveyorLock = new Lock("conveyorLock");
    conveyor = new List();
    cargoCV = new Condition("cargoCV");
    cargoLock = new Lock("cargoLock");
    for (i = 0; i < 10; i++)
    {
        cargoState[i] = C_BUSY;
    }
    aircraft = new List*[numAirlines];
    for (i = 0; i < numAirlines; i++)
    {
        aircraft[i] = new List();
    }
}

Airport::~Airport()
{
    // Liaison variables
    delete liaisonLineLock;
    delete[] liaisonQueues;
    delete[] lineCV;
    delete[] liaisonCV;
    delete[] liaisonLock;
    // Cargo variables
    delete conveyorLock;
    delete conveyor;
    delete cargoCV;
    delete[] aircraft;
}
