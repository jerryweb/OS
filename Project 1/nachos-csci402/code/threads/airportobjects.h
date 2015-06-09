// Defines structs and global data classes used in the airport sim.

#ifndef AIRPORTOBJECTS_H
#define AIRPORTOBJECTS_H
#endif
#include "copyright.h"
#include "thread.h"
#include "list.h"


struct Luggage          // A single bag held by a passenger. airlineCode
                        //  assigned by check-in staff.
{
	int airlineCode;    // The ID of the airline the ticket is for.
	int weight;         // The weight of the bag, in lbs.
};

struct Ticket           // A ticket held by a passenger.
{
	int airline;        // The ID of the airline the ticket is for.
	bool executive;     // The type of ticket held: executive is T while
                        //  economy is F.
};

struct BoardingPass     // A boarding pass created by check-in staff and
                        //  given to passenger.
{
	int gate;           // The ID of the gate the passenger will wait at
                        //  pre-boarding. Identical to the airline ID.
	int seatNum;        // The seat number assigned to the passenger by
                        //  the check-in staff.
};

enum LiaisonState {FREE, BUSY};
enum CheckinState {FREE, BUSY, BREAK};

class Airport {
        // TODO: add constructor and destructor.
    public:
        // TODO: change to condition variables
        // TODO: change from hard-coded numbers where inappropriate
        //  (e.g. # airlines, aircraft).
        Airline* airlines[3];           // Array of airlines. Index is
                                        // airline ID.
        List* aircraft[3];              // Array of aircraft (list of
                                        //  Luggage). Index corresponds
                                        //  to airline ID.
        List* liaisonQueues[7];         // Array of liaison queues (list
                                        //  of Passengers). Index
                                        //  corresponds to liaison ID.
        Lock* liaisonLineLock;          // Lock for the liaison lines.
        Lock* liaisonLock[7];           // Array of locks for each
                                        //  liaison. Index corresponds to
                                        //  liaison ID.
        Condition* liaisonCV[7];        // Array of CVs for each liaison.
                                        //  Index corresponds to liaison ID.
        LiaisonState liaisonState[7];   // Array of states for each liaison.
                                        //  Index corresponds to liaison ID.
        List* checkinQueues[3][5];      // 2D array of check-in queues (list
                                        //  of Passengers): [airline][id].
        CheckinState checkinState[3][5];// 
        Condition* checkinCV[3][5];     // 
        List* conveyor;                 // Conveyor belt for bag transfer
                                        //  from check-in to cargo (list of
                                        //  Luggage).
        Lock* conveyorLock;             // Lock for the conveyor belt.
        Condition* cargoCV;             // CV for cargo worker break.
        List* screenQueues[3];          // Array of screening queues (list
                                        //  of Passengers). Index corresponds
                                        //  to screener ID.
        List* securityQueues[3];        // Array of security queues (list
                                        //  of Passengers). Index corresponds
                                        //  to security ID.
        // TODO: add other lists/variables here.
}

class Airline {
        // TODO: add constructor (and destructor maybe).
    public:
        // No need for an ID because it corresponds with the index of the
        //  airlines array in Airport.
        int seatsAssigned;  // Number of seats currently assigned. Used to
                            //  assign seat numbers to passengers at check-in
                            //  as well as keeping track of check-in progress.
        int ticketsIssued;  // Total number of passengers assigned to this
                            //  flight. Does not change.
}
