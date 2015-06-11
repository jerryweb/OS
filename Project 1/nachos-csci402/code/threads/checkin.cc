#include "checkin.h"

CheckIn::CheckIn(int airline_, int id_, Airport* airport_)
{
    airline = airline_;
	id = id_;
    airport = airport_;
    passengers = 0;
    luggage = 0;
    airport->RequestingCheckinData[id] = false;
}

CheckIn::~CheckIn()
{
    airport = NULL;
}

Passenger* CheckIn::FindPassenger(int execLine)
{
    Passenger* pass = NULL;
    if (! airport->checkinQueues[execLine]->IsEmpty())
    {   // Passenger in executive line.
        pass = (Passenger*)airport->checkinQueues[execLine]->Remove();
        printf("Airline check-in staff %d of airline %d serves an executive class passenger and economy class line length = %d\n",
                id, airline, airport->checkinQueues[id]->Size());
    }
    else if (! airport->checkinQueues[execLine + id]->IsEmpty())
    {   // Passenger in economy line.
        pass = (Passenger*)airport->checkinQueues[id]->Remove();
        printf("Airline check-in staff %d of airline %d serves an economy class passenger and executive class line length = %d\n",
                id, airline, airport->checkinQueues[execLine]->Size());
    }
    else
    {   // Both lines empty.
            airport->checkinLock[id]->Acquire();
            airport->checkinState[id] = CI_BREAK;
            airport->checkinLineLock[airline]->Release();
            //printf("CheckIn %d goes on break\n", id);

            airport->checkinBreakCV[id]->Wait(airport->checkinLock[id]);

            airport->checkinState[id] = CI_BUSY;
            airport->checkinLineLock[airline]->Acquire();
    }
    return pass;
}

void CheckIn::StartCheckInStaff()
{
    Passenger* pass = NULL;
    bool exec;
    int passAirline;
    int execLine = airline * 6;
    BoardingPass bp;
    while (true)
    {
        //printf("adfa: %d\n", airport->RequestingCheckinData[id]);
        airport->checkinLineLock[airline]->Acquire();
        // Check the lines, sleep if both empty.
        if (airport->checkinQueues[execLine]->IsEmpty() &&
            airport->checkinQueues[id]->IsEmpty())
        {   // Both lines empty.
            airport->checkinLock[id]->Acquire();
            airport->checkinState[id] = CI_BREAK;
            airport->checkinLineLock[airline]->Release();
            //printf("CheckIn %d goes on break\n", id);

            airport->checkinBreakCV[id]->Wait(airport->checkinLock[id]);

            airport->checkinState[id] = CI_BUSY;
            airport->checkinLineLock[airline]->Acquire();
        }

        pass = FindPassenger(execLine);
        airport->checkinLock[id]->Acquire();
        airport->checkinLineLock[airline]->Release();
        // Process ticket, create boarding pass.
        airport->airlineLock[airline]->Acquire();
        passAirline = pass->getTicket().airline;
        exec = pass->getTicket().executive;
        passengers++;
        bp.seatNum = airport->airlines[airline]->seatsAssigned;
        airport->airlines[airline]->seatsAssigned++;
        bp.gate = airline;
        pass->SetBoardingPass(bp);

        if (exec) printf("Airline check-in staff %d of airline %d informs executive class passenger %d to board at gate %d\n",
                          id, airline, pass->getID(), airline);
        else printf("Airline check-in staff %d of airline %d informs economy class passenger %d to board at gate %d\n",
                     id, airline, pass->getID(), airline);
        airport->checkinLineCV[id]->Signal(airport->checkinLock[id]);
        airport->airlineLock[airline]->Release();
        // Process luggage and add it to conveyor.
        airport->conveyorLock->Acquire();
        while (! pass->getLuggage()->IsEmpty())
        {
            Luggage* bag = (Luggage*)pass->getLuggage()->Remove();
            bag->airlineCode = passAirline;
            airport->conveyor->Append(bag);
            luggage++;
            weight += bag->weight;
        }
        printf("Airline check-in staff %d of airline %d dropped bags to the conveyor system\n",
                id, airline);
        airport->conveyorLock->Release();
        // Check if all passengers are processed, close if done.
        airport->airlineLock[airline]->Acquire();
        if (airport->airlines[airline]->seatsAssigned >= airport->airlines[airline]->ticketsIssued)
        {
            airport->checkinLock[id]->Acquire();
            printf("Airline check-in staff %d is closing the counter\n", id);
            airport->checkinState[id] = CI_CLOSED;
            airport->airlineLock[airline]->Release();
            airport->checkinBreakCV[id]->Wait(airport->checkinLock[id]);
        }
        airport->airlineLock[airline]->Release();

        
        if(airport->RequestingCheckinData[id]){
            printf("check-in staff %d says hey.\n", id);
            airport->checkinManagerLock->Acquire();
            printf("check-in staff %d is sending data.\n", id);

            // Give manager data
            airport->checkinManagerCV->Signal(airport->checkinManagerLock);
            airport->checkinLock[id]->Acquire();
            airport->checkinManagerLock->Release();
            airport->checkinCV[id]->Wait(airport->checkinLock[id]);

            //Wait for manager to signal that all the data has been collected
            airport->checkinLock[id]->Acquire();
            printf("check-in staff %d has finished reporting data to manager.\n", id);
            airport->checkinLock[id]->Release();
            airport->RequestingCheckinData[id] = false;          
        }

    }
}
