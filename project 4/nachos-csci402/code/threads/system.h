// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "../userprog/translateipt.h"
#include "../userprog/table.h"
#include "../userprog/addrspace.h"
#include "../userprog/translateipt.h"
#include "../userprog/translateexec.h"
#include <stdio.h>

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.
extern void SetEvictionPolicy(bool fifo);

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;				// performance metrics
extern Timer *timer;					// the hardware alarm clock
extern Table* lockTable;				// Tables for the CVs and Locks
extern Table* CVTable;	
extern Table* processTable;				// Table for keeping tracking of all of the address spaces 
										// for the processes
extern Table* LastTimeRecievedTable;
extern BitMap* memMap;
extern BitMap* swapFileMap;
extern OpenFile* swapFile;
extern Lock* swapLock;

extern TranslationEntryIPT* ipt;
extern Lock* iptLock;

extern Lock* forkLock;
extern Lock* execLock;

extern int currentTLB;
extern int evictionPolicy;				// Used to determine which memory eviction policy to use 
										// 0 will use the FIFO policy, and 1 will use the Random policy
extern int currentBoxCountPointer;
extern Lock* boxCountIndexLock;

//#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers

//#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "../filesys/filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern int netname;
extern PostOffice* postOffice;
extern int createLockRequests;
extern int createCVRequests;
extern int createMVRequests;
extern Table* serverLockTable;
extern Table* serverCVTable;
extern Table* MVTable;
extern unsigned int* LTRArray;
extern List* pendingMsg;
#endif

#endif // SYSTEM_H
