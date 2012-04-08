/*
 * backup.cc
 *
 *  Created on: Sep 20, 2010
 *      Author: Abhi
 */

/******************************************************************************************************************
								TITLE: CARLS JUNIOR SIMULATION
										PROJECT !: part2

submitted by:		Abhishek Tayal				Priyanshu Gupta 			Mihir Shah
email id's:			atayal@usc.edu				priyansg@usc.edu			mihirash@usc.edu



/****************************************************************************************************************** */

#include "copyright.h"
#include "system.h"
#include "thread.h"
#include "list.h"
#include "synch.h"


/***********************************************
DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
************************************************/
#define WAITERS 5
#define ORDERTAKERS 5
#define COOKS 5
#define CUSTOMERS 30
#define TABLES	10


/************************************************
ENUM DEFINITIONS :
***********************************************/

	enum status {WAIT,FREE,BUSY};				// for keeping account of order taker's status
	status ordertakerstatus[ORDERTAKERS];

	enum eating_pref {GET_IN=1,TO_GO};			// for keeping account of customer's eating preference

	enum customer_status {FINISH, PENDING};		// for keeping account of customer's order status taken by ordertaker
	customer_status status[ORDERTAKERS];

	enum order_status { FOOD_GIVEN, FOOD_PENDING, FOOD_PACKED};		// for keeping acct of food delivery status in order book
	order_status order_stat;

	enum customer_seating_status { GOT_SEAT, NO_SEAT};			// for keeping track of th customers seating status
	customer_seating_status customer_seating_stat;

	enum cook_status {COOK_FREE,D6B,D3B,SB,FRIES};				// to keep track of what cook is cooking or is free
	cook_status cook_stat[COOKS];

	enum table_status {TABLE_FREE, TABLE_OCCUPIED};				//to keep account of table occupancy status
	table_status table_stat[TABLES];

	enum waiter_status {WAITER_FREE, WAITER_BUSY };				//to keep account of waiter status
		waiter_status waiter_stat[WAITERS];

	enum is_mang_takingorder {YES, NO};							// for customer to know whether he can go to manager to place order
	is_mang_takingorder is_mang_takingorder1;

	enum mang_order_status{ORDER_FINISH, ORDER_PENDING};		// for keeping account of customer's order status taken by manager



/****************************************************
GENERAL GLOBAL VARIABLES DECLARATIONS
****************************************************/

	int linelength=0;							// for number of customers in line to place order
	int index_orderbook=-1;						// gives the available index in the order book
	int token_number[ORDERTAKERS];
	int d6b[ORDERTAKERS];
	int d3b[ORDERTAKERS];
	int sb[ORDERTAKERS];
	int soda[ORDERTAKERS];
	int fries[ORDERTAKERS];
	eating_pref eat_pref[ORDERTAKERS];
	int customer_token_number[CUSTOMERS];
	int number_of_free_tables= TABLES;
	int tableline=0;
	int manager_customer_token;
	int new_cooks=0;
	int waiter_tokennumber[WAITERS];
	int tobe_service_tablenumber[WAITERS];
	int togo_food_ready_token;
	int totalmoney=0;
	int money_received[ORDERTAKERS];
	int money_received_manager;
	int ot_customerno[ORDERTAKERS];
	int manager_customerno;
	int customers_left;


/******************************************************
* DECLARING ALL THE LOCKS USED IN SIMULATION
*******************************************************/
	Lock *customerlinelock;
	Lock *customerordertakerlock[ORDERTAKERS];
	Lock *ordermantainlock;
	Lock *togo_waitinglock;
	Lock *tablelinelock;
	Lock *manager_customer_tablelock;
	Lock *manager_cooklock;
	Lock *total_foodlock;
	Lock *inventorylock;
	Lock *tablelock[TABLES];
	Lock *waiterlock[WAITERS];
	Lock *manager_cust_orderlock;
	Lock *totalmoneylock;
	Lock *customer_gonelock;

/******************************************************
DECLARING ALL THE CONDITION VARIABLES USED IN SIMULATION
*******************************************************/
	Condition *customerlineCV;
	Condition *customerordertakerCV[ORDERTAKERS];
	Condition *ordermantainCV;
	Condition *togo_waitingCV;
	Condition *tablelineCV;
	Condition *manager_customer_tableCV;
	Condition *manager_cookCV;
	Condition *total_foodCV;
	Condition *inventoryCV;
	Condition *tableCV[TABLES];
	Condition *waiterCV[WAITERS];
	Condition *manager_cust_orderCV;


	/****************************************************************************
	ORDER BOOK structure
	 *   structure to hold order information
	1)	customer no: customer_no
	2)  eating preference: eat_pref
	ORDER:
	1) $6 brg: d6b
	2) $3 brg: d3b
	3) simple brg: sb
	4) soda: soda
	5) fries: fries
	*****************************************************************************	*/

	struct order_book
	{

	int customer_no;
	eating_pref eat_pref;
	int d6b,d3b,sb,fries,soda,table_number;
	order_status order_stat;
	customer_seating_status customer_seating_stat;

	};


	order_book* order_book_details[100];	// array of structure containing customer details , so as to contain
										//	the details of all the customers.


/*************************************************************************************
	  INVENTORY Structure
	structure to hold inventory information
	1) $6 brg: d6b
	2) $3 brg: d3b
	3) simple brg: sb
	4) fries: fries

**************************************************************************************/

	struct inventory
	{
		int d6b,d3b,sb,fries;
	}*inventory_obj;

/*************************************************************************************
	 	 AVAILABLE_ITEM Structure
		structure to hold inventory information
		1) $6 brg: d6b
		2) $3 brg: d3b
		3) simple brg: sb
		4) fries: fries

**************************************************************************************/

		struct available_items
		{
			int d6b,d3b,sb,fries;
		}*available_items_obj;

/*************************************************************************************
		 ORDER_PENDING Structure
			structure to hold inventory information
			1) $6 brg: d6b
			2) $3 brg: d3b
			3) simple brg: sb
			4) fries: fries

**************************************************************************************/

			struct pending_order
			{
				int d6b,d3b,sb,fries;
			}*pending_order_obj;

/*************************************************************************************
	 MANAGER_ORDERBOOK Structure
						structure to hold order given to the manager
						1) $6 brg: d6b
						2) $3 brg: d3b
						3) simple brg: sb
						4) fries: fries
						5) soda: soda
						6) eating pref: eat_pref
**************************************************************************************/

						struct manager_orderbook
						{
							int d6b,d3b,sb,fries,soda,token_number;
							eating_pref eat_pref;
							mang_order_status mang_order_status_obj;
						}*manager_orderbook_obj;

/*************************************************************************************
		CUSTOMER_ORDER Structure
			structure to hold inventory information
			1) $6 brg: d6b
			2) $3 brg: d3b
			3) simple brg: sb
			4) fries: fries

**************************************************************************************/
						struct customer_order
								{
									int d6b,d3b,sb,fries,soda;
									eating_pref eat_pref;

								}*customer_order_obj[50];



/******************************************************
* DECLARING ALL THE FUNCTIONS USED IN SIMULATION
*******************************************************/
	void ordertaker(int);
	void customer(int);
	void manager(void);
	void cook(int);
	void waiter(int);
	void testcalls(void);
	void test1(void);
	void test2(void);
	void test3(void);
	void test4(void);
	void test5(void);
	void test6(void);
	void test7(void);
	void test8(void);
	void test9(void);
	void test10(void);
	void test11(void);
	void test12(void);
	void test13(void);
	void test14(void);
	void test15(void);
	void full_simulation(void);


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


void TestSuite() {


#define AUTO_ORDER

		// allocating memory to  structures
	for (int p=0;p<50;p++)
		{
			order_book_details [p]=(order_book*)malloc(sizeof(order_book));
		}

	for (int p=0;p<50;p++)
	{
		customer_order_obj [p]= (customer_order*)malloc(sizeof(customer_order));
	}



	available_items_obj= new available_items;
	pending_order_obj = new pending_order;
	inventory_obj= new inventory;
	manager_orderbook_obj = new manager_orderbook;


	/***********************************************************
	// initializing locks and CV's
	***********************************************************/
	customerlinelock= new Lock("customerlinelock");
	customerlineCV= new Condition("customerlineCV");


	 for(int k=0; k<ORDERTAKERS; k++)
	{
	char *	name,*name1;

	name=(char*) "customerordertakerlock";
	name1=(char*) "customerordertakerCV";

	sprintf(name,name,k );
	sprintf(name1,name1,k );

	customerordertakerlock[k]= new Lock(name);
	customerordertakerCV[k]= new Condition(name1);
	}


	for(int k=0; k<TABLES; k++)
		{
		 	 char *name,*name1;

		 	 name=(char*) "tablelock";
		 	 name1=(char*) "tableCV";

		 	 sprintf(name,name,k );
		 	 sprintf(name1,name1,k );

		 	 tablelock[k]= new Lock(name);
		 	 tableCV[k]= new Condition(name1);
		}

	for(int k=0; k<WAITERS; k++)
		{
		 	 char *name,*name1;

		 	 name=(char*) "waiterlock";
		 	 name1=(char*) "waiterCV";

		 	 sprintf(name,name,k );
		 	 sprintf(name1,name1,k );

		 	 waiterlock[k]= new Lock(name);
		 	 waiterCV[k]= new Condition(name1);
		}



	 tablelinelock= new Lock("tablelinelock");
	 tablelineCV = new Condition("tablelineCV");

	 manager_customer_tablelock=new Lock("manager_customer_tablelock");
	 manager_customer_tableCV = new Condition("manager_customer_tableCV");

	 ordermantainlock = new Lock ("ordermantainlock ");
	 ordermantainCV = new Condition("ordermantainCV ");

	 manager_cooklock=new Lock ("manager_cooklock");
	 manager_cookCV=new Condition("manager_cookCV");

	 total_foodlock=new Lock("total_foodlock");
	 total_foodCV=new Condition("total_foodCV");

	 inventorylock= new Lock("inventorylock");
	 inventoryCV= new Condition("inventoryCV");

	 manager_cust_orderlock=new Lock("manager_cust_orderlock");
	 manager_cust_orderCV= new Condition("manager_cust_orderCV");

	 togo_waitinglock=new Lock("togo_waitinglock");
	 togo_waitingCV=new Condition("togo_waitingCV");

	 totalmoneylock=new Lock("totalmoneylock");

	 customer_gonelock=new Lock("customer_gonelock");


	//initializing the arrays d6b,d3b,sb,soda,fries


	//initializing
	for(int i=0; i<ORDERTAKERS; i++)								// order takers personal order book
	{
	ordertakerstatus[i] = BUSY;
	d6b[i]=NULL;
	d3b[i]=NULL;
	sb[i]=NULL;
	soda[i]=NULL;
	fries[i]=NULL;

	}

	for (int z=0;z<COOKS;z++)						// initializing all cooks status to be free
	{
		cook_stat[z]=COOK_FREE;
	}

	available_items_obj->d3b=100;
	available_items_obj->d6b=100;
	available_items_obj->sb=500;
	available_items_obj->fries=500;



	pending_order_obj->d3b=0;
	pending_order_obj->d6b=0;
	pending_order_obj->sb=0;
	pending_order_obj->fries=0;

	inventory_obj->d3b=1000;
	inventory_obj->d6b=1010;
	inventory_obj->sb=1000;
	inventory_obj->fries=1000;

	is_mang_takingorder1= NO;
	customers_left=0;


	testcalls();
}



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


void testcalls ()
{
//while(1)
{
 int input;

 DEBUG('q',"\n\n1 to Test Customers who wants to eat-in, must wait if the restaurant is full\n");
 DEBUG('q',"2 to Test OrderTaker/Manager gives order number to the Customer when the food is not ready\n");
 DEBUG('q',"3 to Test Customers who have chosent to eat-in, must leave after they have their food and Customers who have chosen to-go, must leave the restaurant after the OrderTaker/Manager has given the food\n");
 DEBUG('q',"4 to Test Manager maintains the track of food inventory. Inventory is refilled when it goes below order level\n");
 DEBUG('q',"5 to Test A Customer who orders only soda need not wait\n");
 DEBUG('q',"6 to Test The OrderTaker and the Manager both somethimes bag the food\n");
 DEBUG('q',"7 to Test Manager goes to the bank for cash when inventory is to be refilled and there is no cash in the restaurant\n");
 DEBUG('q',"8 to Test Cooks goes on break when informed by manager\n");
 DEBUG('q',"9 only soda & its get-in  then we have to show that customer gets soda but will wait for his seat\n");
 DEBUG('q',"10 manager assigns tables to eat-in customer\n");
 DEBUG('q',"11 customer has choosen to_go and food is available so its given instantly\n");
 DEBUG('q',"12 Manager forks new cook if all other cooks are busy\n");
 DEBUG('q',"13 when to_go food is ready its broadcasted by manager or ordertaker\n");
 DEBUG('q',"14 waiters will give food to the get_in customers whose token number has been obtained from manager /ordertaker\n");
 DEBUG('q',"15 when ordertakers are busy.... manager works as an ordertaker \n");
 DEBUG('q',"16 full simulation \n");


 scanf ("%d",&input);

 switch ( input )
 {
  case 1:
    test1();
    break;
  case 2:
    test2();
    break;
  case 3:
    test3();
    break;
  case 4:
    test4();
    break;
  case 5:
    test5();
    break;
  case 6:
    test6();
    break;
  case 7:
    test7();
    break;
  case 8:
    test8();
    break;
  case 9:
    test9();
    break;
  case 10:
    test10();
    break;
  case 11:
    test11();
    break;
  case 12:
    test12();
    break;
  case 13:
    test13();
    break;
  case 14:
    test14();
    break;
  case 15:
    test15();
    break;
  case 16:
	 full_simulation();
    break;
 }
}

}



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////



void test1()
{
#define MANUAL_ORDER


	/***********************************************
	DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
	************************************************/
	#define WAITERS 1
	#define ORDERTAKERS 1
	#define COOKS 1
	#define CUSTOMERS 1

	DEBUG('q', "STARTING TEST 1 !!!!!!!!!!!!!!!!!!!!");



 customer_order_obj[0] ->d6b=rand () % 2;        // placing random order
 customer_order_obj [0]->d3b= rand () % 2;        // placing random order
 customer_order_obj [0]->sb= rand () % 2;         // placing random order
 customer_order_obj [0]->fries= rand () % 2;      // placing random order
 customer_order_obj [0]->soda= rand () % 2;       // placing random order
 customer_order_obj [0]->eat_pref= GET_IN;

 for (int x=0;x<TABLES;x++)             // Initializing All Tables as Occupied
 {
  table_stat[x]=TABLE_OCCUPIED;
 }

 Thread *t;
 t = new Thread("customer0");
   t->Fork((VoidFunctionPtr)customer,0);
 t = new Thread("ordertaker0");
   t->Fork((VoidFunctionPtr)ordertaker,0);
   t = new Thread("manager");
      t->Fork((VoidFunctionPtr)manager,0);
      t = new Thread("waiter");
         t->Fork((VoidFunctionPtr)waiter,0);
         t = new Thread("cook");
            t->Fork((VoidFunctionPtr)cook,0);


}

/////////////////////////////////////////////////////////////////////////////

void test2(void)
{

#define MANUAL_ORDER
	/***********************************************
		DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
		************************************************/
		#define WAITERS 1
		#define ORDERTAKERS 1
		#define COOKS 1
		#define CUSTOMERS 1

	DEBUG('q', "STARTING TEST 2 !!!!!!!!!!!!!!!!!!!!");


	// #define MANUAL_ORDER
	available_items_obj->d3b=0;
	 available_items_obj->d6b=0;
	 available_items_obj->sb=0;
	 available_items_obj->fries=0;

	 customer_order_obj[0] ->d6b =1;//rand () % 2;        // placing random order
	 customer_order_obj[0] ->d3b= rand () % 2;        // placing random order
	 customer_order_obj[0] -> sb= rand () % 2;         // placing random order
	 customer_order_obj[0] -> fries= rand () % 2;      // placing random order
	 customer_order_obj[0] -> soda= rand () % 2;       // placing random order
	 int w= rand () % 2;         // placing random order
	 if (w==0)
	  {
		 customer_order_obj[0]->eat_pref= TO_GO;
	  }
	 else
	  {
		 customer_order_obj[0] -> eat_pref= GET_IN;
	  }

	 Thread *t;
	  t = new Thread("customer0");
	    t->Fork((VoidFunctionPtr)customer,0);
	  t = new Thread("ordertaker0");
	    t->Fork((VoidFunctionPtr)ordertaker,0);
	    t = new Thread("manager");
	       t->Fork((VoidFunctionPtr)manager,0);
	       t = new Thread("waiter");
	          t->Fork((VoidFunctionPtr)waiter,0);
	          t = new Thread("cook");
	             t->Fork((VoidFunctionPtr)cook,0);



}


/////////////////////////////////////////////////////////////////////////////

void test3(void)
	{

	#define MANUAL_ORDER

	/***********************************************
		DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
		************************************************/
		#define WAITERS 2
	#define ORDERTAKERS 2
		#define COOKS 2
		#define CUSTOMERS 2


	DEBUG('q', "STARTING TEST 3 !!!!!!!!!!!!!!!!!!!!");


	customer_order_obj[0]->d6b =rand () % 2;        // placing random order
	customer_order_obj[0]-> d3b= rand () % 2;        // placing random order
	customer_order_obj[0]-> sb= rand () % 2;         // placing random order
	customer_order_obj[0]-> fries= rand () % 2;      // placing random order
	customer_order_obj[0]-> soda= rand () % 2;       // placing random order
	customer_order_obj[0]-> eat_pref= GET_IN;

	customer_order_obj[1]-> d6b =rand () % 2;        // placing random order
	customer_order_obj[1]->	 d3b= rand () % 2;        // placing random order
	customer_order_obj[1]-> sb= rand () % 2;         // placing random order
	customer_order_obj[1]-> fries= rand () % 2;      // placing random order
	customer_order_obj[1]-> soda= rand () % 2;       // placing random order
	customer_order_obj[1]-> eat_pref= TO_GO;

		 available_items_obj->d3b=200;
		 available_items_obj->d6b=200;
		 available_items_obj->sb=200;
		 available_items_obj->fries=200;

		 Thread *t;
		 t = new Thread("customer0");
		   t->Fork((VoidFunctionPtr)customer,0);
		 t = new Thread("customer1");
		   t->Fork((VoidFunctionPtr)customer,1);

		   t = new Thread("ordertaker1");
		   		   t->Fork((VoidFunctionPtr)ordertaker,0);
		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);

		 t = new Thread("waiter0");
		   t->Fork((VoidFunctionPtr)waiter,0);

		}


////////////////////////////////////////////////////////////////////////////

	void test4(void)
	{

#define MANUAL_ORDER

		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 1
			#define COOKS 1
			#define CUSTOMERS 1


	DEBUG('q', "STARTING TEST 4 !!!!!!!!!!!!!!!!!!!!");

	customer_order_obj[0]->d6b =3;        // placing random order
	customer_order_obj[0]-> d3b= 2;        // placing random order
	customer_order_obj[0]-> sb= 1;         // placing random order
	customer_order_obj[0]-> fries= rand () % 2;      // placing random order
	customer_order_obj[0]-> soda= rand () % 2;       // placing random order
		 int w= rand () % 2;         // placing random order
		  if (w==0)
		  {
			  customer_order_obj[0]-> eat_pref= TO_GO;
		  }
		 else
		  {
			 customer_order_obj[0]->eat_pref= GET_IN;
		  }
		 inventory_obj->d3b=0;
		 inventory_obj->d6b=0;
		 inventory_obj->sb=0;
		 inventory_obj->fries=0;

		 available_items_obj->d3b=0;
		 available_items_obj->d6b=0;
		 available_items_obj->sb=0;
		 available_items_obj->fries=0;


		Thread *t;
		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);

		 t = new Thread("customer0");
		   t->Fork((VoidFunctionPtr)customer,0);

		 t = new Thread("cook1");
		   t->Fork((VoidFunctionPtr)cook,0);

		   t = new Thread("ordertaker1");
		   t->Fork((VoidFunctionPtr)ordertaker,0);

		   t = new Thread("waiter0");
		   t->Fork((VoidFunctionPtr)waiter,0);



	}

////////////////////////////////////////////////////////////////////////////

	void test5(void)
	{
#define MANUAL_ORDER

		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 1
			#define COOKS 1
			#define CUSTOMERS 1

	DEBUG('q', "STARTING TEST 5 !!!!!!!!!!!!!!!!!!!!");


		customer_order_obj[0]->d6b =0;//rand () % 2;        // placing random order
		customer_order_obj[0]-> d3b= 0;//rand () % 2;        // placing random order
		customer_order_obj[0]-> sb= 0;//rand () % 2;         // placing random order
		customer_order_obj[0]->fries=0;// rand () % 2;      // placing random order
		customer_order_obj[0]->soda= 1;//rand () % 2;       // placing random order

		customer_order_obj[0]-> eat_pref= TO_GO;

		 Thread *t;
		 t = new Thread("customer0");
		   t->Fork((VoidFunctionPtr)customer,0);
		 t = new Thread("ordertaker0");
		   t->Fork((VoidFunctionPtr)ordertaker,0);
		}

////////////////////////////////////////////////////////////////////////////

	void test6(void)
	{

#define MANUAL_ORDER

		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 1
			#define COOKS 1
			#define CUSTOMERS 5


	DEBUG('q', "STARTING TEST 6 !!!!!!!!!!!!!!!!!!!!");
		for(int l=0;l<5;l++)
		{
			customer_order_obj[l]->d6b =rand () % 2;        // placing random order
			customer_order_obj[l]-> d3b= rand () % 2;        // placing random order
			customer_order_obj[l]-> sb= rand () % 2;         // placing random order
			customer_order_obj[l]-> fries= rand () % 2;      // placing random order
			customer_order_obj[l]-> soda= rand () % 2;       // placing random order
		 int w= rand () % 2;                         // placing random order
		 if (w==0)
		  {
			 customer_order_obj[l]-> eat_pref= TO_GO;
		  }
		 else
		  {
			 customer_order_obj[l]-> eat_pref= GET_IN;
		  }
		}
		 available_items_obj->d3b=100;
		 available_items_obj->d6b=100;
		 available_items_obj->sb=100;
		 available_items_obj->fries=100;

		 Thread *t;
		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);

		 t = new Thread("customer0");
		   t->Fork((VoidFunctionPtr)customer,0);
		 t = new Thread("customer1");
		   t->Fork((VoidFunctionPtr)customer,1);
		 t = new Thread("customer2");
		   t->Fork((VoidFunctionPtr)customer,2);
		 t = new Thread("customer3");
		   t->Fork((VoidFunctionPtr)customer,3);
		 t = new Thread("customer4");
		   t->Fork((VoidFunctionPtr)customer,4);

		 t = new Thread("ordertaker0");
		   t->Fork((VoidFunctionPtr)ordertaker,0);

		   t = new Thread("waiter0");
		   		   t->Fork((VoidFunctionPtr)waiter,0);


	}

////////////////////////////////////////////////////////////////////////////

	void test7(void)
	{

		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 1
			#define COOKS 1
			#define CUSTOMERS 1

#define AUTO_ORDER

	DEBUG('q', "STARTING TEST 7 !!!!!!!!!!!!!!!!!!!!");
		 inventory_obj->d3b=0;
		 inventory_obj->d6b=0;
		 inventory_obj->sb=0;
		 inventory_obj->fries=0;



		 totalmoney=0;

		 Thread *t;
		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);
		   t = new Thread("customer0");
		  		   t->Fork((VoidFunctionPtr)customer,0);
		  		 t = new Thread("ordertaker0");
		  		   t->Fork((VoidFunctionPtr)ordertaker,0);



	}

////////////////////////////////////////////////////////////////////////////

	void test8(void)
	{
#define AUTO_ORDER

		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 1
			#define COOKS 1
			#define CUSTOMERS 1

		DEBUG('q', "STARTING TEST 8 !!!!!!!!!!!!!!!!!!!!");
		available_items_obj->d3b=10;
		 available_items_obj->d6b=10;
		 available_items_obj->sb=10;
		 available_items_obj->fries=1;

		 Thread *t;
		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);

		 t = new Thread("cook1");
		   t->Fork((VoidFunctionPtr)cook,0);

	}

	////////////////////////////////////////////////////////////////////////////

	void test9(void)
	{

#define MANUAL_ORDER
		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 1
			#define COOKS 1
			#define CUSTOMERS 1


		DEBUG('q', "STARTING TEST 9 !!!!!!!!!!!!!!!!!!!!");

		customer_order_obj[0]->d6b =0;//rand () % 2;        // placing random order
		customer_order_obj[0]-> d3b= 0;//rand () % 2;        // placing random order
		customer_order_obj[0]->sb= 0;//rand () % 2;         // placing random order
		customer_order_obj[0]->fries=0;// rand () % 2;      // placing random order
		customer_order_obj[0]->soda= 1;//rand () % 2;       // placing random order
		 //int w= 1;//rand () % 2;         // placing random order
		customer_order_obj[0]->eat_pref= GET_IN;

		 for (int x=0;x<TABLES;x++)
		 {
		   table_stat[x]=TABLE_FREE;
		 }


		 Thread *t;
		 t = new Thread("customer0");
		   t->Fork((VoidFunctionPtr)customer,0);

		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);

		 t = new Thread("ordertaker0");
		   t->Fork((VoidFunctionPtr)ordertaker,0);

	}

	////////////////////////////////////////////////////////////////////////////


	void test10(void)
	{
#define MANUAL_ORDER
		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 1
			#define COOKS 1
			#define CUSTOMERS 1

		DEBUG('q', "STARTING TEST 10 !!!!!!!!!!!!!!!!!!!!");

		customer_order_obj[0]-> d6b = rand () % 2;        // placing random order
		customer_order_obj[0]->d3b= rand () % 2;        // placing random order
		customer_order_obj[0]->sb= rand () % 2;         // placing random order
		customer_order_obj[0]->fries= rand () % 2;      // placing random order
		customer_order_obj[0]->soda= rand () % 2;       // placing random order

		customer_order_obj[0]->eat_pref= GET_IN;

		 Thread *t;
		 t = new Thread("customer0");
		   t->Fork((VoidFunctionPtr)customer,0);

		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);

		 t = new Thread("ordertaker0");
		   t->Fork((VoidFunctionPtr)ordertaker,0);


	}



	void test11(void)
	{

		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 1
			#define COOKS 1
			#define CUSTOMERS 1

		#define MANUAL_ORDER


		DEBUG('q', "STARTING TEST 11 !!!!!!!!!!!!!!!!!!!!");

		customer_order_obj[0]->  d6b = rand () % 2;        // placing random order
		customer_order_obj[0]->  d3b= rand () % 2;        // placing random order
		customer_order_obj[0]-> sb= rand () % 2;         // placing random order
		customer_order_obj[0]-> fries= rand () % 2;      // placing random order
		customer_order_obj[0]->  soda= rand () % 2;       // placing random order
		customer_order_obj[0]->  eat_pref= TO_GO;

		 available_items_obj->d3b=20;
		 available_items_obj->d6b=20;
		 available_items_obj->sb=20;
		 available_items_obj->fries=20;

		  Thread *t;
		 t = new Thread("customer0");
		   t->Fork((VoidFunctionPtr)customer,0);

		 t = new Thread("ordertaker0");
		   t->Fork((VoidFunctionPtr)ordertaker,0);



	}



	void test12(void)
	{

	#define AUTO_ORDER
		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 1
			#define COOKS 1
			#define CUSTOMERS 1

		DEBUG('q', "STARTING TEST 12 !!!!!!!!!!!!!!!!!!!!");

		 pending_order_obj->d3b=20;
		 pending_order_obj->d6b=20;
		 pending_order_obj->sb=20;
		 pending_order_obj->fries=20;

		 inventory_obj->d3b=50;
		 inventory_obj->d6b=50;
		 inventory_obj->sb=50;
		 inventory_obj->fries=50;

		 available_items_obj->d3b=0;
		 available_items_obj->d6b=0;
		 available_items_obj->sb=0;
		 available_items_obj->fries=0;

		   for(int o=0; o<(COOKS + new_cooks); o++)
		   {
			if(cook_stat[o]==COOK_FREE);
		   }

		 Thread *t;
		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);





	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void test13(void)
	{
#define MANUAL_ORDER
		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 1
			#define COOKS 1
			#define CUSTOMERS 1

		DEBUG('q', "STARTING TEST 13 !!!!!!!!!!!!!!!!!!!!");

		customer_order_obj[0]->d6b =rand () % 2;        // placing random order
		customer_order_obj[0]-> d3b= rand () % 2;        // placing random order
		customer_order_obj[0]-> sb= rand () % 2;         // placing random order
		customer_order_obj[0]-> fries= rand () % 2;      // placing random order
		customer_order_obj[0]-> soda= rand () % 2;       // placing random order
		 int w= 0;        // placing random order
		  if (w==0)
		  {
			  customer_order_obj[0]-> eat_pref= TO_GO;
		  }
		 else
		  {
			 customer_order_obj[0]->eat_pref= GET_IN;
		  }


		 pending_order_obj->d3b=0;
		 pending_order_obj->d6b=0;
		 pending_order_obj->sb=0;
		 pending_order_obj->fries=0;

		 inventory_obj->d3b=50;
		 inventory_obj->d6b=50;
		 inventory_obj->sb=50;
		 inventory_obj->fries=50;

		 available_items_obj->d3b=0;
		 available_items_obj->d6b=0;
		 available_items_obj->sb=0;
		 available_items_obj->fries=0;

		 Thread *t;
		 t = new Thread("customer0");
		   t->Fork((VoidFunctionPtr)customer,0);

		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);

		 t = new Thread("ordertaker0");
		   t->Fork((VoidFunctionPtr)ordertaker,0);

		 t = new Thread("cook1");
		   t->Fork((VoidFunctionPtr)cook,0);



	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void test14(void)
	{
#define MANUAL_ORDER
		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 1
			#define COOKS 1
			#define CUSTOMERS 1

		DEBUG('q', "STARTING TEST 14 !!!!!!!!!!!!!!!!!!!!");


		customer_order_obj[0]->d6b = rand () % 2;        // placing random order
		customer_order_obj[0]->d3b= rand () % 2;        // placing random order
		customer_order_obj[0]->sb= rand () % 2;         // placing random order
		customer_order_obj[0]-> fries= rand () % 2;      // placing random order
		customer_order_obj[0]-> soda= rand () % 2;       // placing random order
		 int w= 1;//rand () % 2;         // placing random order
		 if (w==0)
		  {
			 customer_order_obj[0]->  eat_pref= TO_GO;
		  }
		 else
		  {
			 customer_order_obj[0]-> eat_pref= GET_IN;
		  }

		 available_items_obj->d3b=20;
		 available_items_obj->d6b=20;
		 available_items_obj->sb=20;
		 available_items_obj->fries=20;

		 Thread *t;
		 t = new Thread("customer0");
		   t->Fork((VoidFunctionPtr)customer,0);



		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);

		 t = new Thread("waiter0");
		   t->Fork((VoidFunctionPtr)waiter,0);


			 t = new Thread("ordertaker0");
			   t->Fork((VoidFunctionPtr)ordertaker,0);

	}

	//////////////////////////////////////////////////////////////////////////////////////////////////

	void test15(void)
	{

#define AUTO_ORDER
		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 1
			#define ORDERTAKERS 2
			#define COOKS 1
			#define CUSTOMERS 1

		DEBUG('q', "STARTING TEST 15 !!!!!!!!!!!!!!!!!!!!");



		 Thread *t;
		  for(int f=0;f<9;f++)
			    {
			    	char* name2;
			    	name2=	(char*) "customer";

			    	t = new Thread(name2);
			    	t->Fork((VoidFunctionPtr)customer,f);
			    }


		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);

		 t = new Thread("ordertaker0");
		   t->Fork((VoidFunctionPtr)ordertaker,0);

		   t = new Thread("ordertaker1");
		   		   t->Fork((VoidFunctionPtr)ordertaker,1);

		 t = new Thread("waiter0");
		   t->Fork((VoidFunctionPtr)waiter,0);

		 t = new Thread("cook0");
		   t->Fork((VoidFunctionPtr)cook,0);


	}

//////////////////////////////////////////////////////////////////////////////////////////////////



void full_simulation(void)
	{
	#define AUTO_ORDER
		int c,o,w,co;

		/***********************************************
			DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
			************************************************/
			#define WAITERS 5
			#define ORDERTAKERS 5
			#define COOKS 5
			#define CUSTOMERS 30


		DEBUG('q', "STARTING FULL_SIMULATION, WITH 30 CUSTOMERS, 5  WAITERS, 5 ORDERTAKERS, 5 COOKS,  !!!!!!!!!!!!!!!!!!!!");

		 Thread *t;
		  for(int f=0;f<30;f++)
			    {
			    	char* name2;

			    	name2=	(char*) "customer";



			    	t = new Thread(name2);
			    	t->Fork((VoidFunctionPtr)customer,f);
			    }

		  for(int f=0;f<5;f++)
		  			    {
			  	  	  	  	  	char* name2;

			  			    	name2=	(char*) "ordertaker";

		  			    	t = new Thread(name2);
		  			    	t->Fork((VoidFunctionPtr)ordertaker,f);
		  			    }

		  for(int f=0;f<5;f++)
		  		  			    {
		  			  	  	  	  	  	char* name2;

		  			  			    	name2=	(char*) "waiter";

		  		  			    	t = new Thread(name2);
		  		  			    	t->Fork((VoidFunctionPtr)waiter,f);
		  		  			    }

		  for(int f=0;f<5;f++)
		  		  			    {
		  			  	  	  	  	  	char* name2;

		  			  			    	name2=	(char*) "cook";

		  		  			    	t = new Thread(name2);
		  		  			    	t->Fork((VoidFunctionPtr)cook,f);
		  		  			    }

		 t = new Thread("manager");
		   t->Fork((VoidFunctionPtr)manager,0);



	}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ordertaker(int my_otnumber)
{


while(1)
	{

	/***********************************************
			  CHECKING WHEN TO FINISH THE ORDER TAKER
		 ********************************************/
		customer_gonelock->Acquire();

		if(customers_left==CUSTOMERS)
		{
			DEBUG('t',"endinggggggggggggggggg OTTTTTTTTTTTT!!!!!\n");
			customer_gonelock->Release();
			currentThread->Finish();
		}
		else
		{
			customer_gonelock->Release();
		}


	/*******************************************
	 interacting with customers in line for order
	 *********************************************/
	customerlinelock->Acquire();

	while(linelength>0)

	{
		if(is_mang_takingorder1==NO)
		{
	customerlineCV->Signal(customerlinelock) ;
	linelength--;
	ordertakerstatus[my_otnumber]=WAIT;


	//Acquires talk lock so that customer and order taker can talk mutually exclusively
	customerordertakerlock[my_otnumber]->Acquire();
	customerlinelock->Release();

	customerordertakerCV[my_otnumber]->Wait(customerordertakerlock[my_otnumber]);			// waiting for order to be placed


	/******************************************
	 * CALCULATING MONEY
	 *******************************************/

	money_received[my_otnumber]= (d6b[my_otnumber]*6) + (d3b[my_otnumber]*3) + (sb[my_otnumber]*2) + (fries[my_otnumber]*1)
									+ (soda[my_otnumber]*1);
	totalmoneylock->Acquire();
	totalmoney= totalmoney + money_received[my_otnumber];
	totalmoneylock->Release();

	   DEBUG('q',"OrderTaker %d is taking the order of Customer %d\n",my_otnumber,order_book_details[my_otnumber]->customer_no);

	DEBUG('q',"\nCALCULATING MONEY and giving bill\n!");

	customerordertakerCV[my_otnumber]->Signal(customerordertakerlock[my_otnumber]);
	customerordertakerCV[my_otnumber]->Wait(customerordertakerlock[my_otnumber]);								// waiting to get money from customer

	DEBUG('t',"\nOT checking for order details!!!!!!!!!!!!!!!!!!!!!!");


	if ((soda[my_otnumber]!=0)&&(fries[my_otnumber]==0)&&(d6b[my_otnumber]==0)&&(d3b[my_otnumber]==0)&&(sb[my_otnumber]==0))
	{

		ordermantainlock->Acquire();

		DEBUG('q',"OrderTaker %d gives soda to Customer %d\n",my_otnumber,order_book_details[my_otnumber]->customer_no);

		order_book_details[index_orderbook]->order_stat= FOOD_GIVEN;
		ordermantainlock->Release();
		status[my_otnumber]= FINISH;
		customerordertakerCV[my_otnumber]->Signal(customerordertakerlock[my_otnumber]);

		customerordertakerlock[my_otnumber]->Release();
	}


	else if(eat_pref[my_otnumber]==TO_GO)
	{
		total_foodlock->Acquire();

		if(( available_items_obj->d3b-d3b[my_otnumber]>=0) && ( available_items_obj->d6b-d6b[my_otnumber]>=0)
				&& ( available_items_obj->sb-sb[my_otnumber]>=0) && ( available_items_obj->fries-fries[my_otnumber]>=0))	// calculate if food is ready
		{

			// decrease from inventory with lock


			available_items_obj->d3b-= d3b[my_otnumber];
			available_items_obj->d6b-= d6b[my_otnumber];
			available_items_obj->sb-= sb[my_otnumber];
			available_items_obj->fries-= fries[my_otnumber];

			total_foodlock->Release();


			DEBUG('t',"\nto go ready food given to the customer!!!!!!!!!!!!!!!");


			ordermantainlock->Acquire();
			order_book_details[index_orderbook]->order_stat= FOOD_GIVEN;

			DEBUG('t',"OrderTaker %d gives food to Customer %d\n",my_otnumber,order_book_details[my_otnumber]->customer_no);

			ordermantainlock->Release();

			status[my_otnumber]=FINISH;


			customerordertakerCV[my_otnumber]->Signal(customerordertakerlock[my_otnumber]);

			customerordertakerlock[my_otnumber]->Release();

		}


	else
	{
		total_foodlock->Release();

		status[my_otnumber]=PENDING;
		customerordertakerCV[my_otnumber]->Signal(customerordertakerlock[my_otnumber]);


		ordermantainlock->Acquire();	// acquring lock to place order

		index_orderbook++;
		token_number[my_otnumber]=index_orderbook;
		customerordertakerlock[my_otnumber]->Release();

	     DEBUG('q',"OrderTaker %d gives token number %d to Customer %d\n",my_otnumber,index_orderbook,order_book_details[my_otnumber]->customer_no);

		order_book_details[index_orderbook]->d3b=d3b[my_otnumber];	// copying the order from order takers data structures
		order_book_details[index_orderbook]->d6b=d6b[my_otnumber];	// to the order book
		order_book_details[index_orderbook]->sb=sb[my_otnumber];                      //
		order_book_details[index_orderbook]->fries=fries[my_otnumber];                //   ----- do -------
		order_book_details[index_orderbook]->soda=soda[my_otnumber];                  //
		order_book_details[index_orderbook]->eat_pref=eat_pref[my_otnumber];	//
		order_book_details[index_orderbook]->order_stat= FOOD_PENDING;	//	----do-------
		order_book_details[index_orderbook]->customer_seating_stat= GOT_SEAT;
		order_book_details[index_orderbook]->customer_no= ot_customerno[my_otnumber];

		ordermantainlock->Release();



	}

}

else if (eat_pref[my_otnumber] == GET_IN)
{


	status[my_otnumber]=PENDING;
	DEBUG('t',"\nOT gives pending status to customer");
	customerordertakerCV[my_otnumber]->Signal(customerordertakerlock[my_otnumber]);


	//customerordertakerCV[i]->Wait(customerordertakerlock[i]);
	///////////////////////////////////////////////////////////////////////////////////////////WAIT ADDED ABOVE THIS LINE
	ordermantainlock->Acquire();	// acquring lock to place order

	index_orderbook++;

    DEBUG('q',"OrderTaker %d gives token number %d to Customer %d\n",my_otnumber,index_orderbook,order_book_details[my_otnumber]->customer_no);
	token_number[my_otnumber]=index_orderbook;
	customerordertakerlock[my_otnumber]->Release();


	order_book_details[index_orderbook]->d3b=d3b[my_otnumber];	// copying the order from order takers data structures
	order_book_details[index_orderbook]->d6b=d6b[my_otnumber];	// to the order book
	order_book_details[index_orderbook]->sb=sb[my_otnumber];                      //
	order_book_details[index_orderbook]->fries=fries[my_otnumber];                //   ----- do -------
	order_book_details[index_orderbook]->soda=soda[my_otnumber];                  //
	order_book_details[index_orderbook]->eat_pref=eat_pref[my_otnumber];	//
	order_book_details[index_orderbook]->order_stat= FOOD_PENDING;	//	----do-------
	order_book_details[index_orderbook]->customer_seating_stat= NO_SEAT	;
	order_book_details[index_orderbook]->customer_no= ot_customerno[my_otnumber];

	DEBUG ('t',"\norder book details are:%d%d%d%d",order_book_details[index_orderbook]->d3b,order_book_details[index_orderbook]->d6b,
					order_book_details[index_orderbook]->sb,order_book_details[index_orderbook]->fries);

	ordermantainlock->Release();


}
	customerlinelock->Acquire();
	}

		else
		{
			break;
		}


	}




	/*****************************************
		 	 checking if any food to be bagged
		 *****************************************/






		 ordermantainlock->Acquire();


		 ordertakerstatus[my_otnumber]=BUSY;
		 customerlinelock->Release();

		 for(int m=0;m<=index_orderbook;m++)
		 {
			 if(order_book_details[m]->order_stat == FOOD_PENDING )
			 {

				 total_foodlock->Acquire();

				 if((available_items_obj->d6b - order_book_details[m]->d6b)>0 &&
						 (available_items_obj->d3b - order_book_details[m]->d3b)>0 &&
						 (available_items_obj->sb - order_book_details[m]->sb)>0 &&
						 (available_items_obj->fries - order_book_details[m]->fries)>0)
				 {
					 available_items_obj->d6b = available_items_obj->d6b - order_book_details[m]->d6b;
					 available_items_obj->d3b = available_items_obj->d3b - order_book_details[m]->d3b;
					 available_items_obj->sb = available_items_obj->sb - order_book_details[m]->sb;
					 available_items_obj->fries = available_items_obj->fries - order_book_details[m]->fries;

					 order_book_details[m]->order_stat= FOOD_PACKED;
					 DEBUG('t', "FOOD packed by order taker !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
					 DEBUG('q',"OrderTaker %d packed the food of Customer %d\n",my_otnumber, order_book_details[m]->customer_no);
				}
				 total_foodlock->Release();
			 }

		 }
		 ordermantainlock->Release();





	/*******************************************************************************
		 	 DELIVER FOOD WHEN FOOD PACKED and
		 	1) TELL WAITER WHEN EAT_IN and CUSTOMER HAS A TABLE or
		 	2) TO_GO
		********************************************************************************/

	ordermantainlock->Acquire();

		for(int l=0;l<=index_orderbook;l++)
		{
			if(order_book_details[l]->order_stat==FOOD_PACKED)
			{
				if(order_book_details[l]->eat_pref==TO_GO)
				{
					togo_waitinglock->Acquire();

					togo_food_ready_token=l;

					togo_waitingCV->Broadcast(togo_waitinglock);
					togo_waitinglock->Release();

				}

				else					// if eat in
				{
					if(order_book_details[l]->customer_seating_stat==GOT_SEAT)
					{
						for(int g=0;g<WAITERS;g++)
						{
							if(waiter_stat[g]==WAITER_FREE)
							{
								   DEBUG('q',"OrderTaker %d gives token number %d to waiter %d for Customer %d\n",my_otnumber,index_orderbook,g,order_book_details[l]->customer_no);
								DEBUG('t',"\nOT giving food to waiter !!!!!!!!!!!!!!!");
								waiterlock[g]->Acquire();
								waiter_stat[g]=WAITER_BUSY;
								waiter_tokennumber[g]=l;

								waiterCV[g]->Signal(waiterlock[g]);
								waiterlock[g]->Release();
							}
							break;
						}
					}
				}
			}
		}

		ordermantainlock->Release();


		/*************************************
		 * kill order taker when no work left
		 **************************************/
		{
			ordertakerstatus[my_otnumber]=FREE;
				DEBUG('t',"\nCOMMENT: status set to free");
		//		currentThread->Finish();

		}

currentThread->Yield();

currentThread->Yield();

	}		// end while(1)



}  // end ot function




//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void customer( int mynumber)
{

int myordertaker;
customerlinelock->Acquire();
linelength++;
customerlineCV->Wait(customerlinelock);

if( is_mang_takingorder1 == NO)

{

	for(int m=0;m<ORDERTAKERS;m++)
	{
		if(ordertakerstatus[m]==WAIT)
		{
			DEBUG('t',"\nCOMMENT: customer %d looking for my order taker", mynumber);
			myordertaker=m;
			ordertakerstatus[m]=BUSY;
			break;
		}
	}
	customerlinelock->Release();
	customerordertakerlock[myordertaker]->Acquire();

	DEBUG('q',"Customer %d is giving order to OrderTaker %d\n",mynumber,myordertaker);

	DEBUG('t',"\nCOMMENT: customer %d inside place order", mynumber);

	ot_customerno[myordertaker]= mynumber;

#ifdef AUTO_ORDER

	d6b[myordertaker]= rand () % 2;	// placing random order
	d3b[myordertaker]= rand () % 2;	//  placing random order
	sb[myordertaker]= rand () % 2;	// placing random order
	fries[myordertaker]= rand () % 2;	// placing random order
	soda[myordertaker]= rand () % 2;	//	placing random order
	int w= rand () % 2;				// placing random order

	  if (w==0)
	  {
		  eat_pref[myordertaker]= TO_GO;
	  }
	  else
	  {
		  eat_pref[myordertaker]= GET_IN;

	  }
#endif

#ifdef MANUAL_ORDER

	    d6b[myordertaker]= customer_order_obj[mynumber]->d6b;	// placing random order
	  	d3b[myordertaker]= customer_order_obj[mynumber]->d3b;	//  placing random order
	  	sb[myordertaker]= customer_order_obj[mynumber]->sb;	// placing random order
	  	fries[myordertaker]= customer_order_obj[mynumber]->fries;	// placing random order
	  	soda[myordertaker]= customer_order_obj[mynumber]->soda;	//	placing random order
	  	eat_pref[myordertaker]= customer_order_obj[mynumber]->eat_pref;

#endif


	  	/************************************************************************************
	  								PRINTING ORDER
	  	  ************************************************************************************/

	  	  if (d6b[myordertaker]==0)
	  	  {
	  	   DEBUG('q',"Customer %d is not Ordering 6-dollar burger\n",mynumber);
	  	  }
	  	  else
	  	  {
	  	   DEBUG('q',"Customer %d is Ordering %d 6-dollar burger\n",mynumber,d6b[myordertaker]);
	  	  }

	  	  if (d3b[myordertaker]==0)
	  	  {
	  	   DEBUG('q',"Customer %d is not Ordering 3-dollar burger\n",mynumber);
	  	  }
	  	  else
	  	  {
	  	   DEBUG('q',"Customer %d is Ordering %d 3-dollar burger\n",mynumber,d3b[myordertaker]);
	  	  }

	  	  if (sb[myordertaker]==0)
	  	  {
	  	   DEBUG('q',"Customer %d is not Ordering veggie burger\n",mynumber);
	  	  }
	  	  else
	  	  {
	  	   DEBUG('q',"Customer %d is Ordering %d veggie burger\n",mynumber,sb[myordertaker]);
	  	  }

	  	  if (fries[myordertaker]==0)
	  	  {
	  	   DEBUG('q',"Customer %d is not Ordering french fries\n",mynumber);
	  	  }
	  	  else
	  	  {
	  	   DEBUG('q',"Customer %d is Ordering %d pack french fries\n",mynumber,fries[myordertaker]);
	  	  }

	  	  if (soda[myordertaker]==0)
	  	  {
	  	   DEBUG('q',"Customer %d is not Ordering soda\n",mynumber);
	  	  }
	  	  else
	  	  {
	  	   DEBUG('q',"Customer %d is Ordering %d soda\n",mynumber,soda[myordertaker]);
	  	  }

	  	  if (eat_pref[myordertaker]==TO_GO)
	  	  {
	  	   DEBUG('q',"Customer %d chooses to TO GO the food\n",mynumber);
	  	  }
	  	  else
	  	  {
	  	   DEBUG('q',"Customer %d  chooses to EAT IN the food\n",mynumber);
	  	  }
	  	  /************************************************************************************
	  								END OF PRINTING ORDER
	  	  ************************************************************************************/

	  DEBUG('t', "/n THE customer %d ORDER IS:::::::::::::::::::::::::::/n 6 $ BURGER:%d,3 $ BURGER: %d, /n simple BURGER:%d, /n  FRIES:%d, /n  soda%d "
			  "eat_pref:%d", mynumber, d6b[myordertaker],d3b[myordertaker],sb[myordertaker], fries[myordertaker], soda[myordertaker],eat_pref[myordertaker]);


	customerordertakerCV[myordertaker]->Signal(customerordertakerlock[myordertaker]);
	customerordertakerCV[myordertaker]->Wait(customerordertakerlock[myordertaker]);

	// give money

	DEBUG('q',"\nCOMMENT:customer %d giving money !!!!!!!!!!!!", mynumber);

	customerordertakerCV[myordertaker]->Signal(customerordertakerlock[myordertaker]);
	customerordertakerCV[myordertaker]->Wait(customerordertakerlock[myordertaker]);
	DEBUG('t',"\n customer %d reached 111!!!!!!!!!!!!!!!",mynumber);

	if (eat_pref[myordertaker] == TO_GO)
	{
		if(status[myordertaker]==FINISH)
		{

		DEBUG('t',"\ncustomer %d GOT TO_GO READY FOOD ......byeeeeeeeeeee", mynumber);
		customer_gonelock->Acquire();
		customers_left++;
	    DEBUG('q',"Customer %d receives food from the OrderTaker %d\n",mynumber,myordertaker);

		customer_gonelock->Release();

		customerordertakerlock[myordertaker]->Release();
		currentThread->Finish();	// customer leaves the restaurant


		}
		else
		{
		customer_token_number[mynumber]	= token_number[myordertaker];
		DEBUG('q',"\n customer %dTAKING TOKEN NUMBER FROM OT!!!!!!!!!!",mynumber);
		DEBUG('t',"Customer %d receives token from the OrderTaker %d\n",mynumber,myordertaker);

		//customerordertakerCV[myordertaker]->Signal(customerordertakerlock[myordertaker]);
		customerordertakerlock[myordertaker]->Release();

		togo_waitinglock->Acquire();
		DEBUG('t',"\n customer %d waiting to be broadcasted !!!!!!!!!!!!!!!", mynumber);
		togo_waitingCV->Wait(togo_waitinglock);				// wait for token no to be broadcasted


		while(1)
		{

		if(customer_token_number[mynumber]==togo_food_ready_token)
		{

			togo_waitinglock->Release();
			ordermantainlock->Acquire();
			order_book_details[customer_token_number[mynumber]]->order_stat= FOOD_GIVEN;
			ordermantainlock->Release();


			DEBUG('q',"\n customer %d GOT TO_GO FOOD ", mynumber);
			customer_gonelock->Acquire();

			customers_left++;

			customer_gonelock->Release();
			currentThread->Finish();
		}

			togo_waitingCV->Wait(togo_waitinglock);
		}




		}
	}

	else if(eat_pref[myordertaker] == GET_IN)
	{

		if(status[myordertaker]==PENDING)
		{

			customer_token_number[mynumber]	= token_number[myordertaker];
				DEBUG('t',"\ncustomer %d TAKING TOKEN NUMBER FROM OT!!!!!!!!!!", mynumber);
				DEBUG('q',"Customer %d receives token from the OrderTaker %d\n",mynumber,myordertaker);

	/************************************************
	 looking for a table to sit
	 ************************************************/

				DEBUG('q',"\ncustomer %d looking for free table !!!!!!!!!!!!", mynumber);

				tablelinelock->Acquire();


				customerordertakerlock[myordertaker]->Release();

				tableline++;

				tablelineCV->Wait(tablelinelock);

				manager_customer_token= customer_token_number[mynumber];


				tablelinelock->Release();

				manager_customer_tablelock->Acquire();

				manager_customer_tableCV->Signal(manager_customer_tablelock);
				manager_customer_tableCV->Wait(manager_customer_tablelock);

				manager_customer_tablelock->Release();

				ordermantainlock->Acquire();

				int my_table= order_book_details[customer_token_number[mynumber]]->table_number;

				ordermantainlock->Release();

				tablelock[my_table]->Acquire();

				tableCV[my_table]->Wait(tablelock[my_table]);

				currentThread->Yield();						// time given to eat food

				DEBUG('q',"\n customer %d had my food ....byeeeeeee!!! !!!!!!!!!!!", mynumber);

				customer_gonelock->Acquire();
				customers_left++;

				customer_gonelock->Release();

				table_stat[my_table]= TABLE_FREE;

				tablelock[my_table]->Release();

				currentThread->Finish();					// only after food is delivered

		}

		else									// when only soda and get in
		{

					customer_token_number[mynumber]	= token_number[myordertaker];
						DEBUG('q',"\ncustomer %d TAKING TOKEN NUMBER FROM OT!!!!!!!!!!",mynumber);

					    DEBUG('q',"Customer %d receives soda from the OrderTaker %d\n",mynumber,myordertaker);

				/************************************************
						 looking for a table to sit
				 ************************************************/

						DEBUG('q',"\n customer %d looking for free table !!!!!!!!!!!!",mynumber);

						tablelinelock->Acquire();

						customerordertakerlock[myordertaker]->Release();

						tableline++;

						tablelineCV->Wait(tablelinelock);

						manager_customer_token= customer_token_number[mynumber];


						tablelinelock->Release();

						manager_customer_tablelock->Acquire();

						manager_customer_tableCV->Signal(manager_customer_tablelock);
						manager_customer_tableCV->Wait(manager_customer_tablelock);

						manager_customer_tablelock->Release();

						ordermantainlock->Acquire();

						int my_table= order_book_details[customer_token_number[mynumber]]->table_number;

						ordermantainlock->Release();

						tablelock[my_table]->Acquire();

						tableCV[my_table]->Wait(tablelock[my_table]);

						//*********************************************
							//waiter interaction starts from here ....
							//***********************************************


									currentThread->Yield();						// time given to eat food

									DEBUG('q',"\nhad my soda ....byeeeeeee!!! !!!!!!!!!!!");

									customer_gonelock->Acquire();
									customers_left++;

									customer_gonelock->Release();

									table_stat[my_table]= TABLE_FREE;

									tablelock[my_table]->Release();

									currentThread->Finish();
		}



		}
	}

/**************************************************
CUSTOMER GIVING ORDER TO THE MANAGER
**************************************************/

	else
	{
		is_mang_takingorder1=NO;

		customerlinelock->Release();
		manager_cust_orderlock->Acquire();


		DEBUG('t',"\nCOMMENT: customer %d inside place order",mynumber);
		DEBUG('q',"Manager is taking order of customer %d\n",mynumber);

		manager_customerno=mynumber;

#ifdef AUTO_ORDER

		manager_orderbook_obj->token_number= mynumber;
		manager_orderbook_obj->d6b= rand () % 2;	// placing random order
		manager_orderbook_obj->d3b= rand () % 2;	//  placing random order
		manager_orderbook_obj->sb= rand () % 2;	// placing random order
		manager_orderbook_obj->fries= rand () % 2;	// placing random order
		manager_orderbook_obj->soda= rand () % 2;	//	placing random order
	int w= rand () % 2;				// placing random order

	  if (w==0)
	  {
		  manager_orderbook_obj->eat_pref= TO_GO;
	  }
	  else
	  {
		  manager_orderbook_obj-> eat_pref= GET_IN;

	  }
#endif

#ifdef MANUAL_ORDER

	  manager_orderbook_obj->token_number= mynumber;
	  manager_orderbook_obj->d6b= customer_order_obj[mynumber]->d6b;	// placing random order
	  manager_orderbook_obj->d3b= customer_order_obj[mynumber]->d3b;	//  placing random order
	  manager_orderbook_obj->sb= customer_order_obj[mynumber]->sb;	// placing random order
	  manager_orderbook_obj->fries= customer_order_obj[mynumber]->fries;	// placing random order
	  manager_orderbook_obj->soda= customer_order_obj[mynumber]->soda;	//	placing random order
	  manager_orderbook_obj->eat_pref= customer_order_obj[mynumber]->eat_pref;

#endif

	  /************************************************************************************
								PRINTING ORDER
	  ************************************************************************************/

	  if (manager_orderbook_obj->d6b==0)
	  {
	   DEBUG('q',"Customer %d is not Ordering 6-dollar burger\n",mynumber);
	  }
	  else
	  {
	   DEBUG('q',"Customer %d is Ordering %d 6-dollar burger\n",mynumber,manager_orderbook_obj->d6b);
	  }

	  if (manager_orderbook_obj->d3b==0)
	  {
	   DEBUG('q',"Customer %d is not Ordering 3-dollar burger\n",mynumber);
	  }
	  else
	  {
	   DEBUG('q',"Customer %d is Ordering %d 3-dollar burger\n",mynumber,manager_orderbook_obj->d3b);
	  }

	  if (manager_orderbook_obj->sb==0)
	  {
	   DEBUG('q',"Customer %d is not Ordering veggie burger\n",mynumber);
	  }
	  else
	  {
	   DEBUG('q',"Customer %d is Ordering %d veggie burger\n",mynumber,manager_orderbook_obj->sb);
	  }

	  if (manager_orderbook_obj->fries==0)
	  {
	   DEBUG('q',"Customer %d is not Ordering french fries\n",mynumber);
	  }
	  else
	  {
	   DEBUG('q',"Customer %d is Ordering %d pack french fries\n",mynumber,manager_orderbook_obj->fries);
	  }

	  if (manager_orderbook_obj->soda==0)
	  {
	   DEBUG('q',"Customer %d is not Ordering soda\n",mynumber);
	  }
	  else
	  {
	   DEBUG('q',"Customer %d is Ordering %d soda\n",mynumber,manager_orderbook_obj->soda);
	  }

	  if (manager_orderbook_obj->eat_pref==TO_GO)
	  {
	   DEBUG('q',"Customer %d chooses to TO GO the food\n",mynumber);
	  }
	  else
	  {
	   DEBUG('q',"Customer %d  chooses to EAT IN the food\n",mynumber);
	  }
	  /************************************************************************************
								END OF PRINTING ORDER
	  ************************************************************************************/



	  	DEBUG('t', "/n customer %d THE ORDER to maanger IS:::::::::::::::::::::::::::/n 6 $ BURGER:%d,3 $ BURGER: %d, /n simple BURGER:%d, /n  FRIES:%d, /n  soda%d "
		                 "eat_pref:%d",mynumber,manager_orderbook_obj->d6b,manager_orderbook_obj->d3b,manager_orderbook_obj->sb,manager_orderbook_obj-> fries,manager_orderbook_obj-> soda,manager_orderbook_obj->eat_pref);

	  	DEBUG('q',"\ncustomer %d now waiting for the bill",mynumber);
		manager_cust_orderCV->Signal(manager_cust_orderlock);
		manager_cust_orderCV->Wait(manager_cust_orderlock);

		// give money

		DEBUG('q',"\nCOMMENT: customer %d giving money to manager !!!!!!!!!!!!!!", mynumber);

		manager_cust_orderCV->Signal(manager_cust_orderlock);
		manager_cust_orderCV->Wait(manager_cust_orderlock);

		DEBUG('t',"\n customer %d reached 111!!!!!!!!!!!!!!!", mynumber);

		if (manager_orderbook_obj->eat_pref == TO_GO)
		{
		       if(manager_orderbook_obj->mang_order_status_obj == ORDER_FINISH)
		       {
		    	   DEBUG('q', "GOT MY TO_GO FOOD BYEEEEEEEEE!!!!!!!!!!!!!!!!");
		    	   DEBUG('q',"Customer %d receives food from the Manager\n",mynumber);

		    	   customer_gonelock->Acquire();
		    	   			customers_left++;

		    	   	customer_gonelock->Release();

		       manager_cust_orderlock->Release();
		       currentThread->Finish();        // customer leaves the restaurant

		       }
		       else
		       {
		       customer_token_number[mynumber] =manager_orderbook_obj->token_number;
		       DEBUG('q',"\n customer %d TAKING TOKEN NUMBER FROM managerrrrrr!!!!!!!!!!", mynumber);
		       DEBUG('q',"Customer %d receives food from the Manager\n",mynumber);

		       //customerordertakerCV[myordertaker]->Signal(customerordertakerlock[myordertaker]);
		       manager_cust_orderlock->Release();

		       togo_waitinglock->Acquire();
		       togo_waitingCV->Wait(togo_waitinglock);

		       while(1)
		       		{

		       		if(customer_token_number[mynumber]==togo_food_ready_token)
		       		{

		       			togo_waitinglock->Release();
		       			ordermantainlock->Acquire();
		       			order_book_details[customer_token_number[mynumber]]->order_stat= FOOD_GIVEN;
		       			ordermantainlock->Release();

		       			DEBUG('q',"\n customer %d IGOT MY TO GO FOOD < NOW M LEAVING  ..............", mynumber);
		       			customer_gonelock->Acquire();
		       			customers_left++;

		       			customer_gonelock->Release();

		       			currentThread->Finish();
		       		}

		       			togo_waitingCV->Wait(togo_waitinglock);
		       		}



		       }
		}

		else if(manager_orderbook_obj->eat_pref == GET_IN)
		{
		       DEBUG('t',"\ncustomer %d before IF !!!!!!!!!!!!!!!",mynumber);
		       if(manager_orderbook_obj->mang_order_status_obj== ORDER_PENDING)
		       {
		            DEBUG('t',"\nafter if !!!!!!!!!!!!!!");
		            customer_token_number[mynumber]	= manager_orderbook_obj->token_number;
		            						DEBUG('q',"\n customer %d TAKING TOKEN NUMBER FROM managerr!!!!!!!!!!", mynumber);
		            						DEBUG('q',"Customer %d receives food from the Manager\n",mynumber);


		            				/************************************************
		            						 looking for a table to sit
		            				 ************************************************/

		            						DEBUG('q',"\n customer %d looking for free table !!!!!!!!!!!!", mynumber);

		            						tablelinelock->Acquire();

		            						 manager_cust_orderlock->Release();

		            						tableline++;

		            						tablelineCV->Wait(tablelinelock);

		            						manager_customer_token= customer_token_number[mynumber];


		            						tablelinelock->Release();

		            						manager_customer_tablelock->Acquire();

		            						manager_customer_tableCV->Signal(manager_customer_tablelock);
		            						manager_customer_tableCV->Wait(manager_customer_tablelock);

		            						manager_customer_tablelock->Release();

		            						ordermantainlock->Acquire();

		            						int my_table= order_book_details[customer_token_number[mynumber]]->table_number;

		            						ordermantainlock->Release();

		            						tablelock[my_table]->Acquire();

		            						tableCV[my_table]->Wait(tablelock[my_table]);

		            						//*********************************************
		            							//waiter interaction starts from here ....
		            							//***********************************************


		            									currentThread->Yield();						// time given to eat food

		            									DEBUG('q',"\n customer %d had my food ....byeeeeeee!!! !!!!!!!!!!!",mynumber);

		            									customer_gonelock->Acquire();
		            									customers_left++;

		            									customer_gonelock->Release();

		            									table_stat[my_table]= TABLE_FREE;

		            									tablelock[my_table]->Release();

		            									currentThread->Finish();
		       }

		       else                                                    // when only soda and get in
		       {

					customer_token_number[mynumber]	= manager_orderbook_obj->token_number;
						DEBUG('q',"\ncustomer %d TAKING TOKEN NUMBER FROM managerrrrrr!!!!!!!!!!", mynumber);
						DEBUG('q',"Customer %d receives soda from the Manager\n",mynumber);

				/************************************************
						 looking for a table to sit
				 ************************************************/

						DEBUG('q',"\ncustomer %d looking for free table !!!!!!!!!!!!", mynumber);

						tablelinelock->Acquire();

						 manager_cust_orderlock->Release();

						tableline++;

						tablelineCV->Wait(tablelinelock);

						manager_customer_token= customer_token_number[mynumber];


						tablelinelock->Release();

						manager_customer_tablelock->Acquire();

						manager_customer_tableCV->Signal(manager_customer_tablelock);
						manager_customer_tableCV->Wait(manager_customer_tablelock);

						manager_customer_tablelock->Release();

						ordermantainlock->Acquire();

						int my_table= order_book_details[customer_token_number[mynumber]]->table_number;

						ordermantainlock->Release();

						tablelock[my_table]->Acquire();



						//*********************************************
							//waiter interaction starts from here ....
							//***********************************************


									currentThread->Yield();						// time given to eat food

									DEBUG('q',"\n customer %d had my soda....byeeeeeee!!! !!!!!!!!!!!",mynumber);

									customer_gonelock->Acquire();
												customers_left++;

									customer_gonelock->Release();

									table_stat[my_table]= TABLE_FREE;

									tablelock[my_table]->Release();

									currentThread->Finish();


		       }



		}

	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void manager (void)

{
int count=50;
int cook_free;

while(1)

{
	DEBUG('q',"\nstarted manager!!!!!");

	/***********************************
		  CHECKING WHEN TO FINISH THE MANAGER
	 ************************************/
	customer_gonelock->Acquire();
	if(customers_left==CUSTOMERS)
	{
		DEBUG('q',"\nendingggggggggggggggggg manager!!!!!");

		customer_gonelock->Release();
		currentThread->Finish();
	}
	else
	{
		customer_gonelock->Release();
	}


/***********************************
	  CODE FOR MANAGING THE COOK
************************************/

	ordermantainlock->Acquire();

	for(int r=0;r<index_orderbook;r++)
	{

		if (order_book_details[r]->order_stat == FOOD_PENDING)
		{
			pending_order_obj->d6b = pending_order_obj->d6b + order_book_details[r]->d6b;
			pending_order_obj->d3b = pending_order_obj->d3b + order_book_details[r]->d3b;
			pending_order_obj->sb = pending_order_obj->sb + order_book_details[r]->sb;
			pending_order_obj->fries = pending_order_obj->fries + order_book_details[r]->fries;
		}
	}
	ordermantainlock->Release();

		//////////////////////////////////////////////////////////////// checking for d6b

	total_foodlock->Acquire();

		if(available_items_obj->d6b- pending_order_obj->d6b<5)
		{
			total_foodlock->Release();
			cook_free=0;
			for(int o=0; o<(COOKS + new_cooks); o++)
			{
				if(cook_stat[o]==COOK_FREE)
				{
					cook_free++;
					 DEBUG('q',"Manager informs Cook %d to cook 6-dollar burger\n",o);
					cook_stat[o]=D6B;
					manager_cooklock->Acquire();
					manager_cookCV->Signal(manager_cooklock);
					manager_cooklock->Release();
					break;
				}

			}
			if(cook_free==0)
							{
								// hire a new cook
				DEBUG('q',"Manager is hiring a new cook for 6-dollar burger\n");
								DEBUG('q',"\nHIRING A NEW COOK for d6b");
								char* name_cook=(char *) "Cook";
								new_cooks++;
								Thread *t= new Thread(name_cook);
								manager_cooklock->Acquire();
								cook_stat[COOKS+new_cooks]=D6B;
								t->Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
								manager_cooklock->Release();

							}
		}
		else
		{
			total_foodlock->Release();
		}

		////////////////////////////////////////////////////////////////// check for 3db :

		total_foodlock->Acquire();
		if((available_items_obj->d3b- pending_order_obj->d3b) < 5)
				{
					total_foodlock->Release();
					cook_free=0;
					for(int o=0; o<(COOKS + new_cooks); o++)
					{
						if(cook_stat[o]==COOK_FREE)
						{
							DEBUG('q',"Manager informs Cook %d to cook 3-dollar burger\n",o);
							cook_free++;
							cook_stat[o]=D3B;
							manager_cooklock->Acquire();
							manager_cookCV->Signal(manager_cooklock);
							manager_cooklock->Release();
							break;
						}

					}
					if(cook_free==0)
								{
									// hire a new cook
					     DEBUG('q',"Manager is hiring a new cook for 3-dollar burger\n");
										DEBUG('q',"\nHIRING A NEW COOK for d3b");
										char* name_cook=(char *) "Cook";
										new_cooks++;
										Thread *t= new Thread(name_cook);
										manager_cooklock->Acquire();
										cook_stat[COOKS+new_cooks]=D3B;
										t->Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
										manager_cooklock->Release();

									}
				}
		else
				{
					total_foodlock->Release();
				}

		////////////////////////////////////////////////////////////////// check for sb :

		total_foodlock->Acquire();
		if((available_items_obj->sb- pending_order_obj->sb) < 5)
				{
					total_foodlock->Release();
					cook_free=0;
					for(int o=0; o<(COOKS + new_cooks); o++)
					{
						if(cook_stat[o]==COOK_FREE)
						{
							DEBUG('q',"Manager informs Cook %d to cook veggie burger\n",o);
							cook_free++;
							cook_stat[o]=SB;
							manager_cooklock->Acquire();
							manager_cookCV->Signal(manager_cooklock);
							manager_cooklock->Release();
							break;
						}

					}
					if(cook_free==0)
									{
														// hire a new cook
						DEBUG('q',"Manager is hiring a new cook for veggie burger\n");
														DEBUG('q',"\nHIRING A NEW COOK for sb");
														char* name_cook=(char *) "Cook";
														new_cooks++;
														Thread *t= new Thread(name_cook);
														manager_cooklock->Acquire();
														cook_stat[COOKS+new_cooks]=SB;
														t->Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
														manager_cooklock->Release();
									}
				}
		else
				{
					total_foodlock->Release();
				}

		////////////////////////////////////////////////////////////////// check for fries :

		total_foodlock->Acquire();
		if( (available_items_obj->fries - pending_order_obj->fries) <5)
				{
					total_foodlock->Release();
					cook_free=0;
					for(int o=0; o<(COOKS + new_cooks); o++)
					{
						if(cook_stat[o]==COOK_FREE)
						{
							DEBUG('q',"Manager informs Cook %d to cook french fries\n",o);
							cook_free++;
							cook_stat[o]=FRIES;
							manager_cooklock->Acquire();
							manager_cookCV->Signal(manager_cooklock);
							manager_cooklock->Release();
							break;
						}

					}
					if(cook_free==0)
									{
												// hire a new cook
					    DEBUG('q',"Manager is hiring a new cook for french fries\n");
														DEBUG('q',"\nHIRING A NEW COOK for fries");
														char* name_cook=(char *) "Cook";
														new_cooks++;
														Thread *t= new Thread(name_cook);
														manager_cooklock->Acquire();
														cook_stat[COOKS+new_cooks]=FRIES;
														t->Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
														manager_cooklock->Release();
									}
				}
		else
				{
					total_foodlock->Release();
				}

		////////////////////////////////////////////////////////////// check if item is in surplus---D6B

		total_foodlock->Acquire();

		if( (available_items_obj->d6b - pending_order_obj->d6b) >20)
						{
							total_foodlock->Release();

							for(int n=0; n<(COOKS + new_cooks); n++)
							{
								if(cook_stat[n]==D6B)
								{
									 DEBUG('q',"COOK %d is gong on break\n",n);
									cook_stat[n]=COOK_FREE;

								}

							}
						}
		else
				{
					total_foodlock->Release();
				}

		////////////////////////////////////////////////////////////// check if item is in surplus---D3B

		total_foodlock->Acquire();

		if( (available_items_obj->d3b - pending_order_obj->d3b) >20)
						{
							total_foodlock->Release();

							for(int n=0; n<(COOKS + new_cooks); n++)
							{
								if(cook_stat[n]==D3B)
								{
									 DEBUG('q',"COOK %d is gong on break\n",n);

									cook_stat[n]=COOK_FREE;

								}

							}
						}
		else
				{
					total_foodlock->Release();
				}


		////////////////////////////////////////////////////////////// check if item is in surplus---SB

		total_foodlock->Acquire();

		if( (available_items_obj->sb - pending_order_obj->sb) >20)
						{
							total_foodlock->Release();

							for(int n=0; n<(COOKS + new_cooks); n++)
							{
								if(cook_stat[n]==SB)
								{
									 DEBUG('q',"COOK %d is gong on break\n",n);
									cook_stat[n]=COOK_FREE;

								}

							}
						}
		else
				{
					total_foodlock->Release();
				}


		////////////////////////////////////////////////////////////// check if item is in surplus---FRIES

		total_foodlock->Acquire();

		if( (available_items_obj->fries - pending_order_obj->fries) >20)
						{
							total_foodlock->Release();

							for(int n=0; n<(COOKS + new_cooks); n++)
							{
								if(cook_stat[n]==FRIES)
								{
									 DEBUG('q',"COOK %d is gong on break\n",n);
									cook_stat[n]=COOK_FREE;

								}

							}
						}
		else
				{
					total_foodlock->Release();
				}

currentThread->Yield();
//currentThread->Yield();


	/********************************************
	 * CODE FOR TABLE MANAGEMENT
	 ********************************************/

//currentThread->Yield();


	tablelinelock->Acquire();					//acquire get-in line lock to check length of line


	if(tableline>0)							//check if there are customers waiting for the tables
	{

		DEBUG('q',"\na customer waiting for a table!!!!!!!!!!");



		for (int x=0;x<TABLES;x++)							//check if there are any free tables available

	{
			tablelock[x]->Acquire();


			if(table_stat[x]==TABLE_FREE)
			{
				DEBUG('q',"\nfree tables available!!! !!!!!!!!!!!");
				 DEBUG('q',"Customer is informed by the Manager-the restaurant is not full\n");

				table_stat[x]==TABLE_OCCUPIED;
				tablelock[x]->Release();
				tableline--;


				tablelineCV->Signal(tablelinelock);			//signal the customer that a table has been assigned

				manager_customer_tablelock->Acquire();		// acquire lock for interaction with customer

				tablelinelock->Release();

				manager_customer_tableCV->Wait(manager_customer_tablelock);	//wait for customer's token number so that entry can be updated in the main order book



				ordermantainlock->Acquire();			//Acquire main order book lock

				order_book_details[manager_customer_token]->customer_seating_stat = GOT_SEAT; //updating seating status of customer in order book

				order_book_details[manager_customer_token]->table_number = x;			//assigning the free table to the customer

				DEBUG('q',"\nseat given to customer %d!!! !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",order_book_details[manager_customer_token]->customer_no);

				manager_customer_tableCV->Signal(manager_customer_tablelock);


				ordermantainlock->Release();				//Release main order book lock


				manager_customer_tablelock->Release();



				break;

			}
			 DEBUG('q',"Customer is informed by the Manager-the restaurant is full\n");
			tablelock[x]->Release();

	}



	}

	else
	{

		tablelinelock->Release();
	}

	currentThread->Yield();

	/******************************************
	 	BAGGING THE FOOD WHEN REQUIRED
	 ******************************************/

	ordermantainlock->Acquire();


			 for(int m=0;m<=index_orderbook;m++)
			 {
				 if(order_book_details[m]->order_stat == FOOD_PENDING )
				 {

					 total_foodlock->Acquire();

					 if((available_items_obj->d6b - order_book_details[m]->d6b)>0 &&
							 (available_items_obj->d3b - order_book_details[m]->d3b)>0 &&
							 (available_items_obj->sb - order_book_details[m]->sb)>0 &&
							 (available_items_obj->fries - order_book_details[m]->fries)>0)
					 {
						 available_items_obj->d6b = available_items_obj->d6b - order_book_details[m]->d6b;
						 available_items_obj->d3b = available_items_obj->d3b - order_book_details[m]->d3b;
						 available_items_obj->sb = available_items_obj->sb - order_book_details[m]->sb;
						 available_items_obj->fries = available_items_obj->fries - order_book_details[m]->fries;

						 order_book_details[m]->order_stat= FOOD_PACKED;

						 DEBUG('t', "FOOD packed by manager!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
						 DEBUG('q',"Manager packed the food for Customer %d\n",order_book_details[m]->customer_no);
					}
					 total_foodlock->Release();
				 }

			 }
			 ordermantainlock->Release();
// currentThread->Yield();

	/*******************************************************************************
	 	 DELIVER FOOD WHEN FOOD PACKED and
	 	1) TELL WAITER WHEN EAT_IN and CUSTOMER HAS A TABLE or
	 	2) TO_GO
	********************************************************************************/

	ordermantainlock->Acquire();

	for(int l=0;l<=index_orderbook;l++)
	{
		if(order_book_details[l]->order_stat==FOOD_PACKED)
		{
			if(order_book_details[l]->eat_pref==TO_GO)
			{
				togo_waitinglock->Acquire();

				togo_food_ready_token=l;

				togo_waitingCV->Broadcast(togo_waitinglock);
				togo_waitinglock->Release();

			}

			else					// if eat in
			{
				if(order_book_details[l]->customer_seating_stat==GOT_SEAT)
				{
					for(int g=0;g<WAITERS;g++)
					{
						if(waiter_stat[g]==WAITER_FREE)
						{
							waiterlock[g]->Acquire();
							waiter_stat[g]=WAITER_BUSY;
							waiter_tokennumber[g]=l;
							 DEBUG('q',"Manager gives Token number %d to Waiter %d for Customer %d\n",
									 index_orderbook,g,order_book_details[l]->customer_no);
							waiterCV[g]->Signal(waiterlock[g]);
							waiterlock[g]->Release();
						}

						break;
					}
				}
			}
		}
	}

	ordermantainlock->Release();


	/*********************************************
	ACTING AS ORDER TAKER WHEN ORDERTAKER'S BUSY
	********************************************/
	int flag=1;

	for(int r=0; r<ORDERTAKERS;r++)
	{

		if (ordertakerstatus[r]==FREE)
		{
			flag=0;
			break;
		}
	}

	customerlinelock->Acquire();

	if((flag==1) && linelength>3)
	{


		is_mang_takingorder1=YES;
		int iterations=2;



		DEBUG('q',"MANAGER TAKING ORDER of customer %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",manager_orderbook_obj->token_number);
			DEBUG('t',"\nMANAGER TAKING ORDER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			DEBUG('t',"\nbefore acquiring customerlinelock");


			iterations--;


			customerlineCV->Signal(customerlinelock);


			linelength--;


				manager_cust_orderlock->Acquire();		//Acquires talk lock so that customer and manager can talk mutually exclusively
				customerlinelock->Release();

				manager_cust_orderCV->Wait(manager_cust_orderlock);			// waiting for order to be placed


			/******************************************
					 * CALCULATING MONEY
			 *******************************************/

			money_received_manager= (manager_orderbook_obj->d6b*6) + (manager_orderbook_obj->d3b*3) + (manager_orderbook_obj->sb*2)
							+ (manager_orderbook_obj->fries*1)	+ (manager_orderbook_obj->soda*1);
					totalmoneylock->Acquire();
					totalmoney= totalmoney + money_received_manager;
					totalmoneylock->Release();

					DEBUG('q'," manager CALCULATING MONEY and giving bill to customer %d !!!!!!!!!!!!!!!!!!!!!\n",manager_orderbook_obj->token_number);
				DEBUG('t',"\n managerrrrrr CALCULATING MONEY and give bill!!!!!!!!!!!!!!!!!!!!!");

				manager_cust_orderCV->Signal(manager_cust_orderlock);
				manager_cust_orderCV->Wait(manager_cust_orderlock);								// waiting to get money from customer

				DEBUG('t',"\n managerrrrr checking for order details!!!!!!!!!!!!!!!!!!!!!!");


				if ((manager_orderbook_obj->soda!=0)&&(manager_orderbook_obj->fries==0)&&(manager_orderbook_obj->d6b==0)&&
						(manager_orderbook_obj->d3b==0)&&(manager_orderbook_obj->sb==0))
				{
					// decrease from inventory with lock**********

					manager_orderbook_obj->mang_order_status_obj= ORDER_FINISH;
					manager_cust_orderCV->Signal(manager_cust_orderlock);

					manager_cust_orderlock->Release();

					ordermantainlock->Acquire();
					order_book_details[index_orderbook]->order_stat= FOOD_GIVEN;
					ordermantainlock->Release();
				}


				else if(manager_orderbook_obj->eat_pref==TO_GO)
				{
					total_foodlock->Acquire();


				if(( available_items_obj->d3b-manager_orderbook_obj->d3b>=0) && ( available_items_obj->d6b-manager_orderbook_obj->d6b>=0)
						&& ( available_items_obj->sb-manager_orderbook_obj->sb>=0) && ( available_items_obj->fries-manager_orderbook_obj->fries>=0))	// calculate if food is ready
				{

					// decrease from inventory with lock


					available_items_obj->d3b-= manager_orderbook_obj->d3b;
					available_items_obj->d6b-= manager_orderbook_obj->d6b;
					available_items_obj->sb-= manager_orderbook_obj->sb;
					available_items_obj->fries-= manager_orderbook_obj->fries;


					total_foodlock->Release();

					DEBUG('t',"\nfood given to the customer by manager !!!!!!!!!!!!!!!!!");
					DEBUG('q',"food given to the customer%d by manager !!!!!!!!!!!!!!!!!\n",manager_orderbook_obj->token_number);

					ordermantainlock->Acquire();
					order_book_details[index_orderbook]->order_stat= FOOD_GIVEN;
					ordermantainlock->Release();

					manager_orderbook_obj->mang_order_status_obj=ORDER_FINISH;
					manager_cust_orderCV->Signal(manager_cust_orderlock);
					manager_cust_orderlock->Release();

				}






				else
				{

					DEBUG('t', "MANAGERRR GIVING TO_GO ORDER PENDING STATUS");

					total_foodlock->Release();
					manager_orderbook_obj->mang_order_status_obj=ORDER_PENDING;
					manager_cust_orderCV->Signal(manager_cust_orderlock);


					ordermantainlock->Acquire();	// acquring lock to place order

					index_orderbook++;
					manager_orderbook_obj->token_number=index_orderbook;
					manager_cust_orderlock->Release();
					DEBUG('q',"token no. %d given to the customer%d by manager !!!!!!!!!!!!!!!!!\n",index_orderbook,manager_orderbook_obj->token_number);
					order_book_details[index_orderbook]->d3b=manager_orderbook_obj->d3b;	// copying the order from order takers data structures
					order_book_details[index_orderbook]->d6b=manager_orderbook_obj->d6b;	// to the order book
					order_book_details[index_orderbook]->sb=manager_orderbook_obj->sb;                      //
					order_book_details[index_orderbook]->fries=manager_orderbook_obj->fries;                //   ----- do -------
					order_book_details[index_orderbook]->soda=manager_orderbook_obj->soda;                  //
					order_book_details[index_orderbook]->eat_pref=manager_orderbook_obj->eat_pref;	//
					order_book_details[index_orderbook]->order_stat= FOOD_PENDING;	//	----do-------
					order_book_details[index_orderbook]->customer_seating_stat= GOT_SEAT;
					order_book_details[index_orderbook]->customer_no= manager_customerno;

					ordermantainlock->Release();


				}
			}

			else if (manager_orderbook_obj->eat_pref == GET_IN)
			{


				manager_orderbook_obj->mang_order_status_obj=ORDER_PENDING;
				DEBUG('t',"\nOT gives pending status to customer");
				manager_cust_orderCV->Signal(manager_cust_orderlock);


				//customerordertakerCV[i]->Wait(customerordertakerlock[i]);
				///////////////////////////////////////////////////////////////////////////////////////////WAIT ADDED ABOVE THIS LINE
				ordermantainlock->Acquire();	// acquring lock to place order

				index_orderbook++;
				manager_orderbook_obj->token_number=index_orderbook;
				manager_cust_orderlock->Release();
				DEBUG('q',"token no. %d given to the customer%d by manager !!!!!!!!!!!!!!!!!\n",index_orderbook,manager_orderbook_obj->token_number);
				order_book_details[index_orderbook]->d3b=manager_orderbook_obj->d3b;	// copying the order from order takers data structures
				order_book_details[index_orderbook]->d6b=manager_orderbook_obj->d6b;	// to the order book
				order_book_details[index_orderbook]->sb=manager_orderbook_obj->sb;                      //
				order_book_details[index_orderbook]->fries=manager_orderbook_obj->fries;                //   ----- do -------
				order_book_details[index_orderbook]->soda=manager_orderbook_obj->soda;                  //
				order_book_details[index_orderbook]->eat_pref=manager_orderbook_obj->eat_pref;	//
				order_book_details[index_orderbook]->order_stat= FOOD_PENDING;	//	----do-------
				order_book_details[index_orderbook]->customer_seating_stat= NO_SEAT	;
				order_book_details[index_orderbook]->customer_no= manager_customerno;

				ordermantainlock->Release();


			}


				//else
				//{
					//customerlinelock->Release();
				//}
				customerlinelock->Acquire();



		}




	else
	{
		customerlinelock->Release();
	}





/********************************************************
* MANAGING INVENTORY AND TOTAL MONEY
********************************************************* */

	inventorylock->Acquire();


	if(inventory_obj->d6b<20)
	{
		//get total money lock
		totalmoneylock->Acquire();


		if(totalmoney==0)
		{
			//go to bank and withdraw money
			totalmoney += 5000;
			DEBUG('t',"money withdrawn from bank so total money now is %d",totalmoney);
			DEBUG('q',"\nmoney withdrawn from bank so total money now is %d",totalmoney);

		}

		//no need to go to the bank

			totalmoney -=50;

			//release total money lock
			totalmoneylock->Release();


			currentThread->Yield();
			currentThread->Yield();
			currentThread->Yield();
			currentThread->Yield();
			currentThread->Yield();


			//order raw materials
			inventory_obj->d6b += 50;

			DEBUG('t',"inventory for 6db increased by 50 to %d now",inventory_obj->d6b);
			DEBUG('q',"\ninventory for 6db increased by 50 to %d now",inventory_obj->d6b);
	}


	else if(inventory_obj->d3b<20)
	{

	//get total money lock
	totalmoneylock->Acquire();

	if(totalmoney==0)
	{
		//go to bank and withdraw money
		totalmoney += 5000;
		DEBUG('q',"money withdrawn from bank so total money now is %d\n",totalmoney);

	}

	//no need to go to the bank

		totalmoney -=50;

		//release total money lock
		totalmoneylock->Release();


		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();


		//order raw materials
		inventory_obj->d3b += 50;


		DEBUG('q',"inventory for 3db increased by 50 to %d",inventory_obj->d3b);
	}

	else if(inventory_obj->sb<20)
	{
	//get total money lock
	totalmoneylock->Acquire();

	if(totalmoney==0)
	{
		//go to bank and withdraw money
		totalmoney += 5000;
		DEBUG('q',"money withdrawn from bank so total money now is %d",totalmoney);

	}

	//no need to go to the bank

		totalmoney -=50;

		//release total money lock
		totalmoneylock->Release();

		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();

		//order raw materials
		inventory_obj->sb += 50;

		DEBUG('q',"inventory for sb increased by 50 to %d",inventory_obj->sb);

	}


	else if(inventory_obj->fries<20)
	{
	//get total money lock
	totalmoneylock->Acquire();

	if(totalmoney==0)
	{
		//go to bank and withdraw money
		totalmoney += 5000;
		DEBUG('q',"money withdrawn from bank so total money now is %d",totalmoney);

	}

	//no need to go to the bank

		totalmoney -=50;

		//release total money lock
		totalmoneylock->Release();

		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();

		//order raw materials
		inventory_obj->fries += 50;

		DEBUG('q',"inventory for fries increased by 50 to !!!!!!!!!!!!!!!!!%d",inventory_obj->fries);
	}
	inventorylock->Release();







}


}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


void cook(int my_cook_number)
{

	cook_stat[my_cook_number]=COOK_FREE;
	while(1)
	{

		while(cook_stat[my_cook_number]==D6B)
		{
			inventorylock->Acquire();
			if(inventory_obj->d6b > 0)
			{

				inventory_obj->d6b-=10;
				inventorylock->Release();
			}


			else
			{
				cook_stat[my_cook_number]=COOK_FREE;
				inventorylock->Release();
				break;
			}


			currentThread->Yield();					// time taken to prepare food

			DEBUG('q',"\nPREPARED 10 UNITS OF D6B!!!!!!! ");


			   DEBUG('q',"Cook %d is going to cook 6-dollar burger\n",my_cook_number);

			total_foodlock->Acquire();
			available_items_obj->d6b+=10;
			total_foodlock->Release();
			currentThread->Yield();

		}

		while(cook_stat[my_cook_number]==D3B)
				{

			inventorylock->Acquire();
						if(inventory_obj->d3b > 0)
						{

							inventory_obj->d3b-=10;
							inventorylock->Release();
						}


						else
						{
							cook_stat[my_cook_number]=COOK_FREE;
							inventorylock->Release();
							break;
						}

					inventorylock->Acquire();
					inventory_obj->d3b--;
					inventorylock->Release();

					currentThread->Yield();					// time taken to prepare food

					DEBUG('q',"\nPREPARED 10 UNITS OF D3B!!!!!!! ");
					   DEBUG('q',"Cook %d is going to cook 3-dollar burger\n",my_cook_number);
					total_foodlock->Acquire();
					available_items_obj->d3b+=10;
					total_foodlock->Release();
					currentThread->Yield();
				}

		while(cook_stat[my_cook_number]==SB)
				{

					inventorylock->Acquire();
						if(inventory_obj->sb > 0)
						{

							inventory_obj->sb-=10;
							inventorylock->Release();
						}


						else
						{
							cook_stat[my_cook_number]=COOK_FREE;
							inventorylock->Release();
							break;
						}

					inventorylock->Acquire();
					inventory_obj->sb--;
					inventorylock->Release();

					currentThread->Yield();					// time taken to prepare food

					DEBUG('q',"\nPREPARED 10 UNITS OF SB!!!!!!! ");
					   DEBUG('q',"Cook %d is going to cook veggie burger\n",my_cook_number);
					total_foodlock->Acquire();
					available_items_obj->sb+=10;
					total_foodlock->Release();
					currentThread->Yield();
				}
		while(cook_stat[my_cook_number]==FRIES)
				{
			inventorylock->Acquire();
						if(inventory_obj->fries > 0)
						{

							inventory_obj->fries-=10;
							inventorylock->Release();
						}


						else
						{
							cook_stat[my_cook_number]=COOK_FREE;
							inventorylock->Release();
							break;
						}
					inventorylock->Acquire();
					inventory_obj->fries--;
					inventorylock->Release();

					currentThread->Yield();					// time taken to prepare food

					DEBUG('q',"\nPREPARED 10 UNITS OF Fries!!!!!!! ");
					   DEBUG('q',"Cook %d is going to cook Fries\n",my_cook_number);
					total_foodlock->Acquire();
					available_items_obj->fries+=10;
					total_foodlock->Release();
					currentThread->Yield();
				}


		if(cook_stat[my_cook_number]==COOK_FREE)
		{
			manager_cooklock->Acquire();
			manager_cookCV->Wait(manager_cooklock);
			   DEBUG('q',"Cook %d returned from break\n",my_cook_number);
			manager_cooklock->Release();
		}



	}



}



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


void waiter(int my_waiternumber)
{
	while(1)
	{
		waiter_tokennumber[my_waiternumber]=-1;
		if(waiter_stat[my_waiternumber]==WAITER_FREE)
		{
			waiterlock[my_waiternumber]->Acquire();
			waiterCV[my_waiternumber]->Wait(waiterlock[my_waiternumber]);
			waiterlock[my_waiternumber]->Release();

			ordermantainlock->Acquire();
			   DEBUG('q'," Waiter %d validates the token number for Customer %d\n",my_waiternumber,order_book_details[waiter_tokennumber[my_waiternumber]]->customer_no);

			tobe_service_tablenumber[my_waiternumber]=	order_book_details[waiter_tokennumber[my_waiternumber]]->table_number;

			tablelock[tobe_service_tablenumber[my_waiternumber]]->Acquire();

			tableCV[tobe_service_tablenumber[my_waiternumber]]->Signal(tablelock[tobe_service_tablenumber[my_waiternumber]]);
			order_book_details[waiter_tokennumber[my_waiternumber]]->order_stat= FOOD_GIVEN;

			   DEBUG('q'," Waiter %d serves food to Customer %d\n",my_waiternumber,order_book_details[waiter_tokennumber[my_waiternumber]]->customer_no);
			DEBUG('t',"\nfood given to the customer by waiter !!!!!!!!!!!!!!!!!!!!!!!!!!");

			tablelock[tobe_service_tablenumber[my_waiternumber]]->Release();
			ordermantainlock->Release();

			waiter_stat[my_waiternumber]=WAITER_FREE;
		    DEBUG('q'," Waiter %d is going on break \n",my_waiternumber);


		}
	}
}

