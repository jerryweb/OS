//This is the passenger class 

#ifndef PASSENGER_H
#define PASSENGER_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"

class Passenger:public Thread {
	public:
		Passenger(int id, Luggage* luggage, Ticket ticket);
		~Passenger();
		void findShortestLine();
		

	private:
		int id;
		Luggage luggage[3];
		Ticket ticket;  

}