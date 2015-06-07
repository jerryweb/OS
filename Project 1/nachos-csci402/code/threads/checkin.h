//This is the check-in staff class 

#ifndef CHECKIN_H
#define CHECKIN_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"

class CheckIn : public Thread {
	public:
		CheckIn(int airline, int id, Airport* airport);
		~CheckIn();
		void doStuff();

	private:
        int airline;
		int id;
        Airport* airport;
        int passengers;
        int luggage;
}