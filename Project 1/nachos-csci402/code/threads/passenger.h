//This is the passenger class. It holds all of the information and functions for the passenger class


#ifndef PASSENGER_H
#define PASSENGER_H
#endif

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"

class Passenger {
	public:
		Passenger(int ID, Luggage *bags, Ticket T, int *liaisonArray, int *checkInStaffArray);
		~Passenger();

		/*this function iterates through a given array and returns the location
		of the smallest value and the value itself*/
		int findShortestLine(int* list, bool CISline);
		/*This is the first version of the function for the passenger to find 
		the shortest liaison line in the Airport.*/
		void findShortestLiaisonLine();
		/*This finds the shortest line at the check in counter*/
		void findShortestCheckinLine();

		
		
		//these are the get fucntion for the passenger's ticket
		Ticket getTicket() { return ticket; }
		//returns the id of the passenger
		int getID(){ return id; }
		//newly added Kevin

		Luggage getLuggage(){ return *luggage; }
				//void SetScreenPass(bool pnp);
		/*
		void SetSecurityPass(bool pnp);
		bool GetSecurityPass();
		void Questioning();    //yield random cycles
		*/

		BoardingPass GetBoardingPass() { return boardingPass; }
		void SetBoardingPass(BoardingPass BP) { boardingPass = BP; }

	private:

		int id;							//This is the ID of the passenger
		int liaisonList[7];				//Full list of liaisons
		int checkInStaffList[5];		//Full list of Check-in Staff
		Luggage luggage[3];				//This is the array of bags the passenger has
		Ticket ticket;  				//This is the ticket of the passenger
		BoardingPass boardingPass;
		//newly added
		bool securityPass;

};
