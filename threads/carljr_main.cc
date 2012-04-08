/*
 * carljr_main.cc
 *
 *  Created on: Sep 12, 2010
 *      Author: Abhi
 */


//globalItems.cc
#include "copyright.h"
#include "system.h"
#include "thread.h"
#include "list.h"
#include "synch.h"
#include "carljr_func.h"
#include "carljr_main.h"


void TestSuite()
{

/***********************************************************
	// initializing locks and CV's
***********************************************************/
	customerlinelock= new Lock("customerlinelock");
	customerlineCV= new Condition("customerlineCV");

			/*
			 *
			 for(int k=0; k<ORDERTAKERS; k++)
			{
				char *	name,*name1;
				char *no=(char*) k;

				name=(char*) "customerordertakerlock";
				name1=(char*) "customerordertakerCV";

				customerordertakerlock[k]= new Lock(name);
				customerordertakerCV[k]= new Condition(name1);
			}
*/

	customerordertakerlock[0]= new Lock("customerordertakerlock0");
		customerordertakerCV[0]= new Condition("customerordertakerCV0");


		customerordertakerlock[1]= new Lock("customerordertakerlock1");
		customerordertakerCV[1]= new Condition("customerordertakerCV1");


		customerordertakerlock[2]= new Lock("customerordertakerlock2");
		customerordertakerCV[2]= new Condition("customerordertakerCV2");

		ordermantainlock= new Lock("ordermantainlock");
		ordermantainCV= new Condition("ordermantainCV");


	//initializing
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

	    t = new Thread("customer1");
	   	    t->Fork((VoidFunctionPtr)customer,2);

	   	 t = new Thread("customer2");
	   		    t->Fork((VoidFunctionPtr)customer,3);

	   		 t = new Thread("customer3");
	   			    t->Fork((VoidFunctionPtr)customer,4);






	    	//otname=strcat(otname,no);
	    	t = new Thread("ordertaker0");
	    	    t->Fork((VoidFunctionPtr)ordertaker,0);

	    	    t = new Thread("ordertaker1");
	    	    	    	    t->Fork((VoidFunctionPtr)ordertaker,1);

	    	    	    	    t = new Thread("ordertaker2");
	    	    	    	    	    	    t->Fork((VoidFunctionPtr)ordertaker,2);

	    }


