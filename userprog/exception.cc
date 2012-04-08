// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>


using namespace std;

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	DEBUG('s',"%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	DEBUG('s',"%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	DEBUG('s',"%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	DEBUG('s',"%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    DEBUG('s',"%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	DEBUG('s',"%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    DEBUG('s',"%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	DEBUG('s',"%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	DEBUG('s',"%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    DEBUG('s',"%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    DEBUG('s',"%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      DEBUG('s',"%s","Tried to close an unopen file\n");
    }
}

void Yield_Syscall()
{
	DEBUG('e',"inside yield syscall\n");
	currentThread->Yield();
}


SpaceId Exec_Syscall(unsigned int vaddr, int length)
{
	DEBUG('s',"inside exec system call");
	int q;
	int flag=1;

	 char *filename = new char[length+1];	// Kernel buffer to put the name in
	int a= copyin(vaddr,length, filename);

	if(a == -1) //i.e a wrong address was passed
		{
			DEBUG('s',"In exec: ERROR in Virtual Address\n");
			//locksmanagerLock->Release();
			return -1;
		}

	else
	{
	//DEBUG('s',"inside exec system call--------------1\n");

	OpenFile *executable = fileSystem->Open(filename);
	    AddrSpace *space;
	  //  DEBUG('s',"inside exec system call--------------2\n");
	    if (executable == NULL) {
		DEBUG('s',"Unable to open file %s\n", filename);
		return -1;
	    }
	    space = new AddrSpace(executable);
	    unsigned int m = space->get_numpages();				// storing end of addrspace


	    Thread *t;
	    t= new Thread ("exec_thread");
	    t->space= space;

	    pro_tablelock->Acquire();
	    //DEBUG('s',"inside exec system call--------------3\n");
	    pro_table.total_pro++;
	    for (int w=0;w<MAX_PROCESSES; w++)
	    {
	    	if(pro_table.pro_table_array[w].process_flag==0)
	    	{
	    		flag=0;
	    		pro_table.pro_table_array[w].process_flag=1;
	    		pro_table.pro_table_array[w].personal_addrspace=space;
	    		pro_table.pro_table_array[w].total_threads= 0;
	    		pro_table.pro_table_array[w].endof_addrspace= m;
	    		 q = w;
	    		break;
	    	}

	    }
	    if(flag ==1)
	    {
	    	DEBUG('s',"trying to fork more than 100 threads for same process...NO more THREADS ALLOWED to be forked ...........");
	    	currentThread->Finish();
	    }

	    pro_tablelock->Release();													////////////////// release pro_tablelock

	    t->Fork((VoidFunctionPtr)exec_thread,0);
	    //DEBUG('s',"inside exec system call--------------returning\n");
	    return q;
	}
}

void exec_thread(int no)
{

	currentThread->space->InitRegisters();		// set the initial register values
	   currentThread->space->RestoreState();		// load page table register
	   DEBUG('s',"inside exec system call--------------exec_thread\n");
	    machine->Run();			// jump to the user progam
	    //ASSERT(FALSE);			// machine->Run never returns;
						// the address space exits
						// by doing the syscall "exit"
}

///////////////////////////////////////////////////
//Fork()
///////////////////////////////////////////////////



void forking(unsigned int addrs)
{

	unsigned int m;
	machine->WriteRegister(PCReg,addrs );
		machine->WriteRegister(NextPCReg,addrs+4 );
		m=currentThread->space->stackalloc();
		machine->WriteRegister(StackReg,m*PageSize-16);
		currentThread->space->RestoreState();





		// printf("stack alocated at .............%d\n",m);

		 machine->Run();
}


void Fork_Syscall(unsigned int virtual_add)
{
	int flag1=1;
	//printf("inside fork syscall\n");
	Thread*t;
	 t= new Thread ("thread1");
	pro_tablelock->Acquire();
	int pro_index, thread_index;

	for (int i=0;i<MAX_PROCESSES; i++)
	{
		if (currentThread->space==pro_table.pro_table_array[i].personal_addrspace)
		{
			 pro_index=i;
			pro_table.pro_table_array[i].total_threads ++;
			for (int j=0;j<MAX_THREADS;j++)
			{
				if(pro_table.pro_table_array[i].thread_table_array[j].thread_flag==0)
				{
					flag1=0;
					pro_table.pro_table_array[i].thread_table_array[j].thread_flag=1;
					thread_index=j;
					t->space= currentThread->space;
					break;
				}
			}
			if(flag1==1)
			{
				DEBUG('s',"trying to fork more than 100 threads for same process...NO more THREADS ALLOWED to be forked ...........");
				currentThread->Finish();
			}
		break;

		}

	}
	pro_tablelock->Release();

	//fork_data_struct *fork_data_struct_obj = new struct fork_data_struct;

	//fork_data_struct_obj->pro_no= pro_index;
	//fork_data_struct_obj->thr_no= thread_index;
	//fork_data_struct_obj->addrs= virtual_add;

	t->Fork((VoidFunctionPtr)forking,virtual_add);

}







///////////////////////////////////////////////////
//createlock()
///////////////////////////////////////////////////

int CreateLock_Syscall(unsigned int virt_add,int size)
{
  DEBUG('s',"inside create system call");
  // first acquire the lock..
  //locksmanagerLock->Acquire();
  locksmanagerLock->Acquire();
  //Create a new lock and return its array index
  char* mybuf;
  mybuf = new char[size+1];

  if(size<=0 || size >50)
	{
		DEBUG('s',"In create_lock: name is too big or its too small");
		locksmanagerLock->Release();
		return -1;
	}

  //read in the string at the virtual address, copy it to a buffer(mybuf),and returns the number of bytes read in a
  int a = copyin(virt_add, size, mybuf);

   if(a == -1) //i.e a wrong address was passed
	{
		DEBUG('s',"In create_lock: ERROR in Virtual Address\n");
		locksmanagerLock->Release();
		return -1;
	}

  else //address is right..and we know the size of the name of lock in "a"
  {
		// first acquire the lock..


		DEBUG('s',"In create_lock: Now creating a lock \n");

		for(int i=0;i<totallocks;i++)
		{
			if(userlock[i].flag==0)//this i position is empty
			{
				userlock[i].flag=1;
				userlock[i].name=mybuf;//store name of lock

				userlock[i].add_space=currentThread->space;//giving address space of current process to the lock created..

				userlock[i].lock_usagecounter = 0;//initially no thread is using the lock

				userlock[i].lock=new Lock(mybuf);//create a new lock

				locksmanagerLock->Release();
				DEBUG('s',"In create_lock: lock no.%d created successfully %s\n",i,mybuf);
				return i;
			}
		}
		locksmanagerLock->Release();
		DEBUG('s',"In create_lock: No space for creating a Lock!!!\n");
		return -1;

  }
}

void Acquire_Syscall(unsigned int val)
{

	//first acquire the lock
	locksmanagerLock->Acquire();
	int value = val;//no. of the lock to be acquired
	if(value < 0 || value > 100)//illegal lock number
		{
			DEBUG('s',"In acquire_lock: ILLEGAL LOCK NUMBER\n");
			locksmanagerLock->Release();
			return;
		}

	    if(userlock[value].flag == 0)
		{
		    DEBUG('s',"In acquire_lock:LOCK DOES NOT EXIST\n");
			locksmanagerLock->Release();
			return;
	   }

		//check if process trying to acquire lock has same address space as that of lock..
		if(userlock[value].add_space != currentThread->space)
		{
			DEBUG('s',"curr_space=%d and %d",currentThread->space, userlock[value].add_space);
			DEBUG('s',"In acquire_lock: LOCK BELONGS TO DIFFERENT PROCESS\n");
			locksmanagerLock->Release();
			return;
		}
	DEBUG('s',"inside acquire system call for lock %s\n",userlock[val].name);




	// all clear...now lock can be acquired
		userlock[value].lock_usagecounter++;
		DEBUG('s',"lock %s is acquired\n",userlock[value].name);


		locksmanagerLock->Release();

		userlock[value].lock->Acquire();


		return;

}

void Release_Syscall(unsigned int val)
{
	int value = val;//no. of the lock to be released
	//first acquire the lock
	locksmanagerLock->Acquire();

	if(value < 0 || value > 100)//illegal lock number
		{
			DEBUG('s',"In release_lock: ILLEGAL LOCK NUMBER\n");
			locksmanagerLock->Release();
			return;
		}

		if(userlock[value].flag == 0)
		{
		    DEBUG('s',"In release_lock: LOCK DOES NOT EXIST ....\n");
			locksmanagerLock->Release();
			return;
	    }

		//check if process trying to acquire lock has same address space as that of lock..
		if(userlock[value].add_space != currentThread->space)
		{
			DEBUG('s',"In release_lock: LOCK BELONGS TO DIFFERENT PROCESS\n");
			locksmanagerLock->Release();
			return;
		}
	DEBUG('s',"inside release system call for lock\n",userlock[val].name);




	//decreasing no. of thread which are not using this lock

	DEBUG('s',"lock %s is released\n",userlock[value].name);

	userlock[value].lock_usagecounter--;
	// all clear...now lock can be released
	locksmanagerLock->Release();
	userlock[value].lock->Release();

	return;

}

void DestroyLock_Syscall(unsigned int val)
{
	//first acquire the lock..
	locksmanagerLock->Acquire();

	int value=val;//no. of the lock to be destroyed

	if(value < 0 || value > 100)//illegal lock number
	{
		DEBUG('s',"In destroy_lock: ILLEGAL LOCK NUMBER\n");
		locksmanagerLock->Release();
		return;
	}

	//if address spaces dont match
	if(userlock[value].add_space != currentThread->space)
	{
		DEBUG('s',"In destroy_lock: LOCK BELONGS TO DIFFERENT PROCESS so cannot destroy\n");
		locksmanagerLock->Release();
		return;
	}

	//if no lock at this position
	if(userlock[value].flag == 0)
	{
		DEBUG('s',"In destroy_lock: LOCK HAS already BEEN DESTROYED\n");
		locksmanagerLock->Release();
		return;
    }

	//if the lock is in use, no destruction
    if(userlock[value].lock_usagecounter > 0)
	{
		DEBUG('s',"In destroy_lock: the lock #%d is in use!!! so it cannot be destroyed\n",value);
		locksmanagerLock->Release();
		return;
    }

    //all clear to destroy the lock
    if(userlock[value].lock_usagecounter==0)
    {
		userlock[value].flag= 0;
		DEBUG('s',"\ndestroying the lock # %s\n",userlock[value].name);
		locksmanagerLock->Release();
		return;
    }
}

int CreateCV_Syscall(unsigned int virt_add,int size)
{
	cvsmanagerLock->Acquire();
  //Create a new condition and return its array index
  char* mybuf;
  mybuf = new char[size];

  //check if size of name is too big
  if(size<=0 || size >50)
	{
		DEBUG('s',"In create_CV: name is too big or its too small\n");
		cvsmanagerLock->Release();
		return -1;
	}

  //read in the string at the virtual address, copy it to a buffer(mybuf),and returns the number of bytes read in a
  int a = copyin(virt_add, size, mybuf);

  if(a == -1) //i.e a wrong address was passed
	{
		DEBUG('s',"In create_CV:ERROR in Virtual Address\n");
		cvsmanagerLock->Release();
		return -1;
	}

  else //address is right..and we know the size of the name of lock in a
  {
		//first acquire the lock

		DEBUG('s',"In create_CV: creating a new condition \n");

		for(int i=0;i<totalcvs;i++)
		{
			if(usercv[i].flag==0)//this i position is empty
			{
				usercv[i].flag=1;
				usercv[i].name=mybuf; //store name of condition

				usercv[i].add_space=currentThread->space;//giving address space of current process to the condition created..

				usercv[i].cv_usagecounter = 0;//initially no thread is using the lock

				usercv[i].condition=new Condition(mybuf);//create a new condition

				DEBUG('s',"In create_CV: condition no %d variable  %s created!!!\n",i,mybuf);
				cvsmanagerLock->Release();
				return i;
			}
		}
		cvsmanagerLock->Release();
		DEBUG('s',"In create_CV: No space for creating a Condition!!!\n");
		return -1;

  }
}

void DestroyCV_Syscall(unsigned int val)
{
	//first acquire the lock
	cvsmanagerLock->Acquire();

	int value=val;//no. of the condition to be destroyed

	if(value < 0 || value > 100)//illegal condition number
	{
		DEBUG('s',"In destroy_CV:ILLEGAL CONDITION NUMBER\n");
		cvsmanagerLock->Release();
		return;
	}

	//address spaces dont match
	if(usercv[value].add_space != currentThread->space)
	{
		DEBUG('s',"In destroy_CV: Condition BELONGS TO DIFFERENT PROCESS so cannot destroy\n");
		cvsmanagerLock->Release();
		return;
	}

	//if no cv at this index
	if(usercv[value].flag==0)
	{
		DEBUG('s',"In destroy_CV:condition HAS already BEEN DESTROYED\n");
		cvsmanagerLock->Release();
		return;
    }

	//if the condition is in use,no destruction
	if(usercv[value].cv_usagecounter > 0)
	{
		DEBUG('s',"In destroy_CV: the condition is in use!!!\n");
		cvsmanagerLock->Release();
		return;
	}

	//all clear to destroy the cv
	if(usercv[value].cv_usagecounter==0)
    {

		usercv[value].flag==0;
		DEBUG('s',"In destroy_CV: destroying condition variable number %s\n",usercv[value].name);
		cvsmanagerLock->Release();
		return;
	}
}

void Signal_Syscall(unsigned int val, int id)
{
	//first acquire the lock
	locksmanagerLock->Acquire();

	int value=val;//no. of the condition variable
	int lock_id=id;//no. of the lock on which wait is called

	if(lock_id < 0 || lock_id > 100)//illegal lock number
	{
		DEBUG('s',"In Signal: illegal lock number\n");
		locksmanagerLock->Release();
		return;
	}

	//check if process have access to the lock variable
	if(userlock[lock_id].add_space != currentThread->space)
	{
		DEBUG('s',"In Signal: Lock belongs to different process so cannot use it\n");
		locksmanagerLock->Release();
		return;
	}

	//check if lock is destroyed
	if(userlock[lock_id].flag==0)
	{
	    DEBUG('s',"In Signal: lock has been destroyed\n");
		locksmanagerLock->Release();
		return;
    }

	//locksmanagerLock->Release();

	cvsmanagerLock->Acquire();


	if(value < 0 || value > 100)//illegal condition number
	{
		DEBUG('s',"In Signal: illegal condition number\n");
		cvsmanagerLock->Release();
		return;
	}

	//check if process have access to the condition variable
	if(usercv[value].add_space != currentThread->space)
	{
		DEBUG('s',"In Signal: Condition belongs to different process so cannot use it\n");
		cvsmanagerLock->Release();
		return;
	}

	//check if cv is destroyed
	if(usercv[value].flag==0)
	{
		DEBUG('s',"In Signal: condition has already been destroyed\n");
		cvsmanagerLock->Release();
		return;
    }


	cvsmanagerLock->Release();
		locksmanagerLock->Release();
		usercv[value].cv_usagecounter--;
	//all clear now condition can signal on the lock...
	DEBUG('s',"signal done on lock # %s by condition # %s\n",userlock[lock_id].name,usercv[value].name);
	usercv[value].condition->Signal(userlock[lock_id].lock);

	//decrement condition usage counter



	return;

}

void Wait_Syscall(unsigned int val, int id)
{
	//first acquire the lock
	locksmanagerLock->Acquire();

	int value=val;//no. of the condition variable
	int lock_id=id;//no. of the lock on which wait is called

	if(lock_id < 0 || lock_id > 100)//illegal lock number
	{
		DEBUG('s',"In Wait: illegal lock number\n");
		locksmanagerLock->Release();
		return;
	}

	//check if process have access to the lock variable
	if(userlock[lock_id].add_space != currentThread->space)
	{
		DEBUG('s',"In Wait: Lock belongs to different process so cannot use it\n");
		locksmanagerLock->Release();
		return;
	}

	//check if lock is destroyed
	if(userlock[lock_id].flag==0)
	{
	    DEBUG('s',"In Wait: lock has been destroyed\n");
		locksmanagerLock->Release();
		return;
    }

	//userlock[value].lock_usagecounter++;

	//locksmanagerLock->Release();

	cvsmanagerLock->Acquire();


	if(value < 0 || value > 100)//illegal condition number
	{
		DEBUG('s',"In Wait: illegal Condition number\n");
		cvsmanagerLock->Release();
		return;
	}

	//check if process have access to the condition variable
	if(usercv[value].add_space != currentThread->space)
	{
		DEBUG('s',"In Wait: Condition belongs to different process so cannot use it\n");
		cvsmanagerLock->Release();
		return;
	}

	//check if cv is destroyed
	if(usercv[value].flag==0)
	{
		DEBUG('s',"In Wait: condition has already been destroyed\n");
		cvsmanagerLock->Release();
		return;
    }

	usercv[value].cv_usagecounter++;

	DEBUG('s',"In Wait: wait done on lock # %s by condition # %s\n",userlock[lock_id].name,usercv[value].name);
	cvsmanagerLock->Release();
	locksmanagerLock->Release();


	//all clear now condition can signal on the lock...

	usercv[value].condition->Wait(userlock[lock_id].lock);

	return;

}

void Broadcast_Syscall(unsigned int val,int id)
{
	DEBUG('s',"in broadcast\n");
	//first acquire the lock
	locksmanagerLock->Acquire();

	int value=val;//no. of the condition variable
	int lock_id=id;//no. of the lock on which wait is called

	if(lock_id < 0 || lock_id > 100)//illegal lock number
	{
		DEBUG('s',"In Broadcast: the lock number is wrong!!!\n");
		locksmanagerLock->Release();
		return;
	}

	//check if process have access to the lock variable
	if(userlock[lock_id].add_space != currentThread->space)
	{
		DEBUG('s',"In Broadcast: Lock belongs to different process so cannot use it\n");
		locksmanagerLock->Release();
		return;
	}

	//check if lock is destroyed
	if(userlock[lock_id].flag==0)
	{
	    DEBUG('s',"In broadcast: lock has been destroyed\n");
		locksmanagerLock->Release();
		return;
    }

	cvsmanagerLock->Acquire();

	if(value < 0 || value > 100)//illegal condition number
	{
		DEBUG('s',"In Broadcast: illegal condition number\n");
		cvsmanagerLock->Release();
		return;
	}

	//check if process have access to the condition variable
	if(usercv[value].add_space != currentThread->space)
	{
		DEBUG('s',"In Broadcast: condition belongs to different process so cannot use it\n");
		cvsmanagerLock->Release();
		return;
	}

	//check if condition is destroyed
	if(usercv[value].flag==0)
	{
		DEBUG('s',"In Broadcast: condition has already been destroyed\n");
		cvsmanagerLock->Release();
		return;
    }

	//all clear now condition can signal on the lock...
	DEBUG('s',"In Broadcast: now broadcasting the threads waiting on lock# %d and condition# %d\n",lock_id,value);
	int j=usercv[value].condition->Broadcast(userlock[lock_id].lock);

	usercv[value].cv_usagecounter -=j;
	cvsmanagerLock->Release();

	userlock[value].lock_usagecounter -=j;
	locksmanagerLock->Release();

	return;
}


void Exit_Syscall()
{
	int active_threads=0;

	pro_tablelock->Acquire();

	for(int i=0;i<MAX_PROCESSES;i++)
	{
		//check if address space matches
		if(pro_table.pro_table_array[i].personal_addrspace== currentThread->space)
		{
			//if number of threads is 0

			if(pro_table.pro_table_array[i].total_threads==0)
			{
				//if last process
				if(pro_table.total_pro==1)
				{
					DEBUG('s',"1. exiting the last thread of last process ");
					 interrupt->Halt();
				}
				//if not the last process
				else
				{
					DEBUG('s',"2. exiting the last thread of a process.... deleting process addressspace ");
					pro_table.total_pro--;
					//delete currentThread->space;
				}
			}
			//if thread is calling exit
			else
			{
				DEBUG('s',"3. finishing a thread  ");
				pro_table.pro_table_array[i].total_threads--;
				currentThread->Finish();
			}

		}

	}


	pro_tablelock->Release();


}



//////////////////////////////////////////
//printf with no args
/////////////////////////////////////////


void Printf_Syscall(unsigned int add)
{

	char* mybuf;
	  mybuf = new char[100];

	  //read in the string at the virtual address, copy it to a buffer(mybuf),and returns the number of bytes read in a
	  int a = copyin(add, 100, mybuf);

	  if(a == -1) //i.e a wrong address was passed
		{
			printf("In printf:ERROR in Virtual Address\n");
			//cvsmanagerLock->Release();
			return ;
		}

	  else //address is right..and we know the size of the name of lock in a
	  {
		  printf("%s",mybuf);
		  return;
	  }
}

//////////////////////////////////////////
//printf with 1 args
//////////////////////////////////////////


void Printf1_Syscall(unsigned int add,int arg2)
{
	int arg=arg2;
	char* mybuf;
	  mybuf = new char[50];


	  //read in the string at the virtual address, copy it to a buffer(mybuf),and returns the number of bytes read in a
	  int a = copyin(add, 50, mybuf);

	  if(a == -1) //i.e a wrong address was passed
		{
			printf("In printf 1:ERROR in Virtual Address\n");
			//cvsmanagerLock->Release();
			return ;
		}

	  else //address is right..and we know the size of the name of lock in a
	  {
		  printf("%s %d",mybuf,arg);
		  return;
	  }
}



int Scanf_Syscall(void)
{

	int q;
		 scanf("%d",&q);
		  return q;

}



void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
	    case SC_Exec:
	    DEBUG('q',"Test Exec switch case\n");
	   	DEBUG('a', "Exec syscall.\n");
	   	rv= Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
	   	break;

	   	case SC_Fork:
	   	DEBUG('q', "Fork system call. \n");
	   	Fork_Syscall(machine->ReadRegister(4));
	   	break;
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4), machine->ReadRegister(5),machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;
	    case SC_Yield:
	   	DEBUG('a', "Yield syscall.\n");
	   	Yield_Syscall();
	   	break;
	    case SC_CreateLock:
	    DEBUG('a', "CreateLock syscall.\n");
	    rv=CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
	    break;
	    case SC_Acquire:
	   	DEBUG('q', "Acquire system call. \n");
	    Acquire_Syscall(machine->ReadRegister(4));
	    break;
	    case SC_Release:
	    DEBUG('q', "Release system call. \n");
	    Release_Syscall(machine->ReadRegister(4));
	    break;

	    case SC_DestroyLock:
	    DEBUG('q', "DestroyLock system call. \n");
	     DestroyLock_Syscall(machine->ReadRegister(4));
	     break;

	   	    case SC_CreateCV:
	   	    DEBUG('q', "CreateCondition system call.\n");
	   	    rv = CreateCV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
	   	    break;

	   	    case SC_DestroyCV:
	   	    DEBUG('q', "DestroyCondition system call. \n");
	   	    DestroyCV_Syscall(machine->ReadRegister(4));
	   	    break;

	   	    case SC_Signal:
	           DEBUG('q', "Signal system call. \n");
	           Signal_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
	           break;

	   	    case SC_Wait:
	   	    DEBUG('q', "Wait system call. \n");
	   	    Wait_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
	   	    break;

	   	case SC_Broadcast:
	   	       DEBUG('q', "Broadcast system call. \n");
	   	       Broadcast_Syscall(machine->ReadRegister(4),
	   	 						machine->ReadRegister(5));
	   	       break;
	    case SC_Exit:
	   	           DEBUG('q', "exit system call. \n");
	   	           Exit_Syscall();
	   	           break;
	    case SC_Printf:
	   	        DEBUG('q', "printf system call. \n");
	   	       	 Printf_Syscall(machine->ReadRegister(4));
  	           break;
	    case SC_Scanf:
	   	   	        DEBUG('q', "printf system call. \n");
	   	   	       	 rv=Scanf_Syscall();
	     	           break;
	    case SC_Printf1:
	    	   	        DEBUG('q', "printf system call. \n");
	    	   	       	 Printf1_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
	      	           break;




	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
