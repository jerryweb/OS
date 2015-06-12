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
    RequestingLiaisonData = new bool[7];
    liaisonManagerLock = new Lock("liaisonManagerLock");
    liaisonLineLock = new Lock("liaisonLineLock");
    liaisonManagerCV = new Condition("liaisonManagerCV");
    liaisonList = new List(); 
    for (i = 0; i < 7; i++)
    {
        liaisonQueues[i] = new List();
        liaisonLineCV[i] = new Condition("liaisonLineCV");
        RequestingLiaisonData[i] = false;
        liaisonCV[i] = new Condition("liaisonCV");
        liaisonLock[i] = new Lock("liaisonLock");
        liaisonState[i] = L_BUSY;
    }

    // Check-in variables
    
    int numCheckin = numAirlines * 6;
    RequestingCheckinData= new bool[numCheckin];
    checkInStaffList = new List();
    checkinQueues = new List*[numCheckin];
    checkinLineLock = new Lock*[numAirlines];
    checkinLock = new Lock*[numCheckin];
    checkinLineCV = new Condition*[numCheckin];
    checkinCV = new Condition*[numCheckin];
    checkinBreakCV = new Condition*[numCheckin];
    checkinState = new CheckinState[numCheckin];
    checkinManagerLock = new Lock("checkinManagerLock");
    checkinManagerCV = new Condition("checkinManagerCV");
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
        checkinCV[i] = new Condition("checkinCV");
        checkinState[i] = CI_BUSY;
        RequestingCheckinData[i] = false;
    }

    // Cargo variables
    RequestingCargoData = new bool[10];
    cargoHandlerList = new List();
    conveyor = new List();
    conveyorLock = new Lock("conveyorLock");
    CargoHandlerManagerLock = new Lock("CargoHandlerManagerLock");
    cargoCV = new Condition("cargoCV");
    cargoLock = new Lock("cargoLock");
    for (i = 0; i < 10; i++)
    {
        cargoState[i] = C_BUSY;
        cargoDataCV[i] = new Condition("cargoDataCV");
        cargoManagerCV[i] = new Condition("cargoManagerCV");
        cargoDataLock[i] = new Lock("cargoDataLock");
    }
    aircraft = new List*[numAirlines];
    for (i = 0; i < numAirlines; i++)
    {
        aircraft[i] = new List();
    }

	//Screen & Security Variables
    screeningOfficerList = new List();
    securityInspectorList = new List();
	screenQueues = new List*[3];
	for (i = 0; i < 3; i++) {
		screenQueues[i] = new List;
	}

	securityQueues = new List*[3];
	for (i = 0; i < 3; i++) {
		securityQueues[i] = new List;
	}

	returnQueues = new List*[3];
	for (i = 0; i < 3; i++) {
		returnQueues[i] = new List;
	}

	screenLocks = new Lock*[3];
	for (i = 0; i < 3; i++) {
		screenLocks[i] = new Lock("screenLocks");
	}
	screenQueuesLock = new Lock("screenQueuesLock");

	passengerWaitOfficerCV = new Condition*[3];
	for (i = 0; i < 3; i++) {
		passengerWaitOfficerCV[i] = new Condition("passengerWaitOfficerCV");
	}

	officerWaitPassengerCV = new Condition*[3];
	for (i = 0; i < 3; i++) {
		officerWaitPassengerCV[i] = new Condition("officerWaitPassengerCV");
	}

	securityLocks = new Lock*[3];
	for (i = 0; i < 3; i++) {
		securityLocks[i] = new Lock("securityLocks");
	}
	securityQueuesLock = new Lock("securityQueuesLock");

	inspectorWaitRePassengerCV = new Condition*[3];
	for (i = 0; i < 3; i++) {
		inspectorWaitRePassengerCV[i] = new Condition(
				"inspectorWaitRePassengerCV");
	}

	rePassengerWaitInspectorCV = new Condition*[3];
	for (i = 0; i < 3; i++) {
		rePassengerWaitInspectorCV[i] = new Condition(
				"rePassengerWaitInspectorCV");
	}

	inspectorWaitPassengerCV = new Condition*[3];
	for (i = 0; i < 3; i++) {
		inspectorWaitPassengerCV[i] = new Condition("inspectorWaitPassengerCV");
	}

	passengerWaitInspectorCV = new Condition*[3];
	for (i = 0; i < 3; i++) {
		passengerWaitInspectorCV[i] = new Condition("passengerWaitInspectorCV");
	}

	appendReturnLineCV = new Condition*[3];
	for (i = 0; i < 3; i++) {
		appendReturnLineCV[i] = new Condition("appendReturnLineCV");
	}

	updateClearCount = new Lock("updateClearCount");

	lastCV = new Condition*[3];
	for (i=0;i<3;i++) {
		lastCV[i] = new Condition("lastCV");
	}

}

Airport::~Airport() {
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
	//security variables
	delete[] screenQueues;
	delete[] securityQueues;
	delete[] screenLocks;
	delete screenQueuesLock;
	delete[] passengerWaitOfficerCV;
	delete[] officerWaitPassengerCV;
	delete[] securityLocks;
	delete securityQueuesLock;
	delete[] passengerWaitInspectorCV;
	delete[] inspectorWaitPassengerCV;
	delete[] rePassengerWaitInspectorCV;
	delete[] inspectorWaitRePassengerCV;
}
