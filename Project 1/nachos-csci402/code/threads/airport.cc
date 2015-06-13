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
		airlines[i] = new Airline(i, 0, 0);
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
printf("onto checkin\n");
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
printf("cargo\n");
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
printf("security\n");
	//Screen & Security Variables
	clearPassengerCount = new int[3];
	for (i = 0; i < 3; i++) {
		clearPassengerCount[i] = 0;
	}

	updateClearCount = new Lock("updateClearCount");

	screeningOfficerList = new List();
	securityInspectorList = new List();

	screenQueuesLock = new Lock("screenQueuesLock");
	securityQueuesLock = new Lock("securityQueuesLock");

    screenLineLock = new Lock("screenLineLock");
    //securityLineLock = new Lock("securityLineLock")


	screenLocks = new Lock*[3];
	securityLocks = new Lock*[3];
    screenState = new ScreenState[3];
	screenQueues = new List*[3];
	securityQueues = new List*[3];
	returnQueues = new List*[3];
	passengerWaitOfficerCV = new Condition*[3];
	officerWaitPassengerCV = new Condition*[3];
	inspectorWaitRePassengerCV = new Condition*[3];
	rePassengerWaitInspectorCV = new Condition*[3];
	inspectorWaitPassengerCV = new Condition*[3];
	passengerWaitInspectorCV = new Condition*[3];
	inspectorWaitQuestioningCV = new Condition*[3];
	liaWaitPassengerCV = new Condition*[7];
    screenlineCV = new Condition*[3];
	passengerWaitLiaCV = new Condition*[7];

	for (i=0;i<7;i++) {
		liaWaitPassengerCV[i] = new Condition("liaWaitPassengerCV");
		passengerWaitLiaCV[i] = new Condition("passWaitLiaCV");
	}

	for (i = 0; i < 3; i++) {
		screenLocks[i] = new Lock("screenLocks");
       
		securityLocks[i] = new Lock("securityLocks");
        

        screenState[i] = SO_BUSY;   // Array of states for each liaison.

        screenQueues[i] = new List;
        securityQueues[i] = new List;
		returnQueues[i] = new List;

		// passengerWaitOfficerCV[i] = new Condition("passengerWaitOfficerCV");
		// officerWaitPassengerCV[i] = new Condition("officerWaitPassengerCV");
        screenlineCV[i] = new Condition("screenlineCV");
		// inspectorWaitRePassengerCV[i] = new Condition(
		// 		"inspectorWaitRePassengerCV");
		// rePassengerWaitInspectorCV[i] = new Condition(
		// 		"rePassengerWaitInspectorCV");
		// inspectorWaitPassengerCV[i] = new Condition("inspectorWaitPassengerCV");
		// passengerWaitInspectorCV[i] = new Condition("passengerWaitInspectorCV");
		// inspectorWaitQuestioningCV[i] = new Condition(
		// 		"inspectorWaitQuestioningCV");
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
        airlines[i] = new Airline(i, NULL, 0);
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
    checkinQueuesLock = new Lock("checkinQueuesLock");
    
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
    screeningOfficerList = new List();
    securityInspectorList = new List();
	screenQueuesLock = new Lock("screenQueuesLock");
	securityQueuesLock = new Lock("securityQueuesLock");
	updateClearCount = new Lock("updateClearCount");
	screenQueues = new List*[security];
	securityQueues = new List*[security];
	returnQueues = new List*[security];
	screenLocks = new Lock*[security];
	passengerWaitOfficerCV = new Condition*[security];
	officerWaitPassengerCV = new Condition*[security];
	securityLocks = new Lock*[security];
	inspectorWaitRePassengerCV = new Condition*[security];
	rePassengerWaitInspectorCV = new Condition*[security];
	inspectorWaitPassengerCV = new Condition*[security];
	passengerWaitInspectorCV = new Condition*[security];
	for (i = 0; i < security; i++) {

		screenQueues[i] = new List;
		securityQueues[i] = new List;
		returnQueues[i] = new List;
		screenLocks[i] = new Lock("screenLocks");
		passengerWaitOfficerCV[i] = new Condition("passengerWaitOfficerCV");
		officerWaitPassengerCV[i] = new Condition("officerWaitPassengerCV");
		securityLocks[i] = new Lock("securityLocks");
		inspectorWaitRePassengerCV[i] = new Condition("inspectorWaitRePassengerCV");
		rePassengerWaitInspectorCV[i] = new Condition("rePassengerWaitInspectorCV");
		inspectorWaitPassengerCV[i] = new Condition("inspectorWaitPassengerCV");
		passengerWaitInspectorCV[i] = new Condition("passengerWaitInspectorCV");
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
