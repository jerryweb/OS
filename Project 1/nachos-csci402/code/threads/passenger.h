//This is the passenger class. It holds all of the information and functions for the passenger class


#ifndef PASSENGER_H
#define PASSENGER_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airport.h"

class Passenger {
	public:
		Passenger(int ID, List* bags, Ticket T, Airport* A, List** newLiaisonList, int *checkInStaffArray);
		Passenger(int ID, Ticket t, int airlineCode);   // For PassengerFindsCorrectCISLine (test).
        Passenger(int ID);                              // For CheckInTest.
        Passenger();                                    // For PassengerFindsCorrectCISLine (test).
        ~Passenger();

		/*this function iterates through a given array and returns the location
		of the smallest value and the value itself*/
		int findShortestLine(List** list, bool CISline);
		/*This is the first version of the function for the passenger to find 
		the shortest liaison line in the Airport.*/
		void findShortestLiaisonLine();
		int FindShortestCheckinLine();  // Find the shortest line at the check in counter.
        void CheckIn();                 // Drop off luggage and get boarding pass.
		
		
		//these are the get fucntion for the passenger's ticket
		Ticket getTicket() { return ticket; }

		List* getLuggage() { return luggageList; }
		//returns the id of the passenger
		int getID(){ return id; }
		//newly added Kevin

		//Luggage getLuggage(){ return *luggageList; }
				//void SetScreenPass(bool pnp);
		/*
		void SetSecurityPass(bool pnp);
		bool GetSecurityPass();
		void Questioning();    //yield random cycles
		*/

		BoardingPass GetBoardingPass() { return boardingPass; }
		void SetBoardingPass(BoardingPass BP) { boardingPass = BP; }

        int GetAirline() { return airline; }
        void SetAirline(int airline_) { airline = airline_; }
        
	private:

		int id;							//This is the ID of the passenger
        int airline;                    // Code of the airline. Set by liaison.
		int liaisonList[7];				//Full list of liaisons
		List* liaisons[7];
		int checkInStaffList[5];		//Full list of Check-in Staff
		List* luggageList;				//This is the array of bags the passenger has
		Ticket ticket;  				//This is the ticket of the passenger
		Airport* airport;   // "Airport" construct, containing all
                            //  public data.
		BoardingPass boardingPass;
		//newly added
		bool securityPass;

};

#endif
