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
        airlineLock[i] = new Lock("airlineLock");
    }
    
    //Passenger variables
    passengerList = new List();

    // Liaison variables
    liaisonManagerLock = new Lock("liaisonManagerLock");
    liaisonLineLock = new Lock("liaisonLineLock");
    liaisonList = new List(); 
    for (i = 0; i < 7; i++)
    {
        liaisonQueues[i] = new List();
        liaisonLineCV[i] = new Condition("liaisonLineCV");
        liaisonManagerCV[i] = new Condition("liaisonManagerCV");
        liaisonCV[i] = new Condition("liaisonCV");
        liaisonLock[i] = new Lock("liaisonLock");
        liaisonDataLock[i] = new Lock("liaisonDataLock");
        liaisonState[i] = L_BUSY;
    }

    // Check-in variables
    int numCheckin = numAirlines * 6;
    checkInStaffList = new List();
    checkinQueues = new List*[numCheckin];
    checkinLineLock = new Lock*[numAirlines];
    checkinLock = new Lock*[numCheckin];
    checkinLineCV = new Condition*[numCheckin];
    checkinBreakCV = new Condition*[numCheckin];
    checkinState = new CheckinState[numCheckin];
    for (i = 0; i < numAirlines; i++)
    {
        checkinLineLock[i] = new Lock("checkinLineLock");
    }
    for (i = 0; i < numCheckin; i++)
    {
        checkinQueues[i] = new List();
        checkinLock[i] = new Lock("checkinLock");
        checkinLineCV[i] = new Condition("checkinCV");
        checkinBreakCV[i] = new Condition("checkinBreakCV");
        checkinState[i] = CI_BUSY;
    }

    // Cargo variables
    cargoHandlerList = new List();
    conveyor = new List();
    conveyorLock = new Lock("conveyorLock");
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

    managerLock = new Lock("managerLock");
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
