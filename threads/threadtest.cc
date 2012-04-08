



#include "copyright.h"
#include "system.h"
#include "thread.h"
#include "list.h"
#include "synch.h"


/***********************************************
DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
************************************************/
	#define WAITERS 8
	#define ORDERTAKERS 8
	#define COOKS 8
	#define CUSTOMERS 30
	#define TABLES 5


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
	int customer_no[ORDERTAKERS];
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
							int customer_no;
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
									int customer_no;
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


void Problem2() {

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

	//initializing ordertaker's locks and condition variables
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

	//initializing tables locks and condition variables
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
	//printf("Before waiters\n");
		//initializing waiter's locks and condition variables
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


		//Initializing locks and condition variables..

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



	//initializing ordertaker's personal order book variables
	for(int i=0; i<ORDERTAKERS; i++)								// order takers personal order book
	{
	ordertakerstatus[i] = BUSY;
	d6b[i]=NULL;
	d3b[i]=NULL;
	sb[i]=NULL;
	soda[i]=NULL;
	fries[i]=NULL;
	customer_no[i]=NULL;

	}

	for (int z=0;z<COOKS;z++)						// initializing all cooks status to be free
	{
		cook_stat[z]=COOK_FREE;
	}

	//total available made food during start of our simulation
	available_items_obj->d3b=20;
	available_items_obj->d6b=20;
	available_items_obj->sb=20;
	available_items_obj->fries=20;

	//total pending orders during start of our simulation
	pending_order_obj->d3b=0;
	pending_order_obj->d6b=0;
	pending_order_obj->sb=0;
	pending_order_obj->fries=0;

	//total available food inventory during start of our simulation
	inventory_obj->d3b=100;
	inventory_obj->d6b=100;
	inventory_obj->sb=100;
	inventory_obj->fries=100;

	//variable to check whether ordertaker or manager is taking the order from customer
	is_mang_takingorder1= NO;

	//count to keep check of whether all customers left the restaurant or not
	customers_left=0;
	testcalls();

}



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

//function that calls all our test cases
void testcalls ()
{

 int input;

 printf("\n\n1 to Test Customers who wants to eat-in, must wait if the restaurant is full\n");
 printf("2 to Test OrderTaker/Manager gives order number to the Customer when the food is not ready\n");
 printf("3 to Test Customers who have chosent to eat-in, must leave after they have their food and Customers who have chosen to-go, must leave the restaurant after the OrderTaker/Manager has given the food\n");
 printf("4 to Test Manager maintains the track of food inventory. Inventory is refilled when it goes below order level\n");
 printf("5 to Test A Customer who orders only soda need not wait\n");
 printf("6 to Test The OrderTaker and the Manager both somethimes bag the food\n");
 printf("7 to Test Manager goes to the bank for cash when inventory is to be refilled and there is no cash in the restaurant\n");
 printf("8 to Test Cooks goes on break when informed by manager\n");
 printf("9 only soda & its get-in  then we have to show that customer gets soda but will wait for his seat\n");
 printf("10 manager assigns tables to eat-in customer\n");
 printf("11 customer has choosen to_go and food is available so its given instantly\n");
 printf("12 Manager forks new cook if all other cooks are busy\n");
 printf("13 when to_go food is ready its broadcasted by manager or ordertaker\n");
 printf("14 waiters will give food to the get_in customers whose token number has been obtained from manager /ordertaker\n");
 printf("15 when ordertakers are busy.... manager works as an ordertaker \n");
 printf("16 full simulation \n");


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



//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////



void test1()
{
#define MANUAL_ORDER

       printf("STARTING TEST 1 !!!!!!!!!!!!!!!!!!!!");

 customer_order_obj[0] ->d6b=rand () % 2;        // placing random order
 customer_order_obj [0]->d3b= rand () % 2;        // placing random order
 customer_order_obj [0]->sb= rand () % 2;         // placing random order
 customer_order_obj [0]->fries= rand () % 2;      // placing random order
 customer_order_obj [0]->soda= rand () % 2;       // placing random order
 customer_order_obj [0]->eat_pref=GET_IN;

 for (int x=0;x<TABLES;x++)             // Initializing All Tables as Occupied
 {
 table_stat[x]=TABLE_OCCUPIED;
 }

 Thread *t;

 t = new Thread("manager");
     t->Fork((VoidFunctionPtr)manager,0);

 t = new Thread("customer0");
  t->Fork((VoidFunctionPtr)customer,0);

 t = new Thread("ordertaker0");
  t->Fork((VoidFunctionPtr)ordertaker,0);

  t = new Thread("waiter");
    t->Fork((VoidFunctionPtr)waiter,0);

        t = new Thread("cook");
           t->Fork((VoidFunctionPtr)cook,0);


}

/////////////////////////////////////////////////////////////////////////////

void test2(void)
{

#define MANUAL_ORDER

       printf("STARTING TEST 2 !!!!!!!!!!!!!!!!!!!!");


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




       printf( "STARTING TEST 3 !!!!!!!!!!!!!!!!!!!!");

       customer_order_obj[0]->d6b =rand () % 2;        // placing random order
       customer_order_obj[0]-> d3b= rand () % 2;        // placing random order
       customer_order_obj[0]-> sb= rand () % 2;         // placing random order
       customer_order_obj[0]-> fries= rand () % 2;      // placing random order
       customer_order_obj[0]-> soda= rand () % 2;       // placing random order
       customer_order_obj[0]-> eat_pref= GET_IN;

       customer_order_obj[1]-> d6b =rand () % 2;        // placing random order
       customer_order_obj[1]->  d3b= rand () % 2;        // placing random order
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



       printf("STARTING TEST 4 !!!!!!!!!!!!!!!!!!!!");

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



       printf("STARTING TEST 5 !!!!!!!!!!!!!!!!!!!!");


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




       printf("STARTING TEST 6 !!!!!!!!!!!!!!!!!!!!");

				for(int a=0;a<6;a++)
				{
				 customer_order_obj[a]->d6b =0;//rand () % 2;        // placing random order
               customer_order_obj[a]-> d3b= 0;//rand () % 2;        // placing random order
               customer_order_obj[a]-> sb= 0;//rand () % 2;         // placing random order
               customer_order_obj[a]->fries=0;// rand () % 2;      // placing random order
               customer_order_obj[a]->soda= 1;//rand () % 2;       // placing random order

				customer_order_obj[a]-> eat_pref= GET_IN;
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

				 t = new Thread("customer5");
                 t->Fork((VoidFunctionPtr)customer,5);

                t = new Thread("ordertaker0");
                  t->Fork((VoidFunctionPtr)ordertaker,0);


                  t = new Thread("waiter0");
                                  t->Fork((VoidFunctionPtr)waiter,0);



       }

////////////////////////////////////////////////////////////////////////////

       void test7(void)
       {

#define MANUAL_ORDER

       printf("STARTING TEST 7 !!!!!!!!!!!!!!!!!!!!");
                inventory_obj->d3b=0;
                inventory_obj->d6b=0;
                inventory_obj->sb=0;
                inventory_obj->fries=0;

                       customer_order_obj[0]->d6b =rand () % 2;        // placing random order
                       customer_order_obj[0]-> d3b= rand () % 2;        // placing random order
                       customer_order_obj[0]-> sb= rand () % 2;         // placing random order
                       customer_order_obj[0]-> fries= rand () % 2;      // placing random order
                       customer_order_obj[0]-> soda= rand () % 2;       // placing random order
                int w= rand () % 2;                         // placing random order
                if (w==0)
                 {
                        customer_order_obj[0]-> eat_pref= TO_GO;
                 }
                else
                 {
                        customer_order_obj[0]-> eat_pref= GET_IN;
                 }

                totalmoney=0;

                Thread *t;
                t = new Thread("manager");
                  t->Fork((VoidFunctionPtr)manager,0);
                  t = new Thread("customer0");
                                  t->Fork((VoidFunctionPtr)customer,0);
                                t = new Thread("ordertaker0");
                                  t->Fork((VoidFunctionPtr)ordertaker,0);
								  t = new Thread("waiter0");
                                  t->Fork((VoidFunctionPtr)waiter,0);

       }

////////////////////////////////////////////////////////////////////////////

       void test8(void)
       {
#define MANUAL_ORDER



               printf("STARTING TEST 8 !!!!!!!!!!!!!!!!!!!!");
               available_items_obj->d3b=10;
                available_items_obj->d6b=10;
                available_items_obj->sb=10;
                available_items_obj->fries=1;

                Thread *t;

				  t = new Thread("cook");
                  t->Fork((VoidFunctionPtr)cook,0);

				  t = new Thread("manager");
                  t->Fork((VoidFunctionPtr)manager,0);

        }
////////////////////////////////////////////////////////////////////////////

       void test9(void)
       {

#define MANUAL_ORDER



               printf("STARTING TEST 9 !!!!!!!!!!!!!!!!!!!!");

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

               printf( "STARTING TEST 10 !!!!!!!!!!!!!!!!!!!!");

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

             #define MANUAL_ORDER


               printf( "STARTING TEST 11 !!!!!!!!!!!!!!!!!!!!");

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

               printf( "STARTING TEST 12 !!!!!!!!!!!!!!!!!!!!");

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

               printf( "STARTING TEST 13 !!!!!!!!!!!!!!!!!!!!");

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

               printf( "STARTING TEST 14 !!!!!!!!!!!!!!!!!!!!");


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

               printf( "STARTING TEST 15 !!!!!!!!!!!!!!!!!!!!");



                Thread *t;
                 for(int f=0;f<9;f++)
                           {
                               char* name2;
                               name2=  (char*) "customer";

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


for(int f=0;f<30;f++)
{
				customer_order_obj[f]->d6b =rand () % 2;        // placing random order
               customer_order_obj[f]-> d3b= rand () % 2;        // placing random order
               customer_order_obj[f]-> sb= rand () % 2;         // placing random order
               customer_order_obj[f]-> fries= rand () % 2;      // placing random order
               customer_order_obj[f]-> soda= rand () % 2;       // placing random order
                int w= rand () % 2;        // placing random order
                 if (w==0)
                 {
                         customer_order_obj[f]-> eat_pref= TO_GO;
                 }
                else
                 {
                        customer_order_obj[f]->eat_pref= GET_IN;
                 }
}


				 printf( "\nSTARTING FULL_SIMULATION");
				 printf( "\nNO. OF CUSTOMERS:[30]");
				 printf( "\nNO. OF WAITERS:[5] ");
				 printf( "\nNO. OF ORDERTAKERS:[5]");
				 printf( "\nNO. OF COOKS:[5]");
				 printf("\n");

                Thread *t;

				//forking 30 CUSTOMERS
                for(int f=0;f<30;f++)
                {
                    char* name2;

                    name2=  (char*) "customer";

                    t = new Thread(name2);
                    t->Fork((VoidFunctionPtr)customer,f);
                }

				//forking 5 ordertakers
                 for(int f=0;f<5;f++)
                 {
                    char* name2;
                    name2=  (char*) "ordertaker";

                    t = new Thread(name2);
                    t->Fork((VoidFunctionPtr)ordertaker,f);
                }

				//forking 5 waiters
                 for(int f=0;f<5;f++)
                 {
                  char* name2;
                  name2=  (char*) "waiter";

                  t = new Thread(name2);
                  t->Fork((VoidFunctionPtr)waiter,f);
                  }

				//forking 5 cooks
                 for(int f=0;f<5;f++)
                 {
                    char* name2;
                    name2=  (char*) "cook";

                    t = new Thread(name2);
                    t->Fork((VoidFunctionPtr)cook,f);
                 }

				 ////forking manager
                t = new Thread("manager");
                  t->Fork((VoidFunctionPtr)manager,0);



       }




//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


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
			DEBUG('t',"endinggggggggggggggggg OTTTTTTTTTTTT!!!!!");
			printf("\nORDERTAKER ENDS HERE!!!\n");
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

	//while there are customers in the line

	while(linelength>0)

	{
		if(is_mang_takingorder1==NO)                  //check whether manager is going to interact with next customer
		{
		customerlineCV->Signal(customerlinelock) ;
		linelength--;								  //take 1st customer out from the line
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

	printf("\nOrderTaker [%d] is taking order of Customer[%d]",my_otnumber,ot_customerno[my_otnumber]);

	DEBUG('t',"CALCULATING MONEY and give bill!!!!!!!!!!!!!!!!!!!!!");

	customerordertakerCV[my_otnumber]->Signal(customerordertakerlock[my_otnumber]);
	customerordertakerCV[my_otnumber]->Wait(customerordertakerlock[my_otnumber]);				// waiting to get money from customer

	DEBUG('t',"OT checking for order details!!!!!!!!!!!!!!!!!!!!!!");

	//when only soda is ordered
	if ((soda[my_otnumber]!=0)&&(fries[my_otnumber]==0)&&(d6b[my_otnumber]==0)&&(d3b[my_otnumber]==0)&&(sb[my_otnumber]==0))
	{


		status[my_otnumber]= FINISH;

		printf("\nOrderTaker [%d] gives soda to Customer[%d]",my_otnumber,ot_customerno[my_otnumber]);
		customerordertakerCV[my_otnumber]->Signal(customerordertakerlock[my_otnumber]);

		customerordertakerlock[my_otnumber]->Release();
	}


	else if(eat_pref[my_otnumber]==TO_GO)
	{
		total_foodlock->Acquire();

		//check whether to_go food can be given instantly
		if(( available_items_obj->d3b-d3b[my_otnumber]>=0) && ( available_items_obj->d6b-d6b[my_otnumber]>=0)
				&& ( available_items_obj->sb-sb[my_otnumber]>=0) && ( available_items_obj->fries-fries[my_otnumber]>=0))	// calculate if food is ready
		{

			// decrease ordered food from total food

			available_items_obj->d3b-= d3b[my_otnumber];
			available_items_obj->d6b-= d6b[my_otnumber];
			available_items_obj->sb-= sb[my_otnumber];
			available_items_obj->fries-= fries[my_otnumber];

			total_foodlock->Release();


			DEBUG('t',"to go ready food given to the customer!!!!!!!!!!!!!!!");

			printf("\nOrderTaker [%d] gives food to Customer[%d]",my_otnumber,ot_customerno[my_otnumber]);

			status[my_otnumber]=FINISH;
			customerordertakerCV[my_otnumber]->Signal(customerordertakerlock[my_otnumber]);

			customerordertakerlock[my_otnumber]->Release();

		}


	else //when food is not ready...token is given
	{
		total_foodlock->Release();

		status[my_otnumber]=PENDING;

		customerordertakerCV[my_otnumber]->Signal(customerordertakerlock[my_otnumber]);


		ordermantainlock->Acquire();	// acquring lock to place order

		index_orderbook++;
		token_number[my_otnumber]=index_orderbook;
		customerordertakerlock[my_otnumber]->Release();

		order_book_details[index_orderbook]->d3b=d3b[my_otnumber];	// copying the order from order takers data structures
		order_book_details[index_orderbook]->d6b=d6b[my_otnumber];	// to the order book
		order_book_details[index_orderbook]->sb=sb[my_otnumber];                      //
		order_book_details[index_orderbook]->fries=fries[my_otnumber];                //   ----- do -------
		order_book_details[index_orderbook]->soda=soda[my_otnumber];                  //
		order_book_details[index_orderbook]->eat_pref=eat_pref[my_otnumber];	//
		order_book_details[index_orderbook]->order_stat= FOOD_PENDING;	//	----do-------
		order_book_details[index_orderbook]->customer_seating_stat= GOT_SEAT;
		order_book_details[index_orderbook]->customer_no= ot_customerno[my_otnumber];


		printf("\nOrderTaker [%d] gives token number[%d] to Customer[%d]",my_otnumber,index_orderbook,ot_customerno[my_otnumber]);

		ordermantainlock->Release();



	}

}

//when customer prefers eat-in
else if (eat_pref[my_otnumber] == GET_IN)
{


	status[my_otnumber]=PENDING;
	DEBUG('t',"OT gives pending status to customer");
	customerordertakerCV[my_otnumber]->Signal(customerordertakerlock[my_otnumber]);

	ordermantainlock->Acquire();	// acquring lock to place order

	index_orderbook++;
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

	printf("\nOrderTaker [%d] gives token number[%d] to Customer[%d]",my_otnumber,index_orderbook,ot_customerno[my_otnumber]);

	DEBUG('t',"order book details are:%d%d%d%d",order_book_details[index_orderbook]->d3b,order_book_details[index_orderbook]->d6b,
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
			 checking if any food can be bagged
		 *****************************************/
		 ordermantainlock->Acquire();

		//setting ordertaker's status to busy
		 ordertakerstatus[my_otnumber]=BUSY;
		 customerlinelock->Release();

		 //traversing the order book and checking whether any order can be packed

		 for(int m=0;m<=index_orderbook;m++)
		 {
			 if(order_book_details[m]->order_stat == FOOD_PENDING )
			 {
				int ci=order_book_details[m]->customer_no;
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

					printf("\nOrderTaker [%d] has packed the food for Customer[%d]",my_otnumber,ci);
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

				//if its to_go and food is packed broadcast the token number of customer

				if(order_book_details[l]->eat_pref==TO_GO)
				{
					togo_waitinglock->Acquire();

					togo_food_ready_token=l;

					togo_waitingCV->Broadcast(togo_waitinglock);
					togo_waitinglock->Release();

				}

				else	// when eat in tell waiter to deliver the food if customer has got the table
				{
					if(order_book_details[l]->customer_seating_stat==GOT_SEAT)
					{
						for(int g=0;g<WAITERS;g++)
						{
							if(waiter_stat[g]==WAITER_FREE)
							{
								DEBUG('t',"OT giving food to waiter !!!!!!!!!!!!!!!");
								waiterlock[g]->Acquire();
								waiter_stat[g]=WAITER_BUSY;
								waiter_tokennumber[g]=l;

								printf("OrderTaker [%d] gives Token number [%d] to Waiter [%d] for Customer[%d]",my_otnumber,l,g,order_book_details[l]->customer_no);
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
				DEBUG('t',"COMMENT: status set to free");
		//		currentThread->Finish();

		}

currentThread->Yield();

currentThread->Yield();

	}		// end while(1)



}  // end ot function




//////////////////////////////////////////////////////////////////////////////////////////////////
								/////////////CUSTOMER////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void customer( int mynumber)
{

int myordertaker;                                //variable to store id of customer's order-taker
customerlinelock->Acquire();
linelength++;									 //customer gets in line as soon as they enter the restaurant
customerlineCV->Wait(customerlinelock);

if( is_mang_takingorder1 == NO)                   //check if customer has got signal from manager

{

	for(int m=0;m<ORDERTAKERS;m++)                //checks which ordertaker is waiting for customer
	{
		if(ordertakerstatus[m]==WAIT)
		{
			DEBUG('t',"COMMENT: customer [%d] looking for a order taker", mynumber);
			myordertaker=m;
			ordertakerstatus[m]=BUSY;              //customer makes the ordertaker busy who is waiting initially
			break;
		}
	}
	customerlinelock->Release();
	customerordertakerlock[myordertaker]->Acquire();


	DEBUG('t',"COMMENT: customer [%d] is placing order", mynumber);

	ot_customerno[myordertaker]= mynumber;

	//placing the order
#ifdef AUTO_ORDER
 {
	d6b[myordertaker]= rand () % 2 +1;	// placing random order
	d3b[myordertaker]= rand () % 2 +1;	//  placing random order
	sb[myordertaker]= rand () % 2 +1;	// placing random order
	fries[myordertaker]= rand () % 2 +1;	// placing random order
	soda[myordertaker]= rand () % 2 +1;	//	placing random order

	int w= rand () % 2;				// placing random order


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
	    d6b[myordertaker]= customer_order_obj[mynumber]->d6b;	// placing random order
		d3b[myordertaker]= customer_order_obj[mynumber]->d3b;	//  placing random order
		sb[myordertaker]= customer_order_obj[mynumber]->sb;	// placing random order
		fries[myordertaker]= customer_order_obj[mynumber]->fries;	// placing random order
		soda[myordertaker]= customer_order_obj[mynumber]->soda;	//	placing random order
		eat_pref[myordertaker]= customer_order_obj[mynumber]->eat_pref;


}
#endif



	  DEBUG('t', "/n THE customer %d ORDER IS:::::::::::::::::::::::::::/n 6 $ BURGER:%d,3 $ BURGER: %d, /n simple BURGER:%d, /n  FRIES:%d, /n  soda%d "
			  "eat_pref:%d", mynumber, d6b[myordertaker],d3b[myordertaker],sb[myordertaker], fries[myordertaker], soda[myordertaker],eat_pref[myordertaker]);

		printf("\n");
		DEBUG('t', "\ncustomer [%d] is giving order to ordertaker[%d]", mynumber,myordertaker);

				printf("\n customer [%d] has ordered[%d] 6-dollar burger",mynumber,d6b[myordertaker]);

				printf("\n customer [%d] has ordered[%d] 3-dollar burger",mynumber,d3b[myordertaker]);

				printf("\n customer [%d] has ordered[%d] veggie burger",mynumber,sb[myordertaker]);

				printf("\n customer [%d] has ordered[%d] french fries",mynumber,fries[myordertaker]);

				printf("\n customer [%d] has ordered[%d] soda",mynumber,soda[myordertaker]);


		 printf("\n");
			if(eat_pref[myordertaker]==GET_IN)
		{
				printf("\n customer [%d] chooses eat-in food",mynumber);
		}
			else
			{
				printf("\n customer [%d] chooses to-go food\n",mynumber);
			}
			printf("\n");


    customerordertakerCV[myordertaker]->Signal(customerordertakerlock[myordertaker]);
	customerordertakerCV[myordertaker]->Wait(customerordertakerlock[myordertaker]);

	// giving money

	DEBUG('t',"COMMENT:customer %d giving money !!!!!!!!!!!!", mynumber);

	//waiting for either  food or token

	customerordertakerCV[myordertaker]->Signal(customerordertakerlock[myordertaker]);
	customerordertakerCV[myordertaker]->Wait(customerordertakerlock[myordertaker]);


	if (eat_pref[myordertaker] == TO_GO)
	{
		if(status[myordertaker]==FINISH)
		{


		printf("\ncustomer [%d] receives food from the OrderTaker[%d]", mynumber,myordertaker);

		printf("\ncustomer [%d] GOT TO_GO READY FOOD ......byeeeeeeeeeee", mynumber);
		customer_gonelock->Acquire();
		customers_left++;

		customer_gonelock->Release();
printf("\n     %d            [%d]              byeeeeeee!!! !!!!!!!!!!!\n",customers_left,mynumber);
		customerordertakerlock[myordertaker]->Release();
		currentThread->Finish();	// customer leaves the restaurant


		}
		else
		{
		customer_token_number[mynumber]	= token_number[myordertaker];
		DEBUG('t'," customer %dTAKING TOKEN NUMBER FROM OT!!!!!!!!!!",mynumber);

		printf("\nCustomer [%d] is given token number [%d] by the OrderTaker [%d]",mynumber,customer_token_number[mynumber],myordertaker);
		//customerordertakerCV[myordertaker]->Signal(customerordertakerlock[myordertaker]);
		customerordertakerlock[myordertaker]->Release();

		togo_waitinglock->Acquire();
		printf(" \ncustomer [%d] waiting to be get his number broadcasted !!!\n", mynumber);
		togo_waitingCV->Wait(togo_waitinglock);				// wait for token no to be broadcasted


		while(1)
		{

		if(customer_token_number[mynumber]==togo_food_ready_token)
		{

			togo_waitinglock->Release();
			ordermantainlock->Acquire();
			order_book_details[customer_token_number[mynumber]]->order_stat= FOOD_GIVEN;
			ordermantainlock->Release();


			printf("\n customer [%d] RECEIVES FOOD FROM ORDERTAKER\n", mynumber);
			printf("\n customer %d GOT MY TO GO FOOD NOW I AM LEAVING  ..............", mynumber);
			customer_gonelock->Acquire();

			customers_left++;

			customer_gonelock->Release();
printf("\n     %d            [%d]              byeeeeeee!!! !!!!!!!!!!!\n",customers_left,mynumber);
			currentThread->Finish();
		}

			togo_waitingCV->Wait(togo_waitinglock);
		}

		}
	}

	//when the customer has choosen eat_in
	else if(eat_pref[myordertaker] == GET_IN)
	{

		if(status[myordertaker]==PENDING)
		{
			customer_token_number[mynumber]	= token_number[myordertaker];
			DEBUG('t',"customer %d TAKING TOKEN NUMBER FROM OT!!!!!!!!!!", mynumber);
			printf("\nCustomer [%d] is given token number [%d] by the OrderTaker [%d]",mynumber,customer_token_number[mynumber],myordertaker);

	/************************************************
	  cutomer looking for a table to sit
	 ************************************************/

			printf("\ncustomer [%d] looking for free table !!!", mynumber);
			tablelinelock->Acquire();


			customerordertakerlock[myordertaker]->Release();

			tableline++;

			tablelineCV->Wait(tablelinelock);          //wait for manager to wake him when he is assigned a table

			manager_customer_token= customer_token_number[mynumber];

			tablelinelock->Release();

			manager_customer_tablelock->Acquire();

			manager_customer_tableCV->Signal(manager_customer_tablelock);
			printf("\ncustomer [%d] is waiting to sit on the table",mynumber);
			manager_customer_tableCV->Wait(manager_customer_tablelock);

			manager_customer_tablelock->Release();

			ordermantainlock->Acquire();

			//customer finds which table has been assigned to him

			int my_table= order_book_details[customer_token_number[mynumber]]->table_number;

			ordermantainlock->Release();

			tablelock[my_table]->Acquire();

			printf("\ncustomer [%d] is seated at table number [%d]",mynumber,my_table);
			printf("\nCustomer [%d] is waiting for the waiter to serve the food",mynumber);

			tableCV[my_table]->Wait(tablelock[my_table]);
printf("\n   HAVING MY FOOD   %d      !!!!!!!!!!!!!\n",mynumber);
			currentThread->Yield();						// time given to eat food

			printf("\n customer [%d] had my food on table ....byeeeeeee!!! ", mynumber);
			printf("\nCustomer [%d] is served by waiter",mynumber);

			customer_gonelock->Acquire();
			customers_left++;

			customer_gonelock->Release();
printf("\n     %d            [%d]              byeeeeeee!!! !!!!!!!!!!!\n",customers_left,mynumber);
			table_stat[my_table]= TABLE_FREE;          //set table status to free when leaving

			tablelock[my_table]->Release();

			currentThread->Finish();					// only after food is delivered

		}

		else									// when only soda and get in
		{

			customer_token_number[mynumber]	= token_number[myordertaker];
			DEBUG('t',"customer %d TAKING TOKEN NUMBER FROM OT!!!!!!!!!!",mynumber);

			printf("\nCustomer [%d] is given token number [%d] by the OrderTaker [%d]",mynumber,customer_token_number[mynumber],myordertaker);

			/************************************************
     		 looking for a table to sit
	    	 ************************************************/

						printf("\n customer [%d] looking for free table !!!",mynumber);

						tablelinelock->Acquire();

						customerordertakerlock[myordertaker]->Release();

						tableline++;

						tablelineCV->Wait(tablelinelock);

						manager_customer_token= customer_token_number[mynumber];


						tablelinelock->Release();

						manager_customer_tablelock->Acquire();

						manager_customer_tableCV->Signal(manager_customer_tablelock);

						printf("\ncustomer [%d] is waiting to sit on the table",mynumber);

						manager_customer_tableCV->Wait(manager_customer_tablelock);

						manager_customer_tablelock->Release();

						ordermantainlock->Acquire();

						int my_table= order_book_details[customer_token_number[mynumber]]->table_number;

						ordermantainlock->Release();

						tablelock[my_table]->Acquire();

						printf("\ncustomer [%d] is seated at table number [%d]",mynumber,my_table);

						tableCV[my_table]->Wait(tablelock[my_table]);

//*********************************************
//waiter interaction starts from here ....
//***********************************************


				currentThread->Yield();						// time given to eat food

				printf("\nhad my soda on table ....byeeeeeee!!! !!!!!!!!!!!");

     			printf("\nCustomer [%d] already had his soda so no waiter",mynumber);
				customer_gonelock->Acquire();
				customers_left++;

				customer_gonelock->Release();
printf("\n     %d            [%d]              byeeeeeee!!! !!!!!!!!!!!\n",customers_left,mynumber);				table_stat[my_table]= TABLE_FREE;


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


		DEBUG('t',"COMMENT: customer %d inside place order",mynumber);

		manager_customerno=mynumber;

#ifdef AUTO_ORDER
{
		manager_orderbook_obj->d6b= rand () % 2 +1;	// placing random order
		manager_orderbook_obj->d3b= rand () % 2 +1;	//  placing random order
		manager_orderbook_obj->sb= rand () % 2 +1;	// placing random order
		manager_orderbook_obj->fries= rand () % 2 +1;	// placing random order
		manager_orderbook_obj->soda= rand () % 2 +1;	//	placing random order
	int w= rand () % 2;				// placing random order

	  if (w==0)
	  {
		  manager_orderbook_obj->eat_pref= TO_GO;
	  }
	  else
	  {
		  manager_orderbook_obj-> eat_pref= GET_IN;

	  }
}
#endif

#ifdef MANUAL_ORDER
{
	  manager_orderbook_obj->d6b= customer_order_obj[mynumber]->d6b;	// placing random order
	  manager_orderbook_obj->d3b= customer_order_obj[mynumber]->d3b;	//  placing random order
	  manager_orderbook_obj->sb= customer_order_obj[mynumber]->sb;	// placing random order
	  manager_orderbook_obj->fries= customer_order_obj[mynumber]->fries;	// placing random order
	  manager_orderbook_obj->soda= customer_order_obj[mynumber]->soda;	//	placing random order
	  manager_orderbook_obj->eat_pref= customer_order_obj[mynumber]->eat_pref;
}
#endif

		DEBUG('t', "/n customer %d THE ORDER to maanger IS:::::::::::::::::::::::::::/n 6 $ BURGER:%d,3 $ BURGER: %d, /n simple BURGER:%d, /n  FRIES:%d, /n  soda%d "
		                 "eat_pref",mynumber,manager_orderbook_obj->d6b,manager_orderbook_obj->d3b,manager_orderbook_obj->sb,manager_orderbook_obj-> fries,manager_orderbook_obj-> soda);

		printf("\n");
		printf("\nCustomer [%d] is giving order to the Manager\n",mynumber);


				printf("\n customer [%d] has ordered[%d] 6-dollar burger",mynumber,manager_orderbook_obj->d6b);

				printf("\n customer [%d] has ordered[%d] 3-dollar burger",mynumber,manager_orderbook_obj->d3b);

				printf("\n customer [%d] has ordered[%d] veggie burger",mynumber,manager_orderbook_obj->sb);

				printf("\n customer [%d] has ordered[%d] french fries",mynumber,manager_orderbook_obj->fries);

				printf("\n customer [%d] has ordered[%d] soda",mynumber,manager_orderbook_obj->soda);


		 printf("\n");
		if(eat_pref[myordertaker]==GET_IN)
		{
				printf("\n customer [%d] chooses eat-in food",mynumber);
			}
			else
			{
				printf("\n customer [%d] chooses to-go food\n",mynumber);
			}
			printf("\n");


		DEBUG('t',"customer %d now waiting for the bill",mynumber);
		manager_cust_orderCV->Signal(manager_cust_orderlock);
		manager_cust_orderCV->Wait(manager_cust_orderlock);

		// giving money

		DEBUG('t',"COMMENT: customer %d giving money to manager !!!!!!!!!!!!!!", mynumber);

		manager_cust_orderCV->Signal(manager_cust_orderlock);
		manager_cust_orderCV->Wait(manager_cust_orderlock);

		DEBUG('t'," customer %d reached 111!!!!!!!!!!!!!!!", mynumber);

		if (manager_orderbook_obj->eat_pref == TO_GO)
		{
		       if(manager_orderbook_obj->mang_order_status_obj == ORDER_FINISH)
		       {
			   printf( "\nGOT MY TO_GO FOOD BYEEEEEEEEE!!!!!!!!!!");
			   printf("\ncustomer [%d] receives food from the Manager", mynumber);

			   customer_gonelock->Acquire();
						customers_left++;

				customer_gonelock->Release();

		       manager_cust_orderlock->Release();
			   printf("\n     %d            [%d]              byeeeeeee!!! !!!!!!!!!!!\n",customers_left,mynumber);		       currentThread->Finish();        // customer leaves the restaurant

		       }
		       else
		       {
		       customer_token_number[mynumber] =manager_orderbook_obj->token_number;
		       DEBUG('t'," customer %d TAKING TOKEN NUMBER FROM managerrrrrr!!!!!!!!!!", mynumber);

			   printf("\nCustomer [%d] is given token number [%d] by the Manager",mynumber,customer_token_number[mynumber]);

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

					DEBUG('t'," customer %d IGOT MY TO GO FOOD < NOW M LEAVING  ..............", mynumber);

					printf("\n customer [%d] RECEIVES FOOD FROM ORDERTAKER\n", mynumber);
					customer_gonelock->Acquire();
					customers_left++;

					customer_gonelock->Release();
printf("\n     %d            [%d]              byeeeeeee!!! !!!!!!!!!!!\n",customers_left,mynumber);					currentThread->Finish();
				}

					togo_waitingCV->Wait(togo_waitinglock);
				}

		       }
		}

		else if(manager_orderbook_obj->eat_pref == GET_IN)
		{
		       DEBUG('t',"customer %d before IF !!!!!!!!!!!!!!!",mynumber);
		       if(manager_orderbook_obj->mang_order_status_obj== ORDER_PENDING)
		       {

	            customer_token_number[mynumber]	= manager_orderbook_obj->token_number;
				DEBUG('t'," customer %d TAKING TOKEN NUMBER FROM managerr!!!!!!!!!!", mynumber);

			printf("\nCustomer [%d] is given token number [%d] by the Manager",mynumber,customer_token_number[mynumber]);


						/************************************************
							customer looking for a table to sit
						 ************************************************/

								DEBUG('t'," customer %d looking for free table !!!!!!!!!!!!", mynumber);

								tablelinelock->Acquire();

								 manager_cust_orderlock->Release();

								tableline++;

								tablelineCV->Wait(tablelinelock);

								manager_customer_token= customer_token_number[mynumber];


								tablelinelock->Release();

								manager_customer_tablelock->Acquire();

								manager_customer_tableCV->Signal(manager_customer_tablelock);
								printf("\ncustomer [%d] is waiting to sit on the table",mynumber);
								manager_customer_tableCV->Wait(manager_customer_tablelock);

								manager_customer_tablelock->Release();

								ordermantainlock->Acquire();

								int my_table= order_book_details[customer_token_number[mynumber]]->table_number;

								ordermantainlock->Release();

								tablelock[my_table]->Acquire();

								printf("\ncustomer [%d] is seated at table number [%d]",mynumber,my_table);
			printf("\nCustomer [%d] is waiting for the waiter to serve the food",mynumber);

								tableCV[my_table]->Wait(tablelock[my_table]);

								//*********************************************
									//waiter interaction starts from here ....
									//***********************************************


											currentThread->Yield();						// time given to eat food

											DEBUG('t'," customer %d had my food ....byeeeeeee!!! !!!!!!!!!!!",mynumber);
								printf("\nCustomer [%d] is served by waiter",mynumber);
											customer_gonelock->Acquire();
											customers_left++;

											customer_gonelock->Release();

											table_stat[my_table]= TABLE_FREE;

											tablelock[my_table]->Release();
printf("\n     %d            [%d]              byeeeeeee!!! !!!!!!!!!!!\n",customers_left,mynumber);											currentThread->Finish();
		       }

		       else                                                    // when only soda and get in
		       {

					customer_token_number[mynumber]	= manager_orderbook_obj->token_number;
						DEBUG('t',"customer %d TAKING TOKEN NUMBER FROM managerrrrrr!!!!!!!!!!", mynumber);

				/************************************************
						 looking for a table to sit
				 ************************************************/

						DEBUG('t',"customer %d looking for free table !!!!!!!!!!!!", mynumber);

						tablelinelock->Acquire();

						 manager_cust_orderlock->Release();

						tableline++;

						tablelineCV->Wait(tablelinelock);

						manager_customer_token= customer_token_number[mynumber];


						tablelinelock->Release();

						manager_customer_tablelock->Acquire();

						manager_customer_tableCV->Signal(manager_customer_tablelock);
						printf("\ncustomer [%d] is waiting to sit on the table",mynumber);
						manager_customer_tableCV->Wait(manager_customer_tablelock);

						manager_customer_tablelock->Release();

						ordermantainlock->Acquire();

						int my_table= order_book_details[customer_token_number[mynumber]]->table_number;

						ordermantainlock->Release();

						tablelock[my_table]->Acquire();
						printf("\ncustomer [%d] is seated at table number [%d]",mynumber,my_table);



						//*********************************************
							//waiter interaction starts from here ....
							//***********************************************


									currentThread->Yield();						// time given to eat food

									DEBUG('t'," customer[%d]: already had soda and got seat....byeeeeeee!!!",mynumber);
								printf("\nCustomer [%d] already had his soda so no waiter",mynumber);
									customer_gonelock->Acquire();
									customers_left++;

									customer_gonelock->Release();

									table_stat[my_table]= TABLE_FREE;

									tablelock[my_table]->Release();
printf("\n     %d            [%d]              byeeeeeee!!! !!!!!!!!!!!\n",customers_left,mynumber);									currentThread->Finish();


		       }



		}

	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
								//////////MANAGER/////////
//////////////////////////////////////////////////////////////////////////////////////////////////

void manager (void)

{
int count=50;
int cook_free;

while(1)

{
	DEBUG('t',"started manager!!!!!");

	/***********************************
		  CHECKING WHEN TO FINISH THE MANAGER
	 ************************************/
	customer_gonelock->Acquire();
	if(customers_left==CUSTOMERS)
	{
		DEBUG('t',"endingggggggggggggggggg manager!!!!!");
		printf("\nMANAGER ENDS HERE!!!!!\n");

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

		// checking whether 6 dollar burger need to be made

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
					cook_stat[o]=D6B;
					manager_cooklock->Acquire();
					manager_cookCV->Signal(manager_cooklock);
					printf("\nManager informs Cook [%d] to cook 6-dollar burger",o);
					manager_cooklock->Release();
					break;
				}

			}
			if(cook_free==0)
							{
								// hire a new cook
								DEBUG('t',"HIRING A NEW COOK for d6b");
								char* name_cook=(char *) "Cook";
								new_cooks++;
								Thread *t= new Thread(name_cook);
								manager_cooklock->Acquire();
								cook_stat[COOKS+new_cooks]=D6B;
     							printf("\nManager HIRES new Cook to cook 6-dollar burger");
								t->Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
								manager_cooklock->Release();

							}
		}
		else
		{
			total_foodlock->Release();
		}

		//checking whether 3 dollar burger need to be made

		total_foodlock->Acquire();
		if((available_items_obj->d3b- pending_order_obj->d3b) < 5)
				{
					total_foodlock->Release();
					cook_free=0;
					for(int o=0; o<(COOKS + new_cooks); o++)
					{
						if(cook_stat[o]==COOK_FREE)
						{
							cook_free++;
							cook_stat[o]=D3B;
							manager_cooklock->Acquire();
							manager_cookCV->Signal(manager_cooklock);
							printf("\nManager informs Cook [%d] to cook 3-dollar burger",o);
							manager_cooklock->Release();
							break;
						}

					}
					if(cook_free==0)
								{
									// hire a new cook
										DEBUG('t',"HIRING A NEW COOK for d3b");
										char* name_cook=(char *) "Cook";
										new_cooks++;
										Thread *t= new Thread(name_cook);
										manager_cooklock->Acquire();
										cook_stat[COOKS+new_cooks]=D3B;
										printf("\nManager HIRES new Cook to cook 3-dollar burger");
										t->Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
										manager_cooklock->Release();

									}
				}
		else
				{
					total_foodlock->Release();
				}

		//checking whether veggie burger need to be made

		total_foodlock->Acquire();
		if((available_items_obj->sb- pending_order_obj->sb) < 5)
				{
					total_foodlock->Release();
					cook_free=0;
					for(int o=0; o<(COOKS + new_cooks); o++)
					{
						if(cook_stat[o]==COOK_FREE)
						{
							cook_free++;
							cook_stat[o]=SB;
							manager_cooklock->Acquire();
							manager_cookCV->Signal(manager_cooklock);
							printf("\nManager informs Cook [%d] to cook veggie burger",o);
							manager_cooklock->Release();
							break;
						}

					}
					if(cook_free==0)
									{
														// hire a new cook
														DEBUG('t',"HIRING A NEW COOK for sb");
														char* name_cook=(char *) "Cook";
														new_cooks++;
														Thread *t= new Thread(name_cook);
														manager_cooklock->Acquire();
														cook_stat[COOKS+new_cooks]=SB;
														printf("\nManager HIRES new Cook to cook veggie-burger");
														t->Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
														manager_cooklock->Release();
									}
				}
		else
				{
					total_foodlock->Release();
				}

		//checking whether fries need to be made

		total_foodlock->Acquire();
		if( (available_items_obj->fries - pending_order_obj->fries) <5)
				{
					total_foodlock->Release();
					cook_free=0;
					for(int o=0; o<(COOKS + new_cooks); o++)
					{
						if(cook_stat[o]==COOK_FREE)
						{
							cook_free++;
							cook_stat[o]=FRIES;
							manager_cooklock->Acquire();
							manager_cookCV->Signal(manager_cooklock);
							printf("\nManager informs Cook [%d] to cook french fries",o);
							manager_cooklock->Release();
							break;
						}

					}
					if(cook_free==0)
									{
												// hire a new cook
														DEBUG('t',"HIRING A NEW COOK for fries");
														char* name_cook=(char *) "Cook";
														new_cooks++;
														Thread *t= new Thread(name_cook);
														manager_cooklock->Acquire();
														cook_stat[COOKS+new_cooks]=FRIES;
														printf("\nManager HIRES new Cook to cook french fries");
														t->Fork((VoidFunctionPtr)cook,COOKS+new_cooks);
														manager_cooklock->Release();
									}
				}
		else
				{
					total_foodlock->Release();
				}

		//check if item is in surplus---6 dollar burger

		total_foodlock->Acquire();

		if( (available_items_obj->d6b - pending_order_obj->d6b) >20)
						{
							total_foodlock->Release();

							for(int n=0; n<(COOKS + new_cooks); n++)
							{
								if(cook_stat[n]==D6B)
								{
									printf("\nManager sends Cook[%d] to break",n);
									cook_stat[n]=COOK_FREE;

								}

							}
						}
		else
				{
					total_foodlock->Release();
				}

		//check if item is in surplus 3 dollar burger

		total_foodlock->Acquire();

		if( (available_items_obj->d3b - pending_order_obj->d3b) >20)
						{
							total_foodlock->Release();

							for(int n=0; n<(COOKS + new_cooks); n++)
							{
								if(cook_stat[n]==D3B)
								{

									printf("\nManager sends Cook[%d] to break",n);
									cook_stat[n]=COOK_FREE;

								}

							}
						}
		else
				{
					total_foodlock->Release();
				}


		////check if item is in surplus---veggie burger

		total_foodlock->Acquire();

		if( (available_items_obj->sb - pending_order_obj->sb) >20)
						{
							total_foodlock->Release();

							for(int n=0; n<(COOKS + new_cooks); n++)
							{
								if(cook_stat[n]==SB)
								{
									printf("\nManager sends Cook[%d] to break",n);
									cook_stat[n]=COOK_FREE;

								}

							}
						}
		else
				{
					total_foodlock->Release();
				}


		////check if item is in surplus---fries

		total_foodlock->Acquire();

		if( (available_items_obj->fries - pending_order_obj->fries) >20)
						{
							total_foodlock->Release();

							for(int n=0; n<(COOKS + new_cooks); n++)
							{
								if(cook_stat[n]==FRIES)
								{
									printf("\nManager sends Cook[%d] to break",n);
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


	tablelinelock->Acquire();					//acquire eat-in line lock to check length of line


	if(tableline>0)							//check if there are customers waiting for the tables
	{

		DEBUG('t',"a customer waiting for a table!!!!!!!!!!");
		printf("a customer waiting for a table!!!!!!!!!!\n");


		for (int x=0;x<TABLES;x++)							//check if there are any free tables available

	{
			tablelock[x]->Acquire();


			if(table_stat[x]==TABLE_FREE)
			{
				DEBUG('t',"free tables available!!! !!!!!!!!!!!");

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

				DEBUG('t',"seeeeeeeeeeeeeeat given to customer %d!!! !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!",order_book_details[manager_customer_token]->customer_no);

				printf("\nManager gives table[%d] to Customer[%d]",x,order_book_details[manager_customer_token]->customer_no);

				manager_customer_tableCV->Signal(manager_customer_tablelock);


				ordermantainlock->Release();				//Release main order book lock


				manager_customer_tablelock->Release();



				break;

			}
			else{
					printf("\nManager can't give table to Customer as no table free");
					tablelinelock->Release();
					tablelock[x]->Release();
					break;
			}


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

						 printf("\nManager packs food for customer[%d]",order_book_details[m]->customer_no);
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
				printf("Manager broadcasts token number:[%d] for customer",l);
				togo_waitingCV->Broadcast(togo_waitinglock);
				togo_waitinglock->Release();

			}

			else					// if eat in tell waiters if any food needs to be send at table
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
							printf("\nManager wakes up waiter[%d]",g);

							printf("\nManager gives token number[%d] to waiter[%d] for customer[%d]",l,g,order_book_details[l]->customer_no);
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

			DEBUG('t',"MANAGER TAKING ORDER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			DEBUG('t',"before acquiring customerlinelock");

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

				DEBUG('t'," managerrrrrr CALCULATING MONEY and give bill!!!!!!!!!!!!!!!!!!!!!");

				printf("\nManager is taking order of Customer[%d]",manager_customerno);

				manager_cust_orderCV->Signal(manager_cust_orderlock);
				manager_cust_orderCV->Wait(manager_cust_orderlock);								// waiting to get money from customer

				DEBUG('t'," managerrrrr checking for order details!!!!!!!!!!!!!!!!!!!!!!");


				if ((manager_orderbook_obj->soda!=0)&&(manager_orderbook_obj->fries==0)&&(manager_orderbook_obj->d6b==0)&&
						(manager_orderbook_obj->d3b==0)&&(manager_orderbook_obj->sb==0))
				{
					// decrease from inventory with lock**********

					manager_orderbook_obj->mang_order_status_obj= ORDER_FINISH;

					printf("\nManager gives soda to Customer[%d]",manager_customerno);
					manager_cust_orderCV->Signal(manager_cust_orderlock);

					manager_cust_orderlock->Release();
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

					DEBUG('t',"food given to the customer by manager !!!!!!!!!!!!!!!!!");

					printf("\nManager gives food to Customer[%d]",manager_customerno);
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

					order_book_details[index_orderbook]->d3b=manager_orderbook_obj->d3b;	// copying the order from order takers data structures
					order_book_details[index_orderbook]->d6b=manager_orderbook_obj->d6b;	// to the order book
					order_book_details[index_orderbook]->sb=manager_orderbook_obj->sb;                      //
					order_book_details[index_orderbook]->fries=manager_orderbook_obj->fries;                //   ----- do -------
					order_book_details[index_orderbook]->soda=manager_orderbook_obj->soda;                  //
					order_book_details[index_orderbook]->eat_pref=manager_orderbook_obj->eat_pref;	//
					order_book_details[index_orderbook]->order_stat= FOOD_PENDING;	//	----do-------
					order_book_details[index_orderbook]->customer_seating_stat= GOT_SEAT;
					order_book_details[index_orderbook]->customer_no= manager_customerno;

					printf("\nManager gives token number[%d] to Customer[%d]",index_orderbook,manager_customerno);

					ordermantainlock->Release();


				}
			}

			else if (manager_orderbook_obj->eat_pref == GET_IN)
			{


				manager_orderbook_obj->mang_order_status_obj=ORDER_PENDING;
				DEBUG('t',"OT gives pending status to customer");
				manager_cust_orderCV->Signal(manager_cust_orderlock);


				ordermantainlock->Acquire();	// acquring lock to place order

				index_orderbook++;
				manager_orderbook_obj->token_number=index_orderbook;
				manager_cust_orderlock->Release();


				order_book_details[index_orderbook]->d3b=manager_orderbook_obj->d3b;	// copying the order from order takers data structures
				order_book_details[index_orderbook]->d6b=manager_orderbook_obj->d6b;	// to the order book
				order_book_details[index_orderbook]->sb=manager_orderbook_obj->sb;                      //
				order_book_details[index_orderbook]->fries=manager_orderbook_obj->fries;                //   ----- do -------
				order_book_details[index_orderbook]->soda=manager_orderbook_obj->soda;                  //
				order_book_details[index_orderbook]->eat_pref=manager_orderbook_obj->eat_pref;	//
				order_book_details[index_orderbook]->order_stat= FOOD_PENDING;	//	----do-------
				order_book_details[index_orderbook]->customer_seating_stat= NO_SEAT	;
				order_book_details[index_orderbook]->customer_no= manager_customerno;

				printf("\nManager gives token number[%d] to Customer[%d]",index_orderbook,manager_customerno);


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
			printf("\nManager withdraws money from bank so total money now is %d",totalmoney);

		}

			totalmoney -=50;

			totalmoneylock->Release();
			printf("\nManager orders inventory for 6-dollar burger");


			currentThread->Yield();
			currentThread->Yield();
			currentThread->Yield();
			currentThread->Yield();
			currentThread->Yield();


			//order raw materials
			inventory_obj->d6b += 50;


			DEBUG('t',"inventory for 6db increased by 50 to %d",inventory_obj->d6b);
			printf("\ninventory for 6-dollar burger is loaded into the restaurant");
	}


	else if(inventory_obj->d3b<20)
	{

	//get total money lock
	totalmoneylock->Acquire();

	if(totalmoney==0)
	{
		//go to bank and withdraw money
		totalmoney += 5000;
		printf("\nManager withdraws money from bank so total money now is %d",totalmoney);

	}

	//no need to go to the bank
		printf("\nManager orders inventory for 3-dollar burger");
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


		DEBUG('t',"inventory for 3db increased by 50 to %d",inventory_obj->d3b);
		printf("\ninventory for 3-dollar burger is loaded into the restaurant");
	}

	else if(inventory_obj->sb<20)
	{
	//get total money lock
	totalmoneylock->Acquire();

	if(totalmoney==0)
	{
		//go to bank and withdraw money
		totalmoney += 5000;
		printf("\nManager withdraws money from bank so total money now is %d",totalmoney);

	}

	//no need to go to the bank
		printf("\nManager orders inventory for veggie burger");
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

		DEBUG('t',"inventory for sb increased by 50 to %d",inventory_obj->sb);
		printf("\ninventory for veggie burger is loaded into the restaurant");
	}


	else if(inventory_obj->fries<20)
	{
	//get total money lock
	totalmoneylock->Acquire();

	if(totalmoney==0)
	{
		//go to bank and withdraw money
		totalmoney += 5000;
		printf("\nManager withdraws money from bank so total money now is %d",totalmoney);

	}

	//no need to go to the bank

		totalmoney -=50;
		printf("\nManager orders inventory for french fries");
		//release total money lock
		totalmoneylock->Release();

		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();
		currentThread->Yield();

		//order raw materials
		inventory_obj->fries += 50;

		DEBUG('t',"inventory for fries increased by 50 to !!!!!!!!!!!!!!!!!%d",inventory_obj->fries);
		printf("\ninventory for fries is loaded into the restaurant");
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
			printf("\nCook [%d] is cooking 6-dollar burger ",my_cook_number);
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

			DEBUG('t',"PREPARED 10 UNITS OF D6B!!!!!!! ");
			total_foodlock->Acquire();
			available_items_obj->d6b+=10;          //prepare 10 food at a time
			total_foodlock->Release();
			currentThread->Yield();

		}

		while(cook_stat[my_cook_number]==D3B)
				{
			printf("\nCook [%d] is cooking 3-dollar burger ",my_cook_number);
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

					DEBUG('t',"PREPARED 10 UNITS OF D3B!!!!!!! ");
					total_foodlock->Acquire();
					available_items_obj->d3b+=10;
					total_foodlock->Release();
					currentThread->Yield();
				}

		while(cook_stat[my_cook_number]==SB)
				{
					printf("\nCook [%d] is cooking veggie burger ",my_cook_number);
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

					DEBUG('t',"PREPARED 10 UNITS OF SB!!!!!!! ");
					total_foodlock->Acquire();
					available_items_obj->sb+=10;
					total_foodlock->Release();
					currentThread->Yield();
				}
				while(cook_stat[my_cook_number]==FRIES)
				{
						printf("\nCook [%d] is cooking french fries ",my_cook_number);
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

					DEBUG('t',"PREPARED 10 UNITS OF D6B!!!!!!! ");
					total_foodlock->Acquire();
					available_items_obj->fries+=10;
					total_foodlock->Release();
					currentThread->Yield();
				}


		if(cook_stat[my_cook_number]==COOK_FREE)
		{
			manager_cooklock->Acquire();
			printf("\nCook [%d] is on break",my_cook_number);
			manager_cookCV->Wait(manager_cooklock);
			printf("\nCook [%d] returned from break",my_cook_number);
			manager_cooklock->Release();
		}



	}



}



//////////////////////////////////////////////////////////////////////////////////////////////////
                             //////////////WAITER//////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


void waiter(int my_waiternumber)
{
	while(1)
	{
		waiter_tokennumber[my_waiternumber]=-1;
		if(waiter_stat[my_waiternumber]==WAITER_FREE)
		{
			waiterlock[my_waiternumber]->Acquire();

			printf("\nWaiter[%d] is going on break!!",my_waiternumber);
			waiterCV[my_waiternumber]->Wait(waiterlock[my_waiternumber]);  //waiting for signal from either ordertaker or manager

			printf("\nWaiter[%d] returned from break!!",my_waiternumber);

			waiterlock[my_waiternumber]->Release();
			currentThread->Yield();
			currentThread->Yield();
			ordermantainlock->Acquire();

			printf("\nWaiter[%d] got token number[%d] for Customer[%d]",my_waiternumber,waiter_tokennumber[my_waiternumber],order_book_details[waiter_tokennumber[my_waiternumber]]->customer_no);

			tobe_service_tablenumber[my_waiternumber]=	order_book_details[waiter_tokennumber[my_waiternumber]]->table_number;

			tablelock[tobe_service_tablenumber[my_waiternumber]]->Acquire();

			printf("\nWaiter [%d] validates the token number for Customer[%d] ",my_waiternumber,order_book_details[waiter_tokennumber[my_waiternumber]]->customer_no);

			tableCV[tobe_service_tablenumber[my_waiternumber]]->Signal(tablelock[tobe_service_tablenumber[my_waiternumber]]);
			order_book_details[waiter_tokennumber[my_waiternumber]]->order_stat= FOOD_GIVEN;

			DEBUG('t',"food given to the customer by waiter !!!!!!!!!!!!!!!!!!!!!!!!!!");

			printf("\nWaiter[%d] serves food to Customer[%d]",my_waiternumber,order_book_details[waiter_tokennumber[my_waiternumber]]->customer_no);
			tableCV[tobe_service_tablenumber[my_waiternumber]]->Signal(tablelock[tobe_service_tablenumber[my_waiternumber]]);
			tablelock[tobe_service_tablenumber[my_waiternumber]]->Release();
			ordermantainlock->Release();

			waiter_stat[my_waiternumber]=WAITER_FREE;



		}
	}
}

#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock

    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
	    t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();	// Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
	    t1_l1.getName());
    for (int i = 0; i < 10; i++)
	;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock

    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
	printf("%s: Trying to release Lock %s\n",currentThread->getName(),
	       t1_l1.getName());
	t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting

    for ( int i = 0; i < 5 ; i++ )
	t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}

// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting

    for ( int i = 0; i < 5 ; i++ )
	t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;

    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
	t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t3_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
	t3_done.P();

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t4_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
	t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}
#endif

