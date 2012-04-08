//globalItems.cc
#include "copyright.h"
#include "system.h"
#include "thread.h"
#include "list.h"
#include "synch.h"


//defining total no of workers in simulation
#define WAITERS 3
#define ORDERTAKERS 3
#define COOKS 4
#define CUSTOMERS 20

//General Global Variables

int linelength=0;
enum status {WAIT,FREE,BUSY};
status ordertakerstatus[ORDERTAKERS];


//Global Locks used in simulation
Lock *customerlinelock;
Lock *customerordertakerlock[ORDERTAKERS];

//Global Condition Variables used in simulation
Condition *customerlineCV;
Condition *cutomerordertakerCV[ORDERTAKERS];

//Global Monitor Variables used in simulation

//initializing

void main()
{

	for(int i=0; i<3; i++)
{
	ordertakerstatus[i] = BUSY;
	//customerordertakerCV[i] = NULL;
	//customer

}


	Thread *t;



	    // Test 1

	    printf("Starting Test 1\n");



	    t = new Thread("customer");
	    t->Fork((VoidFunctionPtr)customer,1);


	    for (int i=0; i<3; i++)
	    {
	    t = new Thread("ordertaker");
	    	    t->Fork((VoidFunctionPtr)ordertaker,i);

	    }







}
