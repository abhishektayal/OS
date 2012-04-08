/*
 * carljr_main.h
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


//defining total no of workers in simulation
	#define WAITERS 3
	#define ORDERTAKERS 3
	#define COOKS 4
	#define CUSTOMERS 20

	//General Global Variables
	//int j=0;
	int linelength=0;
	int index_orderbook=-1;					// gives the next available index in the order book
	int token_number;




	// enum declarations

	enum status {WAIT,FREE,BUSY};						// for keeping account of order taker's status
	status ordertakerstatus[ORDERTAKERS];

	enum eating_pref {GET_IN,TO_GO};					// for keeping account of customer's eating preference

	enum customer_status {FINISH, PENDING};				// for keeping account of customer's order status



	//Global Locks used in simulation
	Lock *customerlinelock;
	Lock *customerordertakerlock[ORDERTAKERS];
	Lock *ordermantainlock;
	Lock *togo_waitinglock;

	//Global Condition Variables used in simulation
	Condition *customerlineCV;
	Condition *customerordertakerCV[ORDERTAKERS];
	Condition *ordermantainCV;
	Condition *togo_waitingCV;


