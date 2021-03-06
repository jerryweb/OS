// Defines structs and global data classes used in the airport sim.

#ifndef AIRLINE_H
#define AIRLINE_H

#include "copyright.h"
#include "thread.h"
#include "list.h"

class Airline {
    public:
        Airline(int id, int ticketsIssued, int totalBagCount);
        ~Airline();
        
        int id;
        int seatsAssigned;  // Number of seats currently assigned. Used to
                            //  assign seat numbers to passengers at check-in
                            //  as well as keeping track of check-in progress.
        int ticketsIssued;  // Total number of passengers assigned to this
                            //  flight. Does not change.
        int totalBagCount;  // Total number of luggage assigned to this
                            //  flight. Does not change.
};

#endif
