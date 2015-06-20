#include "airport.h"

Airport::Airport(){
    int i;


	// General variables
	numAirlines = 3;
	airlines = new Airline*[numAirlines];
	airlineLock = new Lock*[numAirlines];
    boardingQueue = new List*[numAirlines];
    boardingCV = new Condition*[numAirlines];
    boardingLock = new Lock*[numAirlines];

	for (i = 0; i < numAirlines; i++) {
		airlines[i] = new Airline(i, 0, 0, 0);
		airlineLock[i] = new Lock("airlineLock");
        boardingQueue[i] = new List();
        boardingCV[i] = new Condition("boardingCV");
        boardingLock[i] = new Lock("boardingLock");
	}

	//Passenger variables
	passengerList = new List();

	// Liaison variables
	RequestingLiaisonData = new bool[7];
	liaisonManagerLock = new Lock("liaisonManagerLock");
	liaisonLineLock = new Lock("liaisonLineLock");
	liaisonManagerCV = new Condition("liaisonManagerCV");
	liaisonList = new List();
	for (i = 0; i < 7; i++) {
		liaisonQueues[i] = new List();
		liaisonLineCV[i] = new Condition("liaisonLineCV");
		RequestingLiaisonData[i] = false;
		liaisonCV[i] = new Condition("liaisonCV");
		liaisonLock[i] = new Lock("liaisonLock");
		liaisonState[i] = L_BUSY;
	}

	// Check-in variables

	int numCheckin = numAirlines * 6;
    finalCheckin = new bool[numCheckin];
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
	for (i = 0; i < numAirlines; i++) {
		checkinLineLock[i] = new Lock("checkinLineLock");
	}
    for (i = 0; i < numCheckin; i++)
    {
        checkinQueues[i] = new List();
        checkinLock[i] = new Lock("checkinLock");
        checkinLineCV[i] = new Condition("checkinLineCV");
        checkinBreakCV[i] = new Condition("checkinBreakCV");
        checkinCV[i] = new Condition("checkinCV");
        checkinState[i] = CI_BUSY;
        finalCheckin[i] = false;
        RequestingCheckinData[i] = false;
    }

	// Cargo variables
	RequestingCargoData = new bool[10];
	cargoHandlerList = new List();
	conveyor = new List();
	conveyorLock = new Lock("conveyorLock");
	CargoHandlerManagerLock = new Lock("CargoHandlerManagerLock");
	cargoCV = new Condition("cargoCV");
	for (i = 0; i < 10; i++) {
		cargoState[i] = C_BUSY;
		cargoDataCV[i] = new Condition("cargoDataCV");
		cargoManagerCV[i] = new Condition("cargoManagerCV");
		cargoDataLock[i] = new Lock("cargoDataLock");
		RequestingCargoData[i] = false;
		cargoLock[i] = new Lock("cargoLock");
	}
	aircraft = new List*[numAirlines];
	for (i = 0; i < numAirlines; i++) {
		aircraft[i] = new List();
	}

	//Screen & Security Variables

	screenOfficerList = new List();
	securityInspectorList = new List();

	screenQueuesLock = new Lock("screenQueuesLock");
	securityQueuesLock = new Lock("securityQueuesLock");

	screenLocks = new Lock*[3];
	securityLocks = new Lock*[3];
    screenState = new ScreenState[3];
    securityState = new SecurityState[3];
	screenQueues = new List*[3];
	securityQueues = new List*[3];
	returnQueues = new List*[3];
    screenCV = new Condition*[3];
    screenQueuesCV = new Condition*[3];
    securityQueuesCV = new Condition*[3];
    returnQueuesCV = new Condition*[3];
    securityFinishCV = new Condition*[3];
    screenFreeCV = new Condition*[3];
    securityFreeCV = new Condition*[3];

    //endLock = new Lock("endLock");
    //endCV = new Condition*[3];

	for (i = 0; i < 3; i++) {
		screenLocks[i] = new Lock("screenLocks");
       
		securityLocks[i] = new Lock("securityLocks");
        

        screenState[i] = SO_BUSY;   // Array of states for each liaison.
        securityState[i] = SC_BUSY;

        screenQueues[i] = new List();
        securityQueues[i] = new List();
		returnQueues[i] = new List();

        screenCV[i] = new Condition("screenCV");
        screenFreeCV[i] = new Condition("screenFreeCV");
        securityFinishCV[i] = new Condition("securityFinishCV");
        securityFreeCV[i] = new Condition("securityFreeCV");

        screenQueuesCV[i] = new Condition("screenQueuesCV");
        securityQueuesCV[i] = new Condition("securityQueuesCV");
        returnQueuesCV[i] = new Condition("returnQueuesCV");

        //endCV[i] = new Condition("endCV");
	
	}

}

Airport::Airport(int airlineNum, int passengers, int liaisons, int checkins, int security, int cargos)
{
    int i;
    
    // General variables
    numAirlines = airlineNum;
    airlines = new Airline*[numAirlines];
    airlineLock = new Lock*[numAirlines];
    
    for (i = 0; i < numAirlines; i++)
    {
        airlines[i] = new Airline(i, 0, 0, 0);
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
        if (i >= liaisons) liaisonState[i] = L_NONE;
        else liaisonState[i] = L_BUSY;
    }

    // Check-in variables
    
    int numCheckin = numAirlines * 6;
    finalCheckin = new bool[numCheckin];
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
        checkinLineCV[i] = new Condition("checkinLineCV");
        checkinBreakCV[i] = new Condition("checkinBreakCV");
        checkinCV[i] = new Condition("checkinCV");
        if (i >= checkins) checkinState[i] = CI_NONE;
        else checkinState[i] = CI_BUSY;
        finalCheckin[i] = false;
        RequestingCheckinData[i] = false;
    }

    // Cargo variables
    RequestingCargoData = new bool[10];
    cargoHandlerList = new List();
    conveyor = new List();
    conveyorLock = new Lock("conveyorLock");
    CargoHandlerManagerLock = new Lock("CargoHandlerManagerLock");
    cargoCV = new Condition("cargoCV");
    for (i = 0; i < 10; i++)
    {
        if (i >= cargos) cargoState[i] = C_NONE;
        else cargoState[i] = C_BUSY;
        cargoLock[i] = new Lock("cargoLock");
        cargoDataCV[i] = new Condition("cargoDataCV");
        cargoManagerCV[i] = new Condition("cargoManagerCV");
        cargoDataLock[i] = new Lock("cargoDataLock");
        RequestingCargoData[i] = false;
		cargoLock[i] = new Lock("cargoLock");
    }
    aircraft = new List*[numAirlines];
    for (i = 0; i < numAirlines; i++)
    {
        aircraft[i] = new List();
    }

	//Screen & Security Variables
    screenOfficerList = new List();
    securityInspectorList = new List();
	screenQueuesLock = new Lock("screenQueuesLock");
	securityQueuesLock = new Lock("securityQueuesLock");
	screenQueues = new List*[security];
	securityQueues = new List*[security];
	returnQueues = new List*[security];
	screenLocks = new Lock*[security];
	securityLocks = new Lock*[security];
    securityFinishCV = new Condition*[security];
    screenCV = new Condition*[security];
	for (i = 0; i < security; i++) {

		screenQueues[i] = new List;
		securityQueues[i] = new List;
		returnQueues[i] = new List;
		screenLocks[i] = new Lock("screenLocks");
		securityLocks[i] = new Lock("securityLocks");
		screenCV[i] = new Condition("screenCV");
        securityFinishCV[i] = new Condition("securityFinishCV");
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
    delete[] securityLocks;
    delete securityQueuesLock;
    delete[] screenCV;
    delete[] securityFinishCV;
}