// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "../filesys/filesys.h"
#include "table.h"

#define UserStackSize		1024 	// increase this as necessary!

#define MaxOpenFiles 256
#define MaxChildSpaces 256

class TranslationEntry;

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable_);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code
    unsigned int getNumPages() { return numPages; }
    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    int getID() { return id; }
    Table fileTable;			// Table of openfiles
    Table* threadTable;
    TranslationEntry* getPageTable() { return pageTable; }
    void PageFault();           //handles page fault exceptions
    void setNewPageTable();
    int getPPN(int vpn);
    int getFreePage();
    int HandleMemoryFull();     // Handles eviction of pages in the IPT


 private:
    int id;                         //ID of the address space (used for processes)
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    // int codeSize;
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
    // int stackPage;  //stack page location of the threads
    OpenFile* executable;
    int execSize;

    List* FIFOEvictionQueue;
    
};

#endif // ADDRSPACE_H
