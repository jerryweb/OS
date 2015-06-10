#include "airport.h"

Airport::Airport()
{
    int i;
    // TODO: add everything and non-hardcode it
    
    // General variables
    numAirlines = 3;
    airlines = new Airline*[numAirlines];
    airlineLock = new Lock*[numAirlines];
    for (i = 0; i < numAirlines; i++)
    {
        airlines[i] = new Airline(i, 0, 0);
        airlineLock[i] = new Lock("airlineLock" + i);
    }
    
    //Passenger variables
    passengerList = new List();
    
    // Liaison variables
    liaisonLineLock = new Lock("liaisonLineLock");
    liaisonList = new List();
    for (i = 0; i < 7; i++)
    {
        liaisonQueues[i] = new List();
        liaisonLineCV[i] = new Condition("liaisonLineCV" + i);
        liaisonCV[i] = new Condition("liaisonCV" + i);
        liaisonLock[i] = new Lock("liaisonLock" + i);
        liaisonState[i] = L_BUSY;
    }
    
    // Check-in variables
    int numCheckin = numAirlines * 6;
    checkinQueues = new List*[numCheckin];
    checkinLineLock = new Lock*[numAirlines];
    checkinLock = new Lock*[numCheckin];
    checkinCV = new Condition*[numCheckin];
    checkinBreakCV = new Condition*[numCheckin];
    for (i = 0; i < numAirlines; i++)
    {
        checkinLineLock[i] = new Lock("checkinLineLock" + i);
    }
    for (i = 0; i < numCheckin; i++)
    {
        checkinQueues[i] = new List();
        checkinLock[i] = new Lock("checkinLock" + i);
        checkinCV[i] = new Condition("checkinCV" + i);
        checkinBreakCV[i] = new Condition("checkinBreakCV" + i);
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
    delete[] liaisonLineCV;
    delete[] liaisonCV;
    delete[] liaisonLock;
    // Cargo variables
    delete conveyorLock;
    delete conveyor;
    delete cargoCV;
    delete[] aircraft;
}
