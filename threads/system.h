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
#include "synch.h"

#define MAX_PROCESSES 50
#define MAX_THREADS 100

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock


#ifdef USER_PROGRAM
#include "machine.h"
#include "bitmap.h"
extern Machine* machine;	// user program memory and registers
extern BitMap *bitMap_obj;			// one bit map object for the entire OS

extern Lock *pro_tablelock;
extern Lock *mainmemorylock;
extern Lock *pro_tablelock;
extern Lock *printf_lock;



struct userlocks
{
	int flag;
	char* name;//name of lock
	Lock* lock;//to create lock
	AddrSpace *add_space;//address space pointer

	int lock_usagecounter;//counter to check the usage of lock
};
extern userlocks userlock[100];

struct usercvs
{
	int flag;
	char* name; //name of condition
	Condition* condition; //to create condition
	AddrSpace *add_space; //address space pointer

	int cv_usagecounter;//counter to check the usage of condition
};
extern usercvs usercv[100];

extern Lock *locksmanagerLock;
extern Lock *cvsmanagerLock;
extern Lock *addrupdatelock;

//total locks  and cvs
extern int totallocks;
extern int totalcvs;



struct Thread_table									// struct to store info related to a specific thread
    	{
    		//addspace
    		//indication var for exit
    		int thread_flag;

    	};


    	struct Personal_pro_table								//  struct to store info related to a specific Process
    	{
    		int process_flag;
    		int total_threads;
    		AddrSpace *personal_addrspace;
    		unsigned int endof_addrspace;
    		Thread_table thread_table_array[MAX_THREADS];

    	};


    	struct Pro_table										// global struct to store process table
    	{
    		int total_pro;
    		Personal_pro_table pro_table_array [MAX_PROCESSES] ;

    	};



    	    extern Thread_table thread_table; 	//// struct to store info related to a specific thread
    	 extern Personal_pro_table personal_pro_table;
    	extern Pro_table pro_table;		// structure obj for the process table




#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
