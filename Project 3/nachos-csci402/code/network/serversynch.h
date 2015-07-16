// synchorinzation objects used for server

#ifndef SERVERSYNCH_H
#define SERVERSYNCH_H

#include "list.h"
#include "network.h"
#include "post.h"
//TODO:need to include Table here

enum lockState {
	FREE, BUSY
};

//tableType: 1 for lockTable, 2 for CVTable
bool tableItemExist(char* tName, Table* table, int tableType) {
	bool toReturn = false;
1
	for (int i = 0; i < table->Size(); i++) {
		if (tableType == 1)
			serverLock* tableItem = (serverLock*) sTable->Get(i);
		else
			serverCV* tableItem = (serverCV*) sTable->Get(i);
		if (tableItem->name == lName) {
			toReturn = true;
			break;
		}
	}

	return toReturn;
}

bool ServerReply(char* sMsg,int outMachine,int outMailbox,int fromMailbox) {
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char buffer[MaxMailSize];
	stringstream ss;
	ss.str("");
	ss.clear;

	outPktHdr.to = outMachine;
	outPktHdr.from = 0;
	outMailHdr.to = outMailbox;
	outMailHdr.from = fromMailbox;
	outMailHdr.length = strlen(sMsg) + 1;

	postOffice->Send(outPktHdr, outMailHdr, sMsg);

	delete [] sMsg;
}

class serverLock {
public:
	serverLock(char* dName, int owner, int mailbox);
	~serverLock();

	void Acquire(int outAddr,int outBox);
	void Release(int outAddr,int outBox);

	char* name;
	lockState state;
	int owenerID;       //owner machine id
	int mailboxID;      //owner mailbox number
	List* waitQue;

};

class serverCV {
public:
	serverCV(char* dName);
	~serverCV();

	void Signal(serverLock *sLock,int outAddr,int outBox);
	void Wait(serverLock *sLock,int outAddr,int outBox);
	void Boardcast(serverLock *sLock,int outAddr,int outBox);

	char* name;
	serverLock* waitLock;
	List* waitQue;
};

#endif
