// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "../threads/system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
#include "../threads/synch.h"

extern "C" { int bzero(char *, int); };
TranslationEntryIPT* ipt;
int currentTLB;


Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
    count = 0;
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given i, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1){
    	table[i] = f;
        count++;
        maxCount++;
    }
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
        count--;
	}
	lock->Release();
    }
    return f;
}

void Table::lockAcquire() {
    // Acquire table's lock
	lock->Acquire();
}
void Table::lockRelease() {
    // Acquire table's lock
	lock->Release();
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consrtucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable_) : fileTable(MaxOpenFiles) {
    executable = executable_;
    NoffHeader noffH;
    unsigned int i, size;
    int maxNumThreads = 128;
    id = processTable->Put(this);           //adds a process to the process table

    //Keep track of all of the threads that belong to the process
    threadTable = new Table(maxNumThreads);

    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
    DEBUG('a', "Initializing address space... size %d\n", size);

    numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize,PageSize);
                        // we need to increase the size
						// to leave room for the stack

    size = numPages * PageSize; //page size is 128 bytes

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    int ppn = 0;
    

    
    pageTable = new TranslationEntry[numPages];

    for (i = 0; i < numPages; i++) {
        ppn = getFreePage();
        
        printf("AddrSpace: setting physical page %d valid\n", ppn);
        
        pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
    	pageTable[i].physicalPage = ppn; //ppn not i
    	pageTable[i].valid = FALSE;
    	pageTable[i].use = FALSE;
    	pageTable[i].dirty = FALSE;
    	pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
    					// a separate page, we could set its 
    					// pages to be read-only
        ipt[ppn].virtualPage = i;
        ipt[ppn].space = this;
        ipt[ppn].processID = id;
        ipt[ppn].valid = TRUE;
    	ipt[ppn].use = FALSE;
    	ipt[ppn].dirty = FALSE;
    	ipt[ppn].readOnly = FALSE;
                        
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
            noffH.code.virtualAddr, noffH.code.size);

        executable->ReadAt(&(machine->mainMemory[ppn * PageSize]),
            PageSize, noffH.code.inFileAddr + i*PageSize);
    }
    

    currentThread->setThreadTableLocation(threadTable->Put(currentThread));        //adds the thread to the thread table

    

    //machine->mainMemory[ppn * PageSize];
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    //need to delete this once we start using exec and the constructor gets called
    //more than once
    //bzero(machine->mainMemory, size);

}   

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::setNewPageTable(){
    //unsigned int tempNumPages = numPages;
    //unsigned int size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
    unsigned int previousNumPages = numPages;
    numPages += divRoundUp(UserStackSize,PageSize); //decrement on exit
    TranslationEntry *tempTable = new TranslationEntry[numPages];

    int ppn = 0;
    //copy over old Page Table data
    for (unsigned int i = 0; i < previousNumPages; i++) {
        
        DEBUG('z', "setNewPageTable: copying page %i\n", i);
        
        tempTable[i].virtualPage = pageTable[i].virtualPage;   
        tempTable[i].physicalPage = pageTable[i].physicalPage;
        tempTable[i].valid = pageTable[i].valid;
        tempTable[i].use = pageTable[i].use;
        tempTable[i].dirty = pageTable[i].dirty;
        tempTable[i].readOnly = pageTable[i].readOnly;  
    }

    for (unsigned int i = previousNumPages; i < numPages; ++i)
    {
        ppn = getFreePage();
        
        DEBUG('z', "setNewPageTable: setting physical page %d valid\n", ppn);
        
        tempTable[i].virtualPage = i;   
        tempTable[i].physicalPage = ppn;
        tempTable[i].valid = FALSE;
        tempTable[i].use = FALSE;
        tempTable[i].dirty = FALSE;
        tempTable[i].readOnly = FALSE;
        
        ipt[ppn].virtualPage = i;
        ipt[ppn].space = this;
        ipt[ppn].processID = id;
        ipt[ppn].valid = TRUE;
    	ipt[ppn].use = FALSE;
    	ipt[ppn].dirty = FALSE;
    	ipt[ppn].readOnly = FALSE;
    }

    delete pageTable; 
    pageTable = tempTable;

    RestoreState();
}

void HandleIPTMiss(int vpn)
{
    // allocate one memory page table
    // copy from executable if needed
    // update page table
    //  set valid bit to true
    //  update physicalPage
    // populate IPT from page table
}
//Copy page table info to the tlb
void AddrSpace::PageFault(){
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    //page table index
    int PTIndex = getPPN((int)machine->ReadRegister(39)/PageSize); // will return -1 if not found
    //PTIndex = machine->ReadRegister(39)/PageSize;
    DEBUG('z', "PageFault: reg = %d, vpn = %d, ppn = %d\n", (int)machine->ReadRegister(39), (int)machine->ReadRegister(39)/PageSize, PTIndex);

    //Changed pageTable to ipt, not sure if this is accurate 
    if (PTIndex != -1)
    {
        DEBUG('z', "PageFault: copying ppn %d to tlb %d\n", PTIndex, currentTLB);
        
        machine->tlb[currentTLB].virtualPage = ipt[PTIndex].virtualPage;
        machine->tlb[currentTLB].physicalPage = ipt[PTIndex].physicalPage;
        machine->tlb[currentTLB].valid = ipt[PTIndex].valid;
        machine->tlb[currentTLB].use = ipt[PTIndex].use;
        machine->tlb[currentTLB].dirty = ipt[PTIndex].dirty; 
        machine->tlb[currentTLB].readOnly = ipt[PTIndex].readOnly;
    }
    else
    {
        HandleIPTMiss((int)machine->ReadRegister(39)/PageSize);
    }

    //works like a circular queue
    //currentTLB = (currentTLB++) % TLBSize;            //doesn't work :(
    if(currentTLB >= TLBSize - 1)
        currentTLB = 0;
    else
        currentTLB++;

    (void) interrupt->SetLevel(oldLevel);  //reenable interrupts     
}

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{

    
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//  This will also invalitdate the TLB
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    //DEBUG('z', "RestoreState has been called\n");
    // machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
    //invalidate the TLB
    currentTLB = 0;
    for(int i = 0; i <TLBSize; i++)
    {
        DEBUG('z', "RestoreState: setting tlb page %d invalid\n", i);
        machine->tlb[i].valid = false;
    }
}

int AddrSpace::getPPN(int vpn)
{
    for (int i = 0; i < NumPhysPages; i++)
    {
        TranslationEntryIPT t = ipt[i];
        //printf("i = %d, valid = %d, processID = %d, id = %d, virtualPage = %d, vpn = %d, physicalPage = %d\n", i, (int)t.valid, t.processID, id, t.virtualPage, vpn, t.physicalPage);
        if (t.valid && t.processID == id && t.virtualPage == vpn)
        {
            return t.physicalPage;
        }
    }
    return -1;
}

int AddrSpace::getFreePage()
{
    for (int i = 0; i < NumPhysPages; i++)
    {
        TranslationEntryIPT t = ipt[i];
        if (! t.valid)
        {
            return t.physicalPage;
        }
    }
    return -1;
}
