// Defines structs and global data classes used in the airport sim.

#ifndef AIRPORTOBJECTS_H
#define AIRPORTOBJECTS_H

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

//	These are the states used by the various classes 
enum LiaisonState {L_FREE, L_BUSY, L_NONE};
enum CheckinState {CI_FREE, CI_BUSY, CI_BREAK, CI_CLOSED, CI_NONE};
enum CargoState   {C_BUSY, C_BREAK, C_NONE};
enum AirlineState {BOARDING, BOARDED};
enum ScreenState  {SO_BUSY, SO_FREE};

#endif
