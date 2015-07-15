#include "serversynch.h"
#include "interrupt.h"
#include <sstream>

serverLock::serverLock(char* dName, int owner, int mailbox) {
	name = dName;
	ownerID = owner;
	mailboxID = mailbox;
	waitQue = new List();
}

serverLock::~serverLock() {
	delete waitQue;
}

void serverLock::Acquire(int out) {
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

	if (state == FREE) {
		msg = "0";
		outMailHdr.length = strlen(msg) +1;

		bool success = postOffice->Send(outPktHdr,outMailhdr,msg);

		if (!success) {
			printf("severlock %d sending faliure in acquire\n",name);
			interrupt->Halt();
		}

		//wait for acknowledgement
		postOffice->Receive(out, &inPktHdr, &inMailHdr, buffer);
		int ack = -1;
		ss << buffer;
		ss >> ack;

		if (ack != 0) {
			printf("acknowledgement lost from machine %d in acquire\n",out);
			interrupt->Halt();
		}

		//proceed after acknowledgement
		state = BUSY;
		ownerID = out;

		fflush (stdout);   //TODO: where else to flush?
	}

	else {
		//append the machine index, since the message is always the same
		int* toAppend = new int[1];
		toAppend[0] = out;
		waitQue->Append(toAppend);
	}
}

void serverLock::Release() {

	if (!waitQue->IsEmpty()) {
		int* outPtr = (int*)waitQue->First();
		int out = outPtr[0];

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

		msg = "0";
		outMailHdr.length = strlen(msg) +1;

		bool success = postOffice->Send(outPktHdr,outMailhdr,msg);

		if (!success) {
			printf("severlock %d sending faliure in release\n",name);
			interrupt->Halt();
		}

		//wait for acknowledgement
		postOffice->Receive(out, &inPktHdr, &inMailHdr, buffer);
		int ack = -1;
		ss << buffer;
		ss >> ack;

		if (ack != 0) {
			printf("acknowledgement lost from machine %d in release\n",out);
			interrupt->Halt();
		}

		waitQue->Remove();
		delete [] outPtr;
	}

	else {
		state = FREE;
		ownerID = -1;
	}

}

serverCV::serverCV(char* dName) {
	name = dName;
}

serverCV::~serverCV() {
	delete waitQue;
}

void serverCV::Signal(serverLock *sLock) {

}

void serverCV::Wait(serverLock *sLock) {

}
