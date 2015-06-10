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
        ~Airport();
        // TODO: change to condition variables
        int numAirlines;                // Number of airlines.
        // TODO: change from hard-coded numbers where inappropriate
        //  (e.g. # airlines, aircraft).
        // General variables
        Airline** airlines;             // Array of airlines. Index is
                                        //  airline ID.
        List* airlineState[3];          // This is the list of airline states 
                                        // which can either be boared or boarding
        Lock** airlineLock;             // Array of locks to handle airline
                                        //  seating and passenger checking.
                                        //  Index corresponds to airline ID.
        // Passenger variables
        List* passengerList;            // List of passengers in the sim.
        // Liaison variables
        List* liaisonQueues[7];         // Array of liaison queues (list
                                        //  of Passengers). Index
                                        //  corresponds to liaison ID.
        List* liaisonList;              // List of Liaisons
        
        //Lock* LineLock;                 // Lock for the liaison lines.
        Lock* liaisonLock[7];           // Array of locks for each
                                        //  liaison. Index corresponds to
                                        //  liaison ID.
        
        //Condition* lineCV[7];           // 

        Condition* liaisonLineCV[7];    // Array of CVs for each liaison's
                                        //  line. Index corresponds to ID.
        Condition* liaisonCV[7];        // Array of CVs for each liaison.
                                        //  Index corresponds to ID.
        LiaisonState liaisonState[7];   // Array of states for each liaison.
                                        //  Index corresponds to ID.
        // Check-in variables
        List** checkinQueues;           // Array of check-in queues (list of
                                        //  Passengers). Index corresponds
                                        //  to check-in staff ID. The executive
                                        //  lines are (airlineID * 5).
        CheckinState* checkinState;     // Array of states for each check-in
                                        //  staff. Index corresponds to ID.
        Lock** checkinLineLock;         // Array of locks for each airline's
                                        //  check-in staff. Index corresponds
                                        //  to airline.
        Condition** checkinLineCV;      // Array of CVs for each check-in
                                        //  staff's line. Index corresponds
                                        //  to ID.
        Lock** checkinLock;             // Array of locks for each check-in
                                        //  staff. index corresponds to ID.
        Condition** checkinCV;          // Array of CVs for each check-in
                                        //  staff. Index corresponds to ID.
        Condition** checkinBreakCV;     // Array of CVs for each check-in
                                        //  staff to go on break. Index 
                                        //  corresponds to ID.
        // Cargo variables
        List* conveyor;                 // Conveyor belt for bag transfer
                                        //  from check-in to cargo (list of
                                        //  Luggage).
        List* cargoHandlerList;         // List of all of the cargo handlers in 
                                        //  the airport.
        Lock* conveyorLock;             // Lock for the conveyor belt.
        Condition* cargoCV;             // CV for cargo handler break.
        Lock* cargoLock;                // Lock for cargo handler break.
        CargoState cargoState[10];      // Array of states for each cargo
                                        //  handler. Index corresponds to ID.
        List** aircraft;                // Array of aircraft (list of
                                        //  Luggage). Index corresponds
                                        //  to airline ID.
        // Screening and security variables
        List* screenQueues[3];          // Array of screening queues (list
                                        //  of Passengers). Index corresponds
                                        //  to screener ID.
        List* securityQueues[3];        // Array of security queues (list
                                        //  of Passengers). Index corresponds
                                        //  to security ID.
        // TODO: add other lists/variables here.
//         Lock* screenLocks[3];       //Array of locks for screening lines (each officer has a lock)
//         Lock screenQueuesLock;       //Lock for all the screening lines as a whole, should be used when determining which line is the shortest
//         Lock* securityLocks[3];     //Array of locks for security lines (each inspector has a lock)
//         Lock securityQueuesLock;     //Lock for all the security lines as a whole, should be used when determining which line is the shortest
//         Condition* screenCV[3];      //Array of C.V. for each screen locks

        //  Manager variables



};

#endif
