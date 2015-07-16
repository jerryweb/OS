#include "serversynch.h"
#include "interrupt.h"
#include <sstream>
#include <string>

using namespace std;

serverLock::serverLock(char* dName, int owner, int mailbox) {
	name = dName;
	ownerID = owner;
	mailboxID = mailbox;
	waitQue = new List();
	state = FREE;
}

serverLock::~serverLock() {
	delete waitQue;
}

void serverLock::Acquire(int outAddr, int outBox) {
	char* msg = new char[MaxMailSize];

	if (state == FREE) {
		//proceed
		state = BUSY;
		ownerID = outAddr;
		mailboxID = outBox;

		msg = "0";
		ServerReply(msg, outAddr, mailbox, 0);
	} else {
		//append msg to wait queue
		string toAppend;
		stringstream ss;
		ss << outAddr << " " << outBox;
		toAppend = ss.str();
		msg = (char*) toAppend->c_str();
		waitQue->Append((void*) toAppend);
	}
}

void serverLock::Release(int outAddr, int outBox) {

	char* msg = new char[MaxMailSize];

	if (!waitQue->IsEmpty()) {
		//send reply to waiting userprog
		char* waitMsg = new char[MaxMailSize];
		waitMsg = (char*) waitQue->First();

		int waitAddr, waitBox;
		stringstream ss;
		ss << waitMsg;
		ss >> waitAddr >> waitBox;

		waitMsg = "0";

		ServerReplay(waitMsg,waitAddr,waitBox);
	}

	else {
		state = FREE;
		ownerID = -1;
	}

	//
}

serverCV::serverCV(char* dName, Table* lTable) {
	name = dName;
	lock = NULL;
	waitQue = new List();
	lockTable = lTable;
}

serverCV::~serverCV() {
	delete waitQue;
}

void serverCV::Signal(serverLock *sLock, int out) {
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *msg;
	char buffer[MaxMailSize];
	stringstream ss;
	ss.str("");
	ss.clear;

	outPktHdr.to = out;
	outMailHdr.to = 0;
	outMailHdr.from = out;

	//check if lock exist
	if (!tableItemExist(sLock->name, lockTable, 1)) {
		msg = "1";
	} else {

	}
}

void serverCV::Wait(serverLock *sLock, int out) {
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *msg;
	char buffer[MaxMailSize];
	stringstream ss;
	ss.str("");
	ss.clear;

	outPktHdr.to = out;
	outMailHdr.to = 0;
	outMailHdr.from = out;

}
