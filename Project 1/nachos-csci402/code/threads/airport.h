// Defines global data classes used in the airport sim.

#ifndef AIRPORT_H
#define AIRPORT_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"
#include "airline.h"
#include "synch.h"

class Airport {

public:
	// TODO: add constructor and destructor.
	Airport();
	Airport(int airlines, int passengers, int liaisons, int checkins, int security, int cargo);
	~Airport();
	// TODO: change to condition variables
	int numAirlines;                // Number of airlines.
	// TODO: change from hard-coded numbers where inappropriate
	//  (e.g. # airlines, aircraft).
	// General variables
	bool* RequestingLiaisonData;    // Array of booleans that inform the
									// liaison if the manager wants data
	bool* RequestingCargoData;      // Array of booleans that inform the
									// Cargo Handlers if the manager wants data
	bool* RequestingCheckinData;
	Airline** airlines;             // Array of airlines. Index is
									//  airline ID.
	List* airlineState[3];          // This is the list of airline states
									// which can either be boared or boarding
	Lock** airlineLock;             // Array of locks to handle airline
									//  seating and passenger checking.
									//  Index corresponds to airline ID.
    List** boardingQueue;           // Array of lists of passengers who
                                    //  have reached each boarding area. 
                                    //  ID corresponds to airline.
    Condition** boardingCV;         // Array of CVs for passengers who have
                                    //  reached boarding area. ID
                                    //  corresponds to airline.
    Lock** boardingLock;            // Array of locks for boardingCVs. ID
                                    //  corresponds to airline.
    
	// Passenger variables
	List* passengerList;            // List of passengers in the sim.
	// Liaison variables
	List* liaisonQueues[7];         // Array of liaison queues (list
									//  of Passengers). Index
									//  corresponds to liaison ID.
	List* liaisonList;              // List of Liaisons
	Lock* liaisonManagerLock;       // for interaction with manager
	Lock* liaisonLineLock;          // Lock for the liaison lines.
	Lock* liaisonLock[7];           // Array of locks for each
									//  liaison. Index corresponds to
									//  liaison ID.
	Condition* liaisonLineCV[7];    // Array of CVs for each liaison's
									//  line. Index corresponds to ID.
	Condition* liaisonManagerCV;    //  CV for each liaison interaction
	Condition* liaisonCV[7];        // Array of CVs for each liaison.
									//  Index corresponds to ID.
	LiaisonState liaisonState[7];   // Array of states for each liaison.
									//  Index corresponds to ID.
	// Check-in variables
	bool* finalCheckin;
	List** checkinQueues;           // Array of check-in queues (list of
									//  Passengers). Index corresponds
									//  to check-in staff ID. The executive
									//  lines are (airlineID * 5).
	List* checkInStaffList;         // List of total number of checkin staff
	CheckinState* checkinState;     // Array of states for each check-in
									//  staff. Index corresponds to ID.
	Lock** checkinLineLock;         // Array of locks for each airline's
									//  check-in staff. Index corresponds
									//  to airline.
	Condition** checkinLineCV;      // Array of CVs for each check-in
									//  staff's line. Index corresponds
									//  to ID.
	Condition** checkinCV;
	Lock** checkinLock;             // Array of locks for each check-in
									//  staff. index corresponds to ID.
	Condition** checkinBreakCV;     // Array of CVs for each check-in
									//  staff to go on break. Index
									//  corresponds to ID.
	Condition* checkinManagerCV;
	Lock* checkinManagerLock;       // for interaction with manager

	// Cargo variables
	List* conveyor;                 // Conveyor belt for bag transfer
									//  from check-in to cargo (list of
									//  Luggage).
	List* cargoHandlerList;         // List of all of the cargo handlers in
									//  the airport.
	Lock* conveyorLock;             // Lock for the conveyor belt.
	Lock* CargoHandlerManagerLock;  // for interaction with manager

	Condition* cargoCV;             // CV for cargo handler break.
	Condition* cargoDataCV[10];        // Array of CVs for each liaison.
									   //  Index corresponds to ID.

	Condition* cargoManagerCV[10]; // Array of CVs for each cargo handler interaction
	Lock* cargoLock[10];                // Lock for cargo handler break.
	Lock* cargoDataLock[10];
	CargoState cargoState[10];      // Array of states for each cargo
									//  handler. Index corresponds to ID.
	List** aircraft;                // Array of aircraft (list of
									//  Luggage). Index corresponds
									//  to airline ID.

									// Screening and security variables

	int* clearPassengerCount;         //array to store cleared passenger cout for each airline
									  //used by airport manager

	Lock* updateClearCount;           //Lock to update security inspector's clear passenger count


	List* securityInspectorList;      //used by airport manager

	List** screenQueues;                    // Array of screening queues (list
											//  of Passengers). Index corresponds
											//  to screener ID.

	List** securityQueues;                  // Array of security queues (list
											//  of Passengers). Index corresponds
											//  to security ID.
    ScreenState* screenState;   // Array of states for each liaison.
	
    List** returnQueues;               //Array of returning security queues(list
									   // of Passenger). Index corresponds
									   //  to security ID.
	List* screeningOfficerList;
    Lock* screenLineLock;

	Lock** screenLocks; //Array of locks for screening lines (each officer has a lock)
						//index corresponds to officer id

	Lock* screenQueuesLock;        //Lock for all the screening lines as a whole
								   //should be used when determining which line is the shortest

	Lock** securityLocks; //Array of locks for security lines (each inspector has a lock)
						  //index corresponds to inspector id

	Lock* securityQueuesLock;       //Lock for all the security lines as a whole
									//should be used when determining which line is the shortest

    Condition** screenlineCV;
    Condition** securitylineCV;
    Condition** screenCV;

	Condition** inspectorWaitPassengerCV; //Array of C.V. used in inspector waiting passenger
										  //index corresponds to inspector id

	Condition** passengerWaitInspectorCV; //Array of C.V. used in passenger waiting inspector
										  //index corresponds to inspector id

	Condition** passengerWaitOfficerCV;    //Array of C.V. for each screen locks
										   //index corresponds to officer id

	Condition** officerWaitPassengerCV; //Array of C.V. used in officer waiting Passenger
										//index corresponds to officer id

	Condition** inspectorWaitRePassengerCV; //Array of C.V. used in inspector waiting returned passenger
											//index corresponds to inspector id

	Condition** rePassengerWaitInspectorCV; //Array of C.V. used in returned passenger waiting inspector
											//index corresponds to officer id

	Condition** inspectorWaitQuestioningCV;  //Array of C.V. used in inspector
											 //waiting questiong passenger
											 //inspector called this lock when both securityQueue
											 //and returnQueue is empty but still passenger being questioned
											 //index corresponds to inspector id

	Condition** liaWaitPassengerCV;
	Condition** passengerWaitLiaCV;
	Lock* checkinQueuesLock;

	//  Manager variables
    

};

#endif
