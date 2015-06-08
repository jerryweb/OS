//This is the passenger class. It holds all of the information and functions for the passenger class


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
		int getID(){ return id;}
		/*this function iterates through a given array and returns the location
		of the smallest value and the value itself*/
		void findShortestLine(int* list, int *location, int *minValue);
		/*This is the first version of the function for the passenger to find 
		the shortest liaison line in the Airport.*/
		void findShortestLiaisonLine(int* liaisonList);
		/*This finds the shortest line at the check in counter*/
		void findShortestCheckinLine(int* CheckinList[);

		//newly added Kevin
		void SetSecurityPass(bool pnp);
		bool GetSecurityPass();
		void Questioning();    //yield random cycles
		int GetID();


	private:

		int id;					//This is the ID of the passenger
		Luggage luggage[3];		//This is the array of bags the passenger has
		Ticket ticket;  		//This is the ticket of the passenger

		//newly added
			bool securityPass;

}
