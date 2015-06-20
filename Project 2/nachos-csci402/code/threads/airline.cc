#include "airline.h"

Airline::Airline(int id_, int ticketsIssued_, int totalBagCount_, int totalBagWeight_)
{
    id = id_;
    ticketsIssued  = ticketsIssued_;
    totalBagCount  = totalBagCount_;
    totalBagWeight = totalBagWeight_;
}

Airline::~Airline()
{
    
}
