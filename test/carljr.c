/*
 * carljr.c
 *
 *  Created on: Oct 9, 2010
 *      Author: Abhi
 */




#include "syscall.h"


/***********************************************
DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
************************************************/
	#define WAITERS 5
	#define ORDERTAKERS 5
	#define COOKS 3
	#define CUSTOMERS 30
	#define TABLES 10


/************************************************
ENUM DEFINITIONS :
***********************************************/
#define NULL -1

#define WAIT 0				/* for keeping account of order taker's status*/
#define FREE 1
#define BUSY 2
int ordertakerstatus[ORDERTAKERS];

#define GET_IN 0
#define TO_GO 1


#define FINISH 0
#define PENDING 1
int status[ORDERTAKERS];


#define FOOD_GIVEN 0
#define FOOD_PENDING 1
#define FOOD_PACKED 2
int order_stat;

#define GOT_SEAT 0
#define NO_SEAT 1
int customer_seating_stat;

#define COOK_FREE 0
#define D6B 1
#define D3B 2
#define SB 3
#define FRIES 4
int  cook_stat[COOKS];


#define TABLE_FREE 0
#define TABLE_OCCUPIED 1
int table_stat[TABLES];

#define WAITER_FREE 0
#define WAITER_BUSY 1
int waiter_stat[WAITERS];

#define YES 0
#define NO 1
int is_mang_takingorder1;

#define ORDER_FINISH 0
#define ORDER_PENDING 1




/****************************************************
GENERAL GLOBAL VARIABLES DECLARATIONS
****************************************************/

	int linelength=0;							/* for number of customers in line to place order*/
	int index_orderbook=-1;						/* gives the available index in the order book*/
	int token_number[ORDERTAKERS];
	int d6b[ORDERTAKERS];
	int d3b[ORDERTAKERS];
	int sb[ORDERTAKERS];
	int soda[ORDERTAKERS];
	int fries[ORDERTAKERS];
	int customer_no[ORDERTAKERS];
	int eat_pref[ORDERTAKERS];
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
	int glo_cust_count=0;
	int glo_ot_count=0;
	int glo_cook_count=0;
	int glo_waiter_count=0;

/******************************************************
* DECLARING ALL THE LOCKS USED IN SIMULATION
*******************************************************/
	int customerlinelock;
	int customerordertakerlock[ORDERTAKERS];
	int ordermantainlock;
	int togo_waitinglock;
	int tablelinelock;
	int manager_customer_tablelock;
	int manager_cooklock;
	int total_foodlock;
	int inventorylock;
	int tablelock[TABLES];
	int waiterlock[WAITERS];
	int manager_cust_orderlock;
	int totalmoneylock;
	int customer_gonelock;
	int customer_numberlock;
	int ot_numberlock;
	int cook_numberlock;
	int waiter_numberlock;
	int printlock;
/******************************************************
DECLARING ALL THE CONDITION VARIABLES USED IN SIMULATION
*******************************************************/
	int customerlineCV;
	int customerordertakerCV[ORDERTAKERS];
	int ordermantainCV;
	int togo_waitingCV;
	int tablelineCV;
	int manager_customer_tableCV;
	int manager_cookCV;
	int total_foodCV;
	int inventoryCV;
	int tableCV[TABLES];
	int waiterCV[WAITERS];
	int manager_cust_orderCV;


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
	int eat_pref;
	int d6b,d3b,sb,fries,soda,table_number;
	 int order_stat;
	int customer_seating_stat;

	}order_book_details[100];	/* array of structure containing customer details , so as to contain*/
										/*	the details of all the customers.*/


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
	}inventory_obj;

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
		}available_items_obj;

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
			}pending_order_obj;

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
							int customer_no;
							int d6b,d3b,sb,fries,soda,token_number;
							int eat_pref;
							int mang_order_status_obj;
						}manager_orderbook_obj;

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
									int customer_no;
									int d6b,d3b,sb,fries,soda;
									int eat_pref;

								}customer_order_obj[50];



/******************************************************
* DECLARING ALL THE FUNCTIONS USED IN SIMULATION
*******************************************************/
	void ordertaker(void);
	void customer(void);
	void manager(void);
	void cook(void);
	void waiter(void);
	void full_simulation(void);


/*/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////*/


void main()
{

		/* allocating memory to  structures*/




int k,i,z;

	/***********************************************************
	 initializing locks and CV's
	***********************************************************/
	customerlinelock= CreateLock("customerlinelock",16 );
	customerlineCV= CreateCV("customerlineCV",14);

	/*initializing ordertaker's locks and condition variables*/
	 for( k=0; k<ORDERTAKERS; k=k+1)
	{
	char *	name,*name1;

	name=(char*) "customerordertakerlock";
	name1=(char*) "customerordertakerCV";


	customerordertakerlock[k]= CreateLock(name,24);
	customerordertakerCV[k]=  CreateCV(name1,22);
	}

	/*initializing tables locks and condition variables*/
	for( k=0; k<TABLES; k=k+1)
		{
			 char *name,*name1;

			 name=(char*) "tablelock";
			 name1=(char*) "tableCV";


			 tablelock[k]= CreateLock(name,11);
			 tableCV[k]= CreateCV(name1,9);
		}
	/*printf("Before waiters\n");*/
		/*initializing waiter's locks and condition variables*/
	for( k=0; k<WAITERS; k=k+1)
		{
			 char *name,*name1;

			 name=(char*) "waiterlock";
			 name1=(char*) "waiterCV";


			 waiterlock[k]= CreateLock(name,12);
			 waiterCV[k]= CreateCV(name1,10);
		}


		/*Initializing locks and condition variables..*/

	 tablelinelock= CreateLock("tablelinelock",30);
	 tablelineCV = CreateCV("tablelineCV",30);

	 manager_customer_tablelock=CreateLock("manager_customer_tablelock",30);
	 manager_customer_tableCV = CreateCV("manager_customer_tableCV",30);

	 ordermantainlock = CreateLock ("ordermantainlock",30);
	 ordermantainCV = CreateCV("ordermantainCV ",30);

	 manager_cooklock=CreateLock ("manager_cooklock",30);
	 manager_cookCV=CreateCV("manager_cookCV",30);

	 total_foodlock=CreateLock("total_foodlock",30);
	 total_foodCV=CreateCV("total_foodCV",30);

 	 inventorylock= CreateLock("inventorylock",30);
	 inventoryCV= CreateCV("inventoryCV",30);

	 manager_cust_orderlock=CreateLock("manager_cust_orderlock",30);
	 manager_cust_orderCV= CreateCV("manager_cust_orderCV",30);

	 togo_waitinglock=CreateLock("togo_waitinglock",30);
	 togo_waitingCV=CreateCV("togo_waitingCV",30);

	 totalmoneylock=CreateLock("totalmoneylock",30);

	 customer_gonelock=CreateLock("customer_gonelock",30);

	  customer_numberlock=CreateLock("customer_numberlock",30);
	  ot_numberlock=CreateLock("ot_numberlock",30);
	  cook_numberlock=CreateLock("cook_numberlock",30);
	  waiter_numberlock=CreateLock("waiter_numberlock",30);

	  printlock= CreateLock("printlock",30);



	/*initializing ordertaker's personal order book variables*/
	for( i=0; i<ORDERTAKERS; i++)								/* order takers personal order book*/
	{
	ordertakerstatus[i] = BUSY;
	d6b[i]=NULL;
	d3b[i]=NULL;
	sb[i]=NULL;
	soda[i]=NULL;
	fries[i]=NULL;
	customer_no[i]=NULL;

	}

	for ( z=0;z<COOKS;z=z+1)						/* initializing all cooks status to be free*/
	{
		cook_stat[z]=COOK_FREE;
	}

	/*total available made food during start of our simulation*/
	available_items_obj.d3b=400;
	available_items_obj.d6b=500;
	available_items_obj.sb=400;
	available_items_obj.fries=400;

	/*total pending orders during start of our simulation*/
	pending_order_obj.d3b=0;
	pending_order_obj.d6b=0;
	pending_order_obj.sb=0;
	pending_order_obj.fries=0;

	/*total available food inventory during start of our simulation*/
	inventory_obj.d3b=100;
	inventory_obj.d6b=100;
	inventory_obj.sb=100;
	inventory_obj.fries=100;

	/*variable to check whether ordertaker or manager is taking the order from customer*/
	is_mang_takingorder1= NO;

	/*count to keep check of whether all customers left the restaurant or not*/
	customers_left=0;


	full_simulation();

	for ( i=0;i<100000;i++)
	{
		Yield();
	}



}



/*/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////*/


void full_simulation(void)
       {
  #define AUTO_ORDER

int f,w,i;
for( f=0;f<CUSTOMERS;f=f+1)
{
				customer_order_obj[f].d6b =1;
               customer_order_obj[f]. d3b= 1;
               customer_order_obj[f]. sb= 1;
               customer_order_obj[f]. fries= 1;
               customer_order_obj[f]. soda= 1;
                 w= 1;
                 if (w==0)
                 {
                         customer_order_obj[f]. eat_pref= TO_GO;
                 }
                else
                 {
                        customer_order_obj[f].eat_pref= GET_IN;
                 }
}

Acquire(printlock);
				 Printf((unsigned int) "\nSTARTING FULL_SIMULATION");
				 Printf((unsigned int) "\nNO. OF CUSTOMERS:[30]");
				 Printf((unsigned int) "\nNO. OF WAITERS:[5] ");
				 Printf((unsigned int) "\nNO. OF ORDERTAKERS:[5]");
				 Printf((unsigned int) "\nNO. OF COOKS:[5]");
				 Printf((unsigned int)"\n");
				 Release(printlock);



				/*forking 30 CUSTOMERS*/
                for( f=0;f<CUSTOMERS;f=f+1)
                {



                    Fork(customer);
                }

				/*forking 5 ordertakers*/
                 for( f=0;f<ORDERTAKERS;f=f+1)
                 {

                    Fork(ordertaker);
                }

				/*forking 5 waiters*/
                 for( f=0;f<WAITERS;f++)
                 {

                 Fork(waiter);
                  }

				/*forking 5 cooks*/
                 for( f=0;f<COOKS;f++)
                 {


                    Fork(cook);
                 }



                Fork(manager);




       }




/*/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////*/


void ordertaker(void)
{
	int m,l,g,ci;
int my_otnumber;

Acquire(ot_numberlock);
	 my_otnumber= glo_ot_count;
	 glo_ot_count=glo_ot_count+1;
	 Release(ot_numberlock);
while(1)
	{

	/***********************************************
			  CHECKING WHEN TO FINISH THE ORDER TAKER
		 ********************************************/
	Acquire(customer_gonelock);

		if(customers_left==CUSTOMERS)
		{
			Acquire(printlock);
			Printf((unsigned int)"\nORDERTAKER ENDS HERE!!!\n");
			Release(printlock);
			Release(customer_gonelock);
			Exit();
		}
		else
		{
			Release(customer_gonelock);
		}


	/*******************************************
	 interacting with customers in line for order
	 *********************************************/
		Acquire(customerlinelock);

	/*while there are customers in the line*/

	while(linelength>0)

	{

		if(is_mang_takingorder1==NO)                  /*check whether manager is going to interact with next customer*/
		{
			Acquire(printlock);
			Printf1((unsigned int)"\n order taker ",my_otnumber);
			Printf((unsigned int )"waking a customer in line .... ");
			Release(printlock);
			Signal(customerlineCV,customerlinelock) ;
		linelength= linelength-1;								  /*take 1st customer out from the line*/
		ordertakerstatus[my_otnumber]=WAIT;


	/*Acquires talk lock so that customer and order taker can talk mutually exclusively*/
		Acquire(customerordertakerlock[my_otnumber]);
		Release(customerlinelock);
		Acquire(printlock);

		Release(printlock);
	Wait(customerordertakerCV[my_otnumber],customerordertakerlock[my_otnumber]);			/* waiting for order to be placed*/


	/******************************************
	 * CALCULATING MONEY
	 *******************************************/

	money_received[my_otnumber]= (d6b[my_otnumber]*6) + (d3b[my_otnumber]*3) + (sb[my_otnumber]*2) + (fries[my_otnumber]*1)
									+ (soda[my_otnumber]*1);
	Acquire(totalmoneylock);
	totalmoney= totalmoney + money_received[my_otnumber];
	Release(totalmoneylock);

	Acquire(printlock);
	Printf1((unsigned int)"\nOrderTaker ",my_otnumber);
	Printf1((unsigned int)"is taking order of Customer",ot_customerno[my_otnumber]);
	Release(printlock);


	Signal(customerordertakerCV[my_otnumber],customerordertakerlock[my_otnumber]);
	Wait(customerordertakerCV[my_otnumber],customerordertakerlock[my_otnumber]);				/* waiting to get money from customer*/



	/*when only soda is ordered*/
	if ((soda[my_otnumber]!=0)&&(fries[my_otnumber]==0)&&(d6b[my_otnumber]==0)&&(d3b[my_otnumber]==0)&&(sb[my_otnumber]==0))
	{


		status[my_otnumber]= FINISH;

		Acquire(printlock);
		Printf1((unsigned int)"\nOrderTaker",my_otnumber);
				Printf1((unsigned int)" gives soda to Customer",ot_customerno[my_otnumber]);
		Release(printlock);
		Signal(customerordertakerCV[my_otnumber],customerordertakerlock[my_otnumber]);

		Release(customerordertakerlock[my_otnumber]);
	}


	else if(eat_pref[my_otnumber]==TO_GO)
	{
		Acquire(total_foodlock);

		/*check whether to_go food can be given instantly*/
		if(( available_items_obj.d3b-d3b[my_otnumber]>=0) && ( available_items_obj.d6b-d6b[my_otnumber]>=0)
				&& ( available_items_obj.sb-sb[my_otnumber]>=0) && ( available_items_obj.fries-fries[my_otnumber]>=0))	/* calculate if food is ready*/
		{

			/* decrease ordered food from total food*/

			available_items_obj.d3b= 	available_items_obj.d3b-d3b[my_otnumber];
			available_items_obj.d6b=available_items_obj.d6b- d6b[my_otnumber];
			available_items_obj.sb=available_items_obj.sb- sb[my_otnumber];
			available_items_obj.fries=available_items_obj.fries- fries[my_otnumber];

			Release(total_foodlock);

			Acquire(printlock);
			Printf1((unsigned int)"\nOrderTaker",my_otnumber);
					Printf1((unsigned int)" gives food to Customer",ot_customerno[my_otnumber]);
			Release(printlock);
			status[my_otnumber]=FINISH;
			Signal(customerordertakerCV[my_otnumber],customerordertakerlock[my_otnumber]);

			Release(customerordertakerlock[my_otnumber]);

		}


	else /*when food is not ready...token is given*/
	{
		Release(total_foodlock);

		status[my_otnumber]=PENDING;

		Signal(customerordertakerCV[my_otnumber],customerordertakerlock[my_otnumber]);


		Acquire(ordermantainlock);

		index_orderbook=index_orderbook+1;
		token_number[my_otnumber]=index_orderbook;
		Release(customerordertakerlock[my_otnumber]);

		order_book_details[index_orderbook].d3b=d3b[my_otnumber];
		order_book_details[index_orderbook].d6b=d6b[my_otnumber];
		order_book_details[index_orderbook].sb=sb[my_otnumber];
		order_book_details[index_orderbook].fries=fries[my_otnumber];
		order_book_details[index_orderbook].soda=soda[my_otnumber];
		order_book_details[index_orderbook].eat_pref=eat_pref[my_otnumber];
		order_book_details[index_orderbook].order_stat= FOOD_PENDING;
		order_book_details[index_orderbook].customer_seating_stat= GOT_SEAT;
		order_book_details[index_orderbook].customer_no= ot_customerno[my_otnumber];

		Acquire(printlock);
		Printf1((unsigned int)"\nOrderTaker",my_otnumber);
		Printf1((unsigned int)"gives token number",index_orderbook);
				Printf1((unsigned int)" to Customer",ot_customerno[my_otnumber]);
		Release(printlock);
		Release(ordermantainlock);



	}

}


else if (eat_pref[my_otnumber] == GET_IN)
{


	status[my_otnumber]=PENDING;

	Signal(customerordertakerCV[my_otnumber],customerordertakerlock[my_otnumber]);

	Acquire(ordermantainlock);

	index_orderbook=index_orderbook+1;
	token_number[my_otnumber]=index_orderbook;
	Release(customerordertakerlock[my_otnumber]);


	order_book_details[index_orderbook].d3b=d3b[my_otnumber];
	order_book_details[index_orderbook].d6b=d6b[my_otnumber];
	order_book_details[index_orderbook].sb=sb[my_otnumber];
	order_book_details[index_orderbook].fries=fries[my_otnumber];
	order_book_details[index_orderbook].soda=soda[my_otnumber];
	order_book_details[index_orderbook].eat_pref=eat_pref[my_otnumber];
	order_book_details[index_orderbook].order_stat= FOOD_PENDING;
	order_book_details[index_orderbook].customer_seating_stat= NO_SEAT	;
	order_book_details[index_orderbook].customer_no= ot_customerno[my_otnumber];
	Acquire(printlock);
	Printf1((unsigned int)"\nOrderTaker ",my_otnumber);
	Printf1((unsigned int)	" gives token number",index_orderbook);
	Printf1((unsigned int)" to Customer",ot_customerno[my_otnumber]);
	Release(printlock);

	Release(ordermantainlock);


}
	Acquire(customerlinelock);
	}

		else
		{
			break;
		}


	}



	/*****************************************
			 checking if any food can be bagged
		 *****************************************/
		 Acquire(ordermantainlock);

		/*setting ordertaker's status to busy*/
		 ordertakerstatus[my_otnumber]=BUSY;
		 Release(customerlinelock);

		 /*traversing the order book and checking whether any order can be packed*/

		 for( m=0;m<=index_orderbook;m=m+1)
		 {
			 if(order_book_details[m].order_stat == FOOD_PENDING )
			 {
				 ci=order_book_details[m].customer_no;
				 Acquire(total_foodlock);

				 if((available_items_obj.d6b - order_book_details[m].d6b)>0 &&
						 (available_items_obj.d3b - order_book_details[m].d3b)>0 &&
						 (available_items_obj.sb - order_book_details[m].sb)>0 &&
						 (available_items_obj.fries - order_book_details[m].fries)>0)
				 {
					 available_items_obj.d6b = available_items_obj.d6b - order_book_details[m].d6b;
					 available_items_obj.d3b = available_items_obj.d3b - order_book_details[m].d3b;
					 available_items_obj.sb = available_items_obj.sb - order_book_details[m].sb;
					 available_items_obj.fries = available_items_obj.fries - order_book_details[m].fries;

					 order_book_details[m].order_stat= FOOD_PACKED;

					 Acquire(printlock);
					Printf1((unsigned int)"\nOrderTaker ",my_otnumber);
							Printf1((unsigned int)	" has packed the food for Customer",ci);
					Release(printlock);
				}
				 Release(total_foodlock);
			 }

		 }
		 Release(ordermantainlock);





	/*******************************************************************************
			 DELIVER FOOD WHEN FOOD PACKED and
			1) TELL WAITER WHEN EAT_IN and CUSTOMER HAS A TABLE or
			2) TO_GO
		********************************************************************************/
		 Yield();

	Acquire(ordermantainlock);

		for( l=0;l<=index_orderbook;l=l+1)
		{
			if(order_book_details[l].order_stat==FOOD_PACKED)
			{

				/*if its to_go and food is packed broadcast the token number of customer*/

				if(order_book_details[l].eat_pref==TO_GO)
				{
					Acquire(togo_waitinglock);

					togo_food_ready_token=l;

					Broadcast(togo_waitingCV, togo_waitinglock);
					Release(togo_waitinglock);

				}

				else	/* when eat in tell waiter to deliver the food if customer has got the table*/
				{
					if(order_book_details[l].customer_seating_stat==GOT_SEAT)
					{
						for( g=0;g<WAITERS;g=g+1)
						{
							if(waiter_stat[g]==WAITER_FREE)
							{

								Acquire(waiterlock[g]);
								waiter_stat[g]=WAITER_BUSY;
								waiter_tokennumber[g]=l;
								Acquire(printlock);
								Printf1((unsigned int)"OrderTaker ",l);
										Printf1((unsigned int)" gives Token number ",g);
										Printf1((unsigned int)"to Waiter",my_otnumber);
												Printf1((unsigned int)" for Customer",order_book_details[l].customer_no);
								Release(printlock);
								Signal(waiterCV[g],waiterlock[g]);
								Release(waiterlock[g]);
							}
							break;
						}
					}
				}
			}
		}

		Release(ordermantainlock);


		/*************************************
		 * kill order taker when no work left
		 **************************************/
		{
			ordertakerstatus[my_otnumber]=FREE;



		}



	}		/* end while(1)*/

Yield();
Yield();
Yield();

}  /* end ot function*/




/*/////////////////////////////////////////////////////////////////////////////////////////////////
								/////////////CUSTOMER////////////
/////////////////////////////////////////////////////////////////////////////////////////////////*/

void customer( void)
{

	int mynumber;
	int m,w,my_table;
	int myordertaker;

	Acquire(customer_numberlock);
 mynumber=glo_cust_count;
 glo_cust_count=glo_cust_count+1;
 Release(customer_numberlock);

 /*variable to store id of customer's order-taker*/
Acquire(customerlinelock);
Acquire(printlock);
Printf1((unsigned int)"\ncustomer ",mynumber);
Printf((unsigned int )"waiting in line....");
Release(printlock);

linelength=linelength+1;									 /*customer gets in line as soon as they enter the restaurant*/
Wait(customerlineCV,customerlinelock);

if( is_mang_takingorder1 == NO)                   /*check if customer has got signal from manager*/

{

	for( m=0;m<ORDERTAKERS;m=m+1)                /*checks which ordertaker is waiting for customer*/
	{
		if(ordertakerstatus[m]==WAIT)
		{

			myordertaker=m;
			ordertakerstatus[m]=BUSY;
			break;
		}
	}
	Release(customerlinelock);
	Acquire(customerordertakerlock[myordertaker]);


	ot_customerno[myordertaker]= mynumber;


#ifdef AUTO_ORDER
 {
	d6b[myordertaker]= 1;
	d3b[myordertaker]= 1;
	sb[myordertaker]= 1;
	fries[myordertaker]= 1;
	soda[myordertaker]= 1;

	 w= 1;


	  if (w==0)
	  {
		  eat_pref[myordertaker]= TO_GO;
	  }
	  else
	  {
		  eat_pref[myordertaker]= GET_IN;

	  }
 }
#endif
#ifdef MANUAL_ORDER
 {
	    d6b[myordertaker]= customer_order_obj[mynumber].d6b;
		d3b[myordertaker]= customer_order_obj[mynumber].d3b;
		sb[myordertaker]= customer_order_obj[mynumber].sb;
		fries[myordertaker]= customer_order_obj[mynumber].fries;
		soda[myordertaker]= customer_order_obj[mynumber].soda;
		eat_pref[myordertaker]= customer_order_obj[mynumber].eat_pref;


}
#endif


 Acquire(printlock);

		Printf((unsigned int)"\n");


				Printf1((unsigned int)"\n customer ",mynumber);
						Printf1((unsigned int)" has ordered",d6b[myordertaker]);
						Printf((unsigned int )	 "6-dollar burger");

				Printf1((unsigned int)"\n customer ",mynumber);
				Printf1((unsigned int)" has ordered",d3b[myordertaker]);
				Printf((unsigned int )" 3-dollar burger");

				Printf1((unsigned int)"\n customer ",mynumber);
				Printf1((unsigned int)" has ordered",sb[myordertaker]);
				Printf((unsigned int )" veggie burger");

				Printf1((unsigned int)"\n customer ",mynumber);
								Printf1((unsigned int)" has ordered",fries[myordertaker]);
								Printf((unsigned int )" fries");

								Printf1((unsigned int)"\n customer ",mynumber);
																Printf1((unsigned int)" has ordered",soda[myordertaker]);
																Printf((unsigned int )" soda");


		 Printf((unsigned int)"\n");
		 Release(printlock);
			if(eat_pref[myordertaker]==GET_IN)
		{
				Acquire(printlock);
				Printf1((unsigned int)"\n customer  ",mynumber);
				Printf((unsigned int)"chooses eat-in food");
				Release(printlock);
		}
			else
			{
				Acquire(printlock);
				Printf1((unsigned int)"\n customer ",mynumber);
						Printf((unsigned int )" chooses to-go food\n");
				Release(printlock);
			}
			Acquire(printlock);
			Printf((unsigned int)"\n");
			Release(printlock);


   Signal( customerordertakerCV[myordertaker],customerordertakerlock[myordertaker]);
	Wait(customerordertakerCV[myordertaker],customerordertakerlock[myordertaker]);

	/* giving money*/



	/*waiting for either  food or token*/

	Signal(customerordertakerCV[myordertaker],customerordertakerlock[myordertaker]);
	Wait(customerordertakerCV[myordertaker],customerordertakerlock[myordertaker]);


	if (eat_pref[myordertaker] == TO_GO)
	{
		if(status[myordertaker]==FINISH)
		{

			Acquire(printlock);
		Printf1((unsigned int)"\ncustomer ", mynumber);
				Printf1((unsigned int)" receives food from the OrderTaker",myordertaker);


		Printf1((unsigned int)"\ncustomer ", mynumber);
				Printf((unsigned int )	" GOT TO_GO READY FOOD ......byeeeeeeeeeee");
		Release(printlock);
		Acquire(customer_gonelock);
		customers_left= customers_left+1;

		Release(customer_gonelock);
		Acquire(printlock);

Release(printlock);
		Release(customerordertakerlock[myordertaker]);
		Exit();	/* customer leaves the restaurant*/


		}
		else
		{
		customer_token_number[mynumber]	= token_number[myordertaker];

		Acquire(printlock);
		Printf1((unsigned int)"\nCustomer ",mynumber);
		Printf1((unsigned int)	" is given token number ",customer_token_number[mynumber]);
		Printf1((unsigned int)	" by the OrderTaker ",myordertaker);
		Release(printlock);
		/*customerordertakerCV[myordertaker].Signal(customerordertakerlock[myordertaker]);*/
		Release(customerordertakerlock[myordertaker]);

		Acquire(togo_waitinglock);
		Acquire(printlock);
		Printf1((unsigned int)" \ncustomer ", mynumber);
				Printf((unsigned int )" waiting to be get his number broadcasted !!!\n");
		Release(printlock);
		Wait(togo_waitingCV,togo_waitinglock);				/* wait for token no to be broadcasted*/


		while(1)
		{

		if(customer_token_number[mynumber]==togo_food_ready_token)
		{

			Release(togo_waitinglock);
			Acquire(ordermantainlock);
			order_book_details[customer_token_number[mynumber]].order_stat= FOOD_GIVEN;
			Release(ordermantainlock);

			Acquire(printlock);
			Printf1((unsigned int)"\n customer ", mynumber);
					Printf((unsigned int )" RECEIVES FOOD FROM ORDERTAKER\n");
			Printf1((unsigned int)"\n customer ", mynumber);
					Printf((unsigned int )"GOT MY TO GO FOOD NOW I AM LEAVING  ..............");
			Release(printlock);
			Acquire(customer_gonelock);

			customers_left=customers_left+1;

			Release(customer_gonelock);
			Acquire(printlock);

Release(printlock);
Exit();
		}

			Wait(togo_waitingCV,togo_waitinglock);
		}

		}
	}

	/*when the customer has choosen eat_in*/
	else if(eat_pref[myordertaker] == GET_IN)
	{

		if(status[myordertaker]==PENDING)
		{
			customer_token_number[mynumber]	= token_number[myordertaker];
			Acquire(printlock);
			Printf1((unsigned int)"\nCustomer ",mynumber);
			Printf1((unsigned int)" is given token number ",customer_token_number[mynumber]);
			Printf1((unsigned int)	"by the OrderTaker ",myordertaker);

	/************************************************
	  cutomer looking for a table to sit
	 ************************************************/

			Printf1((unsigned int)"\ncustomer ", mynumber);
					Printf((unsigned int )"looking for free table !!!");
			Release(printlock);
			Acquire(tablelinelock);


			Release(customerordertakerlock[myordertaker]);

			tableline=tableline+1;

			Wait(tablelineCV,tablelinelock);          /*wait for manager to wake him when he is assigned a table*/

			manager_customer_token= customer_token_number[mynumber];

			Release(tablelinelock);

			Acquire(manager_customer_tablelock);

			Signal(manager_customer_tableCV,manager_customer_tablelock);
			Acquire(printlock);
			Printf1((unsigned int)"\ncustomer ",mynumber);
			Printf((unsigned int )" is waiting  for  table to be assigned by mang");
			Release(printlock);
			Wait(manager_customer_tableCV,manager_customer_tablelock);

			Release(manager_customer_tablelock);

			Acquire(ordermantainlock);

			/*customer finds which table has been assigned to him*/

			 my_table= order_book_details[customer_token_number[mynumber]].table_number;

			Release(ordermantainlock);

			Acquire(tablelock[my_table]);

			Acquire(printlock);
			Printf1((unsigned int)"\ncustomer",mynumber);
			Printf1((unsigned int)" is seated at table number",my_table);
			Printf1((unsigned int)"\nCustomer ",mynumber);
			Printf((unsigned int )" is waiting for the waiter to serve the food");
			Release(printlock);
			Wait(tableCV[my_table],tablelock[my_table]);


			Acquire(printlock);
			Printf1((unsigned int)"\nCustomer",mynumber);
								Printf((unsigned int )" is served by waiter");
Printf1((unsigned int)"\n   HAVING MY FOOD  ",mynumber);

Release(printlock);
			Yield();						/* time given to eat food*/

			Acquire(printlock);
			Printf1((unsigned int)"\n customer ", mynumber);
			Printf((unsigned int )" had my food on table ....byeeeeeee!!! ");

			Release(printlock);
			Acquire(customer_gonelock);
			customers_left=customers_left+1;

			Release(customer_gonelock);
			Acquire(printlock);

Release(printlock);
table_stat[my_table]= TABLE_FREE;          /*set table status to free when leaving*/

			Release(tablelock[my_table]);

			Exit();				/* only after food is delivered*/

		}

		else									/* when only soda and get in*/
		{

			customer_token_number[mynumber]	= token_number[myordertaker];

			Acquire(printlock);
			Printf1((unsigned int)"\nCustomer ",mynumber);
			Printf1((unsigned int)"is given token number ",customer_token_number[mynumber]);
			Printf1((unsigned int)	" by the OrderTaker",myordertaker);

			/************************************************
     		 looking for a table to sit
	    	 ************************************************/

						Printf1((unsigned int)"\n customer",mynumber);
								Printf((unsigned int )"looking for free table !!!");
						Release(printlock);
						Acquire(tablelinelock);

						Release(customerordertakerlock[myordertaker]);

						tableline=tableline+1;

						Wait(tablelineCV,tablelinelock);

						manager_customer_token= customer_token_number[mynumber];


						Release(tablelinelock);

						Acquire(manager_customer_tablelock);

						Signal(manager_customer_tableCV,manager_customer_tablelock);

						Acquire(printlock);
						Printf1((unsigned int)"\ncustomer ",mynumber);
						Printf((unsigned int )"is waiting to sit on the table");
						Release(printlock);
						Wait(manager_customer_tableCV,manager_customer_tablelock);

						Release(manager_customer_tablelock);

						Acquire(ordermantainlock);

						 my_table= order_book_details[customer_token_number[mynumber]].table_number;

						Release(ordermantainlock);

						Acquire(tablelock[my_table]);
						Acquire(printlock);
						Printf1((unsigned int)"\ncustomer ",mynumber);
						Printf1((unsigned int)" is seated at table number",my_table);
						Release(printlock);
						Wait(tableCV[my_table],tablelock[my_table]);

/*********************************************
waiter interaction starts from here ....
***********************************************/


				Yield();						/* time given to eat food*/
				Acquire(printlock);
				Printf((unsigned int)"\nhad my soda on table ....byeeeeeee!!! !!!!!!!!!!!");

     			Printf1((unsigned int)"\nCustomer ",mynumber);
     					Printf((unsigned int )"already had his soda so no waiter");
     			Release(printlock);
				Acquire(customer_gonelock);
				customers_left=customers_left+1;

				Release(customer_gonelock);
				Acquire(printlock);
Release(printlock);

				Release(tablelock[my_table]);
				Exit();
		}



		}
	}

/**************************************************
CUSTOMER GIVING ORDER TO THE MANAGER
**************************************************/

	else
	{
		is_mang_takingorder1=NO;

		Release(customerlinelock);
		Acquire(manager_cust_orderlock);


		manager_customerno=mynumber;

#ifdef AUTO_ORDER
{
		manager_orderbook_obj.d6b= 1;
		manager_orderbook_obj.d3b= 1;
		manager_orderbook_obj.sb= 1;
		manager_orderbook_obj.fries= 1;
		manager_orderbook_obj.soda= 1;
	 w= 1;

	  if (w==0)
	  {
		  manager_orderbook_obj.eat_pref= TO_GO;
	  }
	  else
	  {
		  manager_orderbook_obj. eat_pref= GET_IN;

	  }
}
#endif

#ifdef MANUAL_ORDER
{
	  manager_orderbook_obj.d6b= customer_order_obj[mynumber].d6b;	/* placing random order*/
	  manager_orderbook_obj.d3b= customer_order_obj[mynumber].d3b;	/* placing random order*/
	  manager_orderbook_obj.sb= customer_order_obj[mynumber].sb;	/* placing random order*/
	  manager_orderbook_obj.fries= customer_order_obj[mynumber].fries;	/*placing random order */
	  manager_orderbook_obj.soda= customer_order_obj[mynumber].soda;	/*	placing random order*/
	  manager_orderbook_obj.eat_pref= customer_order_obj[mynumber].eat_pref;
}
#endif

Acquire(printlock);
Printf((unsigned int)"\n");


				Printf1((unsigned int)"\n customer ",mynumber);
						Printf1((unsigned int)" has ordered",manager_orderbook_obj.d6b);
						Printf((unsigned int )	 "6-dollar burger");

				Printf1((unsigned int)"\n customer ",mynumber);
				Printf1((unsigned int)" has ordered",manager_orderbook_obj.d3b);
				Printf((unsigned int )" 3-dollar burger");

				Printf1((unsigned int)"\n customer ",mynumber);
				Printf1((unsigned int)" has ordered",manager_orderbook_obj.sb);
				Printf((unsigned int )" veggie burger");

				Printf1((unsigned int)"\n customer ",mynumber);
								Printf1((unsigned int)" has ordered",manager_orderbook_obj.fries);
								Printf((unsigned int )" fries");

								Printf1((unsigned int)"\n customer ",mynumber);
																Printf1((unsigned int)" has ordered",manager_orderbook_obj.soda);
																Printf((unsigned int )" soda");


		 Printf((unsigned int)"\n");
		 Release(printlock);
		if(eat_pref[myordertaker]==GET_IN)
		{
			Acquire(printlock);
				Printf1((unsigned int)"\n customer ",mynumber);
						Printf((unsigned int )"chooses eat-in food");
				Release(printlock);
		}
			else
			{
				Acquire(printlock);
				Printf1((unsigned int)"\n customer  ",mynumber);
						Printf((unsigned int )"chooses to-go food\n");
				Release(printlock);
			}
		Acquire(printlock);
			Printf((unsigned int)"\n");
			Release(printlock);


		Signal(manager_cust_orderCV,manager_cust_orderlock);
		Wait(manager_cust_orderCV,manager_cust_orderlock);

		/* giving money*/


		Signal(manager_cust_orderCV,manager_cust_orderlock);
		Wait(manager_cust_orderCV,manager_cust_orderlock);


		if (manager_orderbook_obj.eat_pref == TO_GO)
		{
		       if(manager_orderbook_obj.mang_order_status_obj == ORDER_FINISH)
		       {
		    	   Acquire(printlock);
		    	   Printf1((unsigned int)"\ncustomer ", mynumber);
		    	  					   Printf((unsigned int )" receives food from the Manager");
			   Printf((unsigned int) "\nGOT MY TO_GO FOOD BYEEEEEEEEE!!!!!!!!!!");

			   Release(printlock);

			   Acquire(customer_gonelock);
						customers_left++;

				Release(customer_gonelock);

		       Release(manager_cust_orderlock);
		       Acquire(printlock);

			   Release(printlock);
			   Exit();        /*customer leaves the restaurant*/

		       }
		       else
		       {
		       customer_token_number[mynumber] =manager_orderbook_obj.token_number;
		       Acquire(printlock);
			   Printf1((unsigned int)"\nCustomer ",mynumber);
			   Printf1((unsigned int)"is given token number  by the Manager",customer_token_number[mynumber]);
			   Release(printlock);
		       Release(manager_cust_orderlock);

		       Acquire(togo_waitinglock);
		       Wait(togo_waitingCV,togo_waitinglock);

		       while(1)
				{

				if(customer_token_number[mynumber]==togo_food_ready_token)
				{

					Release(togo_waitinglock);
					Acquire(ordermantainlock);
					order_book_details[customer_token_number[mynumber]].order_stat= FOOD_GIVEN;
					Release(ordermantainlock);
					Acquire(printlock);
					Printf1((unsigned int)"\n customer", mynumber);
							Printf((unsigned int )" RECEIVES FOOD FROM ORDERTAKER\n");
					Release(printlock);
					Acquire(customer_gonelock);
					customers_left=customers_left+1;

					Release(customer_gonelock);
					Acquire(printlock);

Release(printlock);
Exit();
				}

					Wait(togo_waitingCV,togo_waitinglock);
				}

		       }
		}

		else if(manager_orderbook_obj.eat_pref == GET_IN)
		{
		      		       if(manager_orderbook_obj.mang_order_status_obj== ORDER_PENDING)
		       {

	            customer_token_number[mynumber]	= manager_orderbook_obj.token_number;

	            Acquire(printlock);
			Printf1((unsigned int)"\nCustomer ",mynumber);
					Printf1((unsigned int)" is given token number ",customer_token_number[mynumber]);
					Printf((unsigned int )"by the Manager");

			Release(printlock);
						/************************************************
							customer looking for a table to sit
						 ************************************************/

								Acquire(tablelinelock);

								 Release(manager_cust_orderlock);

								tableline=tableline+1;

								Wait(tablelineCV,tablelinelock);

								manager_customer_token= customer_token_number[mynumber];


								Release(tablelinelock);

								Acquire(manager_customer_tablelock);

								Signal(manager_customer_tableCV,manager_customer_tablelock);
								Acquire(printlock);
								Printf1((unsigned int)"\ncustomer  ",mynumber);
										Printf((unsigned int )"is waiting to sit on the table");
								Release(printlock);

								Wait(manager_customer_tableCV,manager_customer_tablelock);

								Release(manager_customer_tablelock);

								Acquire(ordermantainlock);

								 my_table= order_book_details[customer_token_number[mynumber]].table_number;

								Release(ordermantainlock);

								Acquire(tablelock[my_table]);
								Acquire(printlock);
								Printf1((unsigned int)"\ncustomer ",mynumber);
										Printf1((unsigned int)"is seated at table number ",my_table);
			Printf1((unsigned int)"\nCustomer",mynumber);
					Printf((unsigned int )" is waiting for the waiter to serve the food");
			Release(printlock);
								Wait(tableCV[my_table],tablelock[my_table]);

								Acquire(printlock);
								Printf1((unsigned int)"\nCustomer ",mynumber);
								Printf((unsigned int )" is served by waiter");
								Release(printlock);
								/*********************************************
									waiter interaction starts from here ....
									***********************************************/


											Yield();						/* time given to eat food*/

											Acquire(printlock);
											Printf1((unsigned int)"\nCustomer ",mynumber);
																			Printf((unsigned int )" had food by managr byeeeeeee!!!");
																		Release(printlock);
																		Acquire(customer_gonelock);
											customers_left++;

											Release(customer_gonelock);

											table_stat[my_table]= TABLE_FREE;

											Release(tablelock[my_table]);
											Acquire(printlock);

											Release(printlock);
											Exit();
		       }

		       else                                                    /* when only soda and get in*/
		       {

					customer_token_number[mynumber]	= manager_orderbook_obj.token_number;

				/************************************************
						 looking for a table to sit
				 ************************************************/

						Acquire(tablelinelock);

						 Release(manager_cust_orderlock);

						tableline=tableline+1;

						Wait(tablelineCV,tablelinelock);

						manager_customer_token= customer_token_number[mynumber];


						Release(tablelinelock);

						Acquire(manager_customer_tablelock);

						Signal(manager_customer_tableCV,manager_customer_tablelock);
						Acquire(printlock);
						Printf1((unsigned int)"\ncustomer ",mynumber);
								Printf((unsigned int )"is waiting to sit on the table");
						Release(printlock);
						Wait(manager_customer_tableCV,manager_customer_tablelock);

						Release(manager_customer_tablelock);

						Acquire(ordermantainlock);

						 my_table= order_book_details[customer_token_number[mynumber]].table_number;

						Release(ordermantainlock);

						Acquire(tablelock[my_table]);
						Acquire(printlock);
						Printf1((unsigned int)"\ncustomer ",mynumber);
								Printf1((unsigned int)" is seated at table number ",my_table);
						Release(printlock);
						Release(printlock);


						/*********************************************
							waiter interaction starts from here ....
							************************************************/


									Yield();						/* time given to eat food*/

									Acquire(printlock);
								Printf1((unsigned int)"\nCustomer",mynumber);
								Printf((unsigned int )"  already had his soda so no waiter");

								Release(printlock);
									Acquire(customer_gonelock);
									customers_left=customers_left+1;

									Release(customer_gonelock);

									table_stat[my_table]= TABLE_FREE;

									Release(tablelock[my_table]);
									Acquire(printlock);

									Release(printlock);
									Exit();


		       }



		}

	}
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////
								/////////////manager////////////
/////////////////////////////////////////////////////////////////////////////////////////////////*/

void manager (void)
{

int count=50;
int m,r,o,n,x,l,g,flag,iterations;
int cook_free;

while(1)

{
      /***********************************

              CHECKING WHEN TO FINISH THE MANAGER

       ************************************/

      Acquire(customer_gonelock);

      if(customers_left==CUSTOMERS)

      {
			Acquire(printlock);
            Printf((unsigned int)"\nMANAGER ENDS HERE!!!!!\n");
			Release(printlock);

            Release(customer_gonelock);
            Exit();
      }

      else
      {
            Release(customer_gonelock);
      }


/***********************************

        CODE FOR MANAGING THE COOK

************************************/
      Acquire(ordermantainlock);
      for( r=0;r<index_orderbook;r=r + 1)

      {
            if (order_book_details[r].order_stat == FOOD_PENDING)

            {

                  pending_order_obj.d6b = pending_order_obj.d6b + order_book_details[r].d6b;

                  pending_order_obj.d3b = pending_order_obj.d3b + order_book_details[r].d3b;

                  pending_order_obj.sb = pending_order_obj.sb + order_book_details[r].sb;

                  pending_order_obj.fries = pending_order_obj.fries + order_book_details[r].fries;

            }

      }

      Release(ordermantainlock);
            /* checking whether 6 dollar burger need to be made*/

      Acquire(total_foodlock);
            if(available_items_obj.d6b- pending_order_obj.d6b<5)

            {

                  Release(total_foodlock);

                  cook_free=0;

                  for( o=0; o<(COOKS + new_cooks); o= o+ 1)

                  {

                        if(cook_stat[o]==COOK_FREE)

                        {

                              cook_free=cook_free + 1;

                              cook_stat[o]=D6B;

                              Acquire(manager_cooklock);

                              Signal(manager_cookCV,manager_cooklock);

							  Acquire(printlock);
                              Printf1((unsigned int)"\nManager informs Cook ",o);
							  Printf((unsigned int)" to cook 6-dollar burger");
							  Release(printlock);

                              Release(manager_cooklock);

                              break;

                        }

                  }

                  if(cook_free==0)

                                          {

                                                /* hire a new cook*/



                                                new_cooks=new_cooks + 1;

                                                Fork(cook);

                                                Acquire(manager_cooklock);

                                                cook_stat[COOKS+new_cooks]=D6B;

			Acquire(printlock);
            Printf((unsigned int)"\nManager HIRES new Cook to cook 6-dollar burger");
			Release(printlock);


/*
  check this                                              t->Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
*/
                                                Release(manager_cooklock);
                                          }

            }

            else

            {

                  Release(total_foodlock);

            }

            /*checking whether 3 dollar burger need to be made*/

            Acquire(total_foodlock);

            if((available_items_obj.d3b- pending_order_obj.d3b) < 5)

                        {

                              Release(total_foodlock);

                              cook_free=0;

                              for(o=0; o<(COOKS + new_cooks); o= o+ 1)

                              {

                                    if(cook_stat[o]==COOK_FREE)

                                    {

                                          cook_free=cook_free + 1;

                                          cook_stat[o]=D3B;

                                          Acquire(manager_cooklock);

                                          Signal(manager_cookCV,manager_cooklock);

										  Acquire(printlock);
                                          Printf1((unsigned int)"\nManager informs Cook ",o);
										  Printf((unsigned int)" to cook 3-dollar burger");
										  Release(printlock);

                                          Release(manager_cooklock);

                                          break;

                                    }

                              }

                              if(cook_free==0)

                                                {

                                                      /* hire a new cook*/



                                                            new_cooks=new_cooks+1;

                                                            Fork(cook);

                                                            Acquire(manager_cooklock);

                                                            cook_stat[COOKS+new_cooks]=D3B;

															Acquire(printlock);
                                                            Printf((unsigned int)"\nManager HIRES new Cook to cook 3-dollar burger");
															Release(printlock);

/*
  check this                                              t.Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
*/

                                                            Release(manager_cooklock);
                                                      }

                        }

            else

                        {

                              Release(total_foodlock);

                        }

            /*checking whether veggie burger need to be made*/

            Acquire(total_foodlock);

            if((available_items_obj.sb- pending_order_obj.sb) < 5)

                        {

                              Release(total_foodlock);

                              cook_free=0;

                              for( o=0; o<(COOKS + new_cooks); o= o+ 1)

                              {

                                    if(cook_stat[o]==COOK_FREE)

                                    {

                                          cook_free=cook_free + 1;

                                          cook_stat[o]=SB;

                                          Acquire(manager_cooklock);

                                          Signal(manager_cookCV,manager_cooklock);

										  Acquire(printlock);
                                          Printf1((unsigned int)"\nManager informs Cook",o);
										  Printf((unsigned int)" to cook veggie burger");
										  Release(printlock);

                                          Release(manager_cooklock);

                                          break;

                                    }

                              }

                              if(cook_free==0)

                              {





                                    new_cooks=new_cooks + 1;

                                    Fork(cook);

                                   Acquire(manager_cooklock);

                                    cook_stat[COOKS+new_cooks]=SB;

									Acquire(printlock);
                                   Printf((unsigned int)"\nManager HIRES new Cook to cook veggie-burger");
								   Release(printlock);
/*
                                    t.Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
*/
                                   Release(manager_cooklock);

                            }

                        }

            else

                        {

                              Release(total_foodlock);

                        }

            /*checking whether fries need to be made*/
            Acquire(total_foodlock);

            if( (available_items_obj.fries - pending_order_obj.fries) <5)

                        {

                              Release(total_foodlock);

                              cook_free=0;

                              for( o=0; o<(COOKS + new_cooks); o=o + 1)

                              {

                                    if(cook_stat[o]==COOK_FREE)

                                    {

                                          cook_free=cook_free + 1;

                                          cook_stat[o]=FRIES;

                                          Acquire(manager_cooklock);

                                          Signal(manager_cookCV,manager_cooklock);

										  Acquire(printlock);
                                          Printf1((unsigned int)"\nManager informs Cook",o);
										  Printf((unsigned int)" to cook french fries");
										  Release(printlock);

                                          Release(manager_cooklock);

                                          break;

                                    }

                              }

                              if(cook_free==0)

                             {

                                /* hire a new cook*/



                               new_cooks=new_cooks+1;

                                Fork(cook);

                                Acquire(manager_cooklock);

                                cook_stat[COOKS+new_cooks]=FRIES;

								Acquire(printlock);
                                Printf((unsigned int)"\nManager HIRES new Cook to cook french fries");
								Release(printlock);
/*
                                t.Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
*/
                                Release(manager_cooklock);

                              }

                        }

            	else
                        {

                            Release(total_foodlock);

                        }

            /*check if item is in surplus---6 dollar burger*/

            Acquire(total_foodlock);
            if( (available_items_obj.d6b - pending_order_obj.d6b) >20)

                {
                    Release(total_foodlock);
                    for( n=0; n<(COOKS + new_cooks); n=n + 1)

                    {
                        if(cook_stat[n]==D6B)

							{

								Acquire(printlock);
                                Printf1((unsigned int)"\nManager sends Cook",n);
								Printf((unsigned int)" to break");
								Release(printlock);

                                cook_stat[n]=COOK_FREE;

                            }
                   }

                }

            else
            {
                Release(total_foodlock);
			}

            /*check if item is in surplus 3 dollar burger*/

            Acquire(total_foodlock);
            if( (available_items_obj.d3b - pending_order_obj.d3b) >20)

            {
                Release(total_foodlock);
                for( n=0; n<(COOKS + new_cooks); n=n + 1)
                {

                    if(cook_stat[n]==D3B)

                    {
						Acquire(printlock);
                                Printf1((unsigned int)"\nManager sends Cook",n);
								Printf((unsigned int)" to break");
						Release(printlock);

                        cook_stat[n]=COOK_FREE;

                    }

                }

            }

            else

            {
                Release(total_foodlock);
            }


            /*check if item is in surplus---veggie burger*/

            Acquire(total_foodlock);
            if( (available_items_obj.sb - pending_order_obj.sb) >20)

            {

                Release(total_foodlock);
               for( n=0; n<(COOKS + new_cooks); n=n + 1)
                {

                    if(cook_stat[n]==SB)

                    {
						Acquire(printlock);
                                Printf1((unsigned int)"\nManager sends Cook",n);
								Printf((unsigned int)" to break");
						Release(printlock);

                        cook_stat[n]=COOK_FREE;

                    }

                }

            }

            else
           {

                Release(total_foodlock);

           }


            /*check if item is in surplus---fries*/

            Acquire(total_foodlock);
            if( (available_items_obj.fries - pending_order_obj.fries) >20)

            {

                Release(total_foodlock);
                for( n=0; n<(COOKS + new_cooks); n=n + 1)

                {

                    if(cook_stat[n]==FRIES)

                    {
						Acquire(printlock);
                                Printf1((unsigned int)"\nManager sends Cook",n);
								Printf((unsigned int)" to break");
						Release(printlock);

                        cook_stat[n]=COOK_FREE;

                    }

                }

            }

            else

                        {

                              Release(total_foodlock);

                        }

Yield();
Yield();
Yield();



      /********************************************

       * CODE FOR TABLE MANAGEMENT

       ********************************************/

      Acquire(tablelinelock);     /*acquire eat-in line lock to check length of line*/

      if(tableline>0)       /*check if there are customers waiting for the tables*/

      {
			Acquire(printlock);
            Printf((unsigned int)"a customer waiting for a table!!!!!!!!!!\n");
			Release(printlock);

            for ( x=0;x<TABLES;x=x + 1)       /*check if there are any free tables available*/

      {

        Acquire(tablelock[x]);


        if(table_stat[x]==TABLE_FREE)

        {

            table_stat[x]==TABLE_OCCUPIED;

            Release(tablelock[x]);

            tableline = tableline -1;

            Signal(tablelineCV,tablelinelock);   /*signal the customer that a table has been assigned*/

            Acquire(manager_customer_tablelock);  /* acquire lock for interaction with customer*/

            Release(tablelinelock);
            Wait(manager_customer_tableCV,manager_customer_tablelock); /*wait for customer's token number so that entry can be updated in the main order book */

            Acquire(ordermantainlock);   /*Acquire main order book lock*/

            order_book_details[manager_customer_token].customer_seating_stat = GOT_SEAT; /*updating seating status of customer in order book */
            order_book_details[manager_customer_token].table_number = x;   /*assigning the free table to the customer*/

			Acquire(printlock);
            Printf1((unsigned int)"\nManager gives table ",x);
			Printf1((unsigned int)" to Customer ",order_book_details[manager_customer_token].customer_no);
			Release(printlock);

            Signal(manager_customer_tableCV,manager_customer_tablelock);


            Release(ordermantainlock);    /*Release main order book lock*/

            Release(manager_customer_tablelock);

           break;

        }

       else{
			  Acquire(printlock);
              Printf((unsigned int)"\nManager can't give table to Customer as no table free");
			  Release(printlock);

              Release(tablelinelock);

              Release(tablelock[x]);

              break;

           }
      }

      }
      else

      {
            Release(tablelinelock);
      }
      Yield();
      Yield();
      Yield();

	  /******************************************

            BAGGING THE FOOD WHEN REQUIRED

       ******************************************/

      Acquire(ordermantainlock);


      for( m=0;m<=index_orderbook;m=m + 1)
      {

        if(order_book_details[m].order_stat == FOOD_PENDING )
        {

            Acquire(total_foodlock);
            if((available_items_obj.d6b - order_book_details[m].d6b)>0 &&

            (available_items_obj.d3b - order_book_details[m].d3b)>0 &&

            (available_items_obj.sb - order_book_details[m].sb)>0 &&

            (available_items_obj.fries - order_book_details[m].fries)>0)

			{

                available_items_obj.d6b = available_items_obj.d6b - order_book_details[m].d6b;

                available_items_obj.d3b = available_items_obj.d3b - order_book_details[m].d3b;

                available_items_obj.sb = available_items_obj.sb - order_book_details[m].sb;

                available_items_obj.fries = available_items_obj.fries - order_book_details[m].fries;

                order_book_details[m].order_stat= FOOD_PACKED;

			   Acquire(printlock);
               Printf1((unsigned int)"\nManager packs food for customer ",order_book_details[m].customer_no);
			   Release(printlock);

            }

            Release(total_foodlock);

            }

            }

            Release(ordermantainlock);



      /*******************************************************************************

             DELIVER FOOD WHEN FOOD PACKED and

            1) TELL WAITER WHEN EAT_IN and CUSTOMER HAS A TABLE or

            2) TO_GO

      ********************************************************************************/
      Acquire(ordermantainlock);

      for( l=0;l<=index_orderbook;l=l + 1)

      {

            if(order_book_details[l].order_stat==FOOD_PACKED)

            {

                  if(order_book_details[l].eat_pref==TO_GO)

                  {

                        Acquire(togo_waitinglock);
                        togo_food_ready_token=l;

						Acquire(printlock);
                        Printf1((unsigned int)"Manager broadcasts token number:",l);
						Printf((unsigned int)" for customer\n");
						Release(printlock);

                        Broadcast(togo_waitingCV,togo_waitinglock);

                        Release(togo_waitinglock);
                  }

                  else     /* if eat in tell waiters if any food needs to be send at table*/

                  {

                        if(order_book_details[l].customer_seating_stat==GOT_SEAT)

                        {

                              for( g=0;g<WAITERS;g=g + 1)

                              {

                                    if(waiter_stat[g]==WAITER_FREE)

                                    {

                                          Acquire(waiterlock[g]);

                                          waiter_stat[g]=WAITER_BUSY;

                                          waiter_tokennumber[g]=l;

										  Acquire(printlock);
                                          Printf1((unsigned int)"\nManager wakes up waiter ",g);
                                          Printf1((unsigned int)"\nManager gives token number ",l);
										  Printf1((unsigned int)" to waiter ",g);
										  Printf1((unsigned int)" for customer ",order_book_details[l].customer_no);
										  Release(printlock);

                                          Signal(waiterCV[g],waiterlock[g]);

                                          Release(waiterlock[g]);

                                    }

                                    break;

                              }

                        }

                  }

            }

      }
      Release(ordermantainlock);


      /*********************************************

      ACTING AS ORDER TAKER WHEN ORDERTAKER'S BUSY

      ********************************************/

       flag=1;

      for( r=0; r<ORDERTAKERS;r=r + 1)

      {
            if (ordertakerstatus[r]==FREE)

            {

                  flag=0;
                  break;

            }

      }
      Acquire(customerlinelock);
      if((flag==1) && linelength>3)

      {
            is_mang_takingorder1=YES;

             iterations=2;

            iterations =iterations -1 ;

                  Signal(customerlineCV,customerlinelock);
                  linelength=linelength-1;

                        Acquire(manager_cust_orderlock);  /*Acquires talk lock so that customer and manager can talk mutually exclusively*/

                        Release(customerlinelock);
                        Wait(manager_cust_orderCV,manager_cust_orderlock);   /* waiting for order to be placed*/


                  /******************************************

                               * CALCULATING MONEY

                   *******************************************/
                  money_received_manager= (manager_orderbook_obj.d6b*6) + (manager_orderbook_obj.d3b*3) + (manager_orderbook_obj.sb*2)

                                          + (manager_orderbook_obj.fries*1) + (manager_orderbook_obj.soda*1);

                  Acquire(totalmoneylock);

                              totalmoney= totalmoney + money_received_manager;

                Release(totalmoneylock);

                Acquire(printlock);
				Printf1((unsigned int)"\nManager is taking order of Customer ",manager_customerno);
				Release(printlock);

				Signal(manager_cust_orderCV,manager_cust_orderlock);

				Wait(manager_cust_orderCV,manager_cust_orderlock);        /* waiting to get money from customer*/

				if ((manager_orderbook_obj.soda!=0)&&(manager_orderbook_obj.fries==0)&&(manager_orderbook_obj.d6b==0)&&

				(manager_orderbook_obj.d3b==0)&&(manager_orderbook_obj.sb==0))

                {

				/* decrease from inventory with lock*/

				manager_orderbook_obj.mang_order_status_obj= ORDER_FINISH;

				Acquire(printlock);
                Printf1((unsigned int)"\nManager gives soda to Customer ",manager_customerno);
				Release(printlock);

                Signal(manager_cust_orderCV,manager_cust_orderlock);
                Release(manager_cust_orderlock);

                }


                else if(manager_orderbook_obj.eat_pref==TO_GO)

                {

                    Acquire(total_foodlock);

                  if(( available_items_obj.d3b-manager_orderbook_obj.d3b>=0) && ( available_items_obj.d6b-manager_orderbook_obj.d6b>=0)
                   && ( available_items_obj.sb-manager_orderbook_obj.sb>=0) && ( available_items_obj.fries-manager_orderbook_obj.fries>=0))

                   {

                              /* decrease from inventory with lock */


                              available_items_obj.d3b=available_items_obj.d3b - manager_orderbook_obj.d3b;

                              available_items_obj.d6b=available_items_obj.d6b - manager_orderbook_obj.d6b;

                              available_items_obj.sb=available_items_obj.sb - manager_orderbook_obj.sb;

                              available_items_obj.fries=available_items_obj.fries - manager_orderbook_obj.fries;


                              Release(total_foodlock);

							  Acquire(printlock);
                              Printf1((unsigned int)"\nManager gives food to Customer ",manager_customerno);
							  Release(printlock);

                              manager_orderbook_obj.mang_order_status_obj=ORDER_FINISH;

                              Signal(manager_cust_orderCV,manager_cust_orderlock);

                              Release(manager_cust_orderlock);
                        }

                        else

                        {

                              Release(total_foodlock);

                              manager_orderbook_obj.mang_order_status_obj=ORDER_PENDING;

                              Signal(manager_cust_orderCV,manager_cust_orderlock);

                              Acquire(ordermantainlock); /* acquring lock to place order*/

                              index_orderbook=index_orderbook+1;

                              manager_orderbook_obj.token_number=index_orderbook;

                              Release(manager_cust_orderlock);
                              order_book_details[index_orderbook].d3b=manager_orderbook_obj.d3b; /* copying the order from order takers data structures*/

                              order_book_details[index_orderbook].d6b=manager_orderbook_obj.d6b; /* to the order book*/

                              order_book_details[index_orderbook].sb=manager_orderbook_obj.sb;

                              order_book_details[index_orderbook].fries=manager_orderbook_obj.fries;

                              order_book_details[index_orderbook].soda=manager_orderbook_obj.soda;

                              order_book_details[index_orderbook].eat_pref=manager_orderbook_obj.eat_pref;

                              order_book_details[index_orderbook].order_stat= FOOD_PENDING;

                              order_book_details[index_orderbook].customer_seating_stat= GOT_SEAT;

                              order_book_details[index_orderbook].customer_no= manager_customerno;

							  Acquire(printlock);
                              Printf1((unsigned int)"\nManager gives token number ",index_orderbook);
							  Printf1((unsigned int)" to Customer ",manager_customerno);
							  Release(printlock);

                              Release(ordermantainlock);


                        }

                  }

                  else if (manager_orderbook_obj.eat_pref == GET_IN)

                  {

                        manager_orderbook_obj.mang_order_status_obj=ORDER_PENDING;

                        Signal(manager_cust_orderCV,manager_cust_orderlock);


                        Acquire(ordermantainlock); /* acquring lock to place order*/

                        index_orderbook=index_orderbook + 1;

                        manager_orderbook_obj.token_number=index_orderbook;

                        Release(manager_cust_orderlock);


                        order_book_details[index_orderbook].d3b=manager_orderbook_obj.d3b;

                        order_book_details[index_orderbook].d6b=manager_orderbook_obj.d6b;

                        order_book_details[index_orderbook].sb=manager_orderbook_obj.sb;

                        order_book_details[index_orderbook].fries=manager_orderbook_obj.fries;

                        order_book_details[index_orderbook].soda=manager_orderbook_obj.soda;

                        order_book_details[index_orderbook].eat_pref=manager_orderbook_obj.eat_pref;

                        order_book_details[index_orderbook].order_stat= FOOD_PENDING;

                        order_book_details[index_orderbook].customer_seating_stat= NO_SEAT ;

                        order_book_details[index_orderbook].customer_no= manager_customerno;

						Acquire(printlock);
                        Printf1((unsigned int)"\nManager gives token number ",index_orderbook);
						Printf1((unsigned int)" to Customer ",manager_customerno);
						Release(printlock);


                        Release(ordermantainlock);


                  }

				  Acquire(customerlinelock);

            }

      else

      {

            Release(customerlinelock);

      }





/********************************************************

* MANAGING INVENTORY AND TOTAL MONEY

********************************************************* */

      Acquire(inventorylock);

      if(inventory_obj.d6b<20)

      {

            /*get total money lock*/

            Acquire(totalmoneylock);
            if(totalmoney==0)

            {

                  /*go to bank and withdraw money*/

                  totalmoney = totalmoney + 5000;

				  Acquire(printlock);
                  Printf1((unsigned int)"\nManager withdraws money from bank so total money now is ",totalmoney);
				  Release(printlock);

            }

                  totalmoney = totalmoney -50;

                  Release(totalmoneylock);

				  Acquire(printlock);
                  Printf((unsigned int)"\nManager orders inventory for 6-dollar burger");
				  Release(printlock);

                  Yield();

                  Yield();

                  Yield();

                  Yield();

                  Yield();


                  /*order raw materials*/

                  inventory_obj.d6b =inventory_obj.d6b + 50;

				  Acquire(printlock);
                  Printf((unsigned int)"\ninventory for 6-dollar burger is loaded into the restaurant");
				  Release(printlock);

      }


      else if(inventory_obj.d3b<20)

      {
      /*get total money lock*/

      Acquire(totalmoneylock);
      if(totalmoney==0)

      {

            /*go to bank and withdraw money*/

            totalmoney =totalmoney + 5000;

			Acquire(printlock);
            Printf1((unsigned int)"\nManager withdraws money from bank so total money now is ",totalmoney);
			Release(printlock);

      }
      /*no need to go to the bank*/
			Acquire(printlock);
            Printf((unsigned int)"\nManager orders inventory for 3-dollar burger");
			Release(printlock);

            totalmoney =totalmoney - 50;

            /*release total money lock*/

            Release(totalmoneylock);


			Yield();

            Yield();

            Yield();

            Yield();

            Yield();


            /*order raw materials*/

            inventory_obj.d3b =inventory_obj.d3b + 50;

		   Acquire(printlock);
           Printf((unsigned int)"\ninventory for 3-dollar burger is loaded into the restaurant");
		   Release(printlock);

      }
      else if(inventory_obj.sb<20)

      {

      /*get total money lock*/

      Acquire(totalmoneylock);
      if(totalmoney==0)

      {

            /*go to bank and withdraw money*/

            totalmoney =totalmoney + 5000;

			Acquire(printlock);
            Printf1((unsigned int)"\nManager withdraws money from bank so total money now is ",totalmoney);
			Release(printlock);

      }
      /*no need to go to the bank*/

			Acquire(printlock);
            Printf((unsigned int)"\nManager orders inventory for veggie burger");
			Release(printlock);

            totalmoney =totalmoney - 50;

            /*release total money lock*/

            Release(totalmoneylock);
            Yield();

            Yield();

            Yield();

            Yield();

            Yield();
            /*order raw materials*/

            inventory_obj.sb =inventory_obj.sb + 50;

			Acquire(printlock);
            Printf((unsigned int)"\ninventory for veggie burger is loaded into the restaurant");
			Release(printlock);

      }


      else if(inventory_obj.fries<20)

      {

      /*get total money lock*/

      Acquire(totalmoneylock);
      if(totalmoney==0)

      {

            /*go to bank and withdraw money*/

            totalmoney =totalmoney + 5000;

			Acquire(printlock);
            Printf1((unsigned int)"\nManager withdraws money from bank so total money now is ",totalmoney);
			Release(printlock);

      }
      /*no need to go to the bank*/

            totalmoney = totalmoney - 50;

			Acquire(printlock);
            Printf((unsigned int)"\nManager orders inventory for french fries");
			Release(printlock);

            /*release total money lock*/

            Release(totalmoneylock);
            Yield();

            Yield();

            Yield();

            Yield();

            Yield();
            /*order raw materials*/

            inventory_obj.fries =inventory_obj.fries + 50;

			Acquire(printlock);
            Printf((unsigned int)"\ninventory for fries is loaded into the restaurant");
			Release(printlock);

      }

      Release(inventorylock);
}


}

/*/////////////////////////////////////////////////////////////////////////////////////////////////
								/////////////cook////////////
/////////////////////////////////////////////////////////////////////////////////////////////////*/


void cook(void)

{


	int my_cook_number;
	Acquire(cook_numberlock);
	my_cook_number= glo_cook_count;
	glo_cook_count= glo_cook_count+1;
	Release(cook_numberlock);
      cook_stat[my_cook_number]=COOK_FREE;

      while(1)

      {
            while(cook_stat[my_cook_number]==D6B)

            {
				  Acquire(printlock);
                  Printf1((unsigned int)"\nCook  ",my_cook_number);
				  Printf((unsigned int)" is cooking 6-dollar burger ");
				  Release(printlock);

                  Acquire(inventorylock);

                  if(inventory_obj.d6b > 0)

                  {

                        inventory_obj.d6b = inventory_obj.d6b - 10;

                        Release(inventorylock);

                  }


                  else

                  {

                        cook_stat[my_cook_number]=COOK_FREE;

                        Release(inventorylock);

                        break;

                  }


                  Yield();     /* time taken to prepare food*/



                  Acquire(total_foodlock);

                  available_items_obj.d6b = available_items_obj.d6b +10;          /*prepare 10 food at a time*/

                  Release(total_foodlock);

                  Yield();
            }

            while(cook_stat[my_cook_number]==D3B)

                        {

				  Acquire(printlock);
                  Printf1((unsigned int)"\nCook ",my_cook_number);
				  Printf((unsigned int)" is cooking 3-dollar burger ");
				  Release(printlock);

                  Acquire(inventorylock);

                  if(inventory_obj.d3b > 0)

                                    {

                                          inventory_obj.d3b = inventory_obj.d3b -10;

                                          Release(inventorylock);

                                    }


                                    else

                                    {

                                          cook_stat[my_cook_number]=COOK_FREE;

                                          Release(inventorylock);

                                          break;

                                    }

                              Acquire(inventorylock);

                              inventory_obj.d3b = inventory_obj.d3b -1;

                              Release(inventorylock);
                              Yield();     /* time taken to prepare food*/



                              Acquire(total_foodlock);

                              available_items_obj.d3b = available_items_obj.d3b +10;

                              Release(total_foodlock);

                              Yield();

                        }

            while(cook_stat[my_cook_number]==SB)

                        {
							  Acquire(printlock);
                              Printf1((unsigned int)"\nCook ",my_cook_number);
							  Printf((unsigned int)" is cooking veggie burger ");
							  Release(printlock);
                              Acquire(inventorylock);

                                    if(inventory_obj.sb > 0)

                                    {

                                          inventory_obj.sb = inventory_obj.sb -10;

                                          Release(inventorylock);

                                    }


                                    else

                                    {

                                          cook_stat[my_cook_number]=COOK_FREE;

                                          Release(inventorylock);

                                          break;

                                    }

                              Acquire(inventorylock);

                              inventory_obj.sb= inventory_obj.sb -1;

                              Release(inventorylock);
                              Yield();     /* time taken to prepare food */



                              Acquire(total_foodlock);

                              available_items_obj.sb =available_items_obj.sb +10;

                              Release(total_foodlock);

                              Yield();

                        }

                        while(cook_stat[my_cook_number]==FRIES)

                        {
								    Acquire(printlock);
                                    Printf1((unsigned int)"\nCook ",my_cook_number);
									Printf((unsigned int)" is cooking french fries ");
									Release(printlock);

                                    Acquire(inventorylock);

                                    if(inventory_obj.fries > 0)

                                    {

                                          inventory_obj.fries = inventory_obj.fries - 10;

                                          Release(inventorylock);

                                    }


                                    else

                                    {

                                          cook_stat[my_cook_number]=COOK_FREE;

                                          Release(inventorylock);

                                          break;

                                    }

                              Acquire(inventorylock);

                              inventory_obj.fries = inventory_obj.fries -1;

                              Release(inventorylock);
                              Yield();     /* time taken to prepare food*/


                              Acquire(total_foodlock);

                              available_items_obj.fries = available_items_obj.fries +10;

                              Release(total_foodlock);

                              Yield();

                        }


            if(cook_stat[my_cook_number]==COOK_FREE)

            {

                  Acquire(manager_cooklock);

				  Acquire(printlock);
                  Printf1((unsigned int)"\nCook ",my_cook_number);
				  Printf((unsigned int)" is on break");
				  Release(printlock);

                  Wait(manager_cookCV,manager_cooklock);

				  Acquire(printlock);
                  Printf1((unsigned int)"\nCook ",my_cook_number);
				  Printf((unsigned int)" returned from break");
				  Release(printlock);

                  Release(manager_cooklock);

            }


      }



}

/*/////////////////////////////////////////////////////////////////////////////////////////////////
								/////////////waiter////////////
/////////////////////////////////////////////////////////////////////////////////////////////////*/


void waiter(void)

{

	int my_waiternumber;

	Acquire( waiter_numberlock);
	my_waiternumber=glo_waiter_count;
	glo_waiter_count=glo_waiter_count+1;
	Release( waiter_numberlock);

      while(1)

      {

            waiter_tokennumber[my_waiternumber]=-1;

            if(waiter_stat[my_waiternumber]==WAITER_FREE)

            {

                  Acquire(waiterlock[my_waiternumber]);

				  Acquire(printlock);
                  Printf1((unsigned int)"\nWaiter ",my_waiternumber);
				  Printf((unsigned int)" is going on break!!");
				  Release(printlock);

                  Wait(waiterCV[my_waiternumber],waiterlock[my_waiternumber]);  /*waiting for signal from either ordertaker or manager*/

				  Acquire(printlock);
                  Printf1((unsigned int)"\nWaiter ",my_waiternumber);
				  Printf((unsigned int)" returned from break!!");
				  Release(printlock);

				  Release(waiterlock[my_waiternumber]);
                  Yield();
                  Yield();
                  Acquire(ordermantainlock);

				  Acquire(printlock);
                  Printf1((unsigned int)"\nWaiter ",my_waiternumber);
				  Printf1((unsigned int)" got token number ",waiter_tokennumber[my_waiternumber]);
				  Printf1((unsigned int)" for Customer ",order_book_details[waiter_tokennumber[my_waiternumber]].customer_no);
				  Release(printlock);

				  tobe_service_tablenumber[my_waiternumber]= order_book_details[waiter_tokennumber[my_waiternumber]].table_number;
                  Acquire(tablelock[tobe_service_tablenumber[my_waiternumber]]);

				  Acquire(printlock);
                  Printf1((unsigned int)"\nWaiter ",my_waiternumber);
				  Printf1((unsigned int)" validates the token number for Customer ",order_book_details[waiter_tokennumber[my_waiternumber]].customer_no);
				  Release(printlock);

				  Signal(tableCV[tobe_service_tablenumber[my_waiternumber]],tablelock[tobe_service_tablenumber[my_waiternumber]]);

                  order_book_details[waiter_tokennumber[my_waiternumber]].order_stat= FOOD_GIVEN;

				  Acquire(printlock);
                  Printf1((unsigned int)"\nWaiter ",my_waiternumber);
				  Printf1((unsigned int)" serves food to Customer ",order_book_details[waiter_tokennumber[my_waiternumber]].customer_no);
				  Release(printlock);

Signal(tableCV[tobe_service_tablenumber[my_waiternumber]],tablelock[tobe_service_tablenumber[my_waiternumber]]);

                  Release(tablelock[tobe_service_tablenumber[my_waiternumber]]);

                  Release(ordermantainlock);
                  waiter_stat[my_waiternumber]=WAITER_FREE;



            }

      }

}


