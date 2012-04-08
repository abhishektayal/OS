/*
 * carljr_func.h
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

void ordertaker(int);
void customer(customer_details*);
int calculatemoney (order_book*);			// function to calculate money to be paid by the customer



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


		/****************************************************************************
					CUSTOMER DETAILS
		 *   structure to hold customer specific information:
			1)	customer no: customer_no
			2)  eating preference: eat_pref
			ORDER:
			1) $6 brg: d6b
			2) $3 brg: d3b
			3) simple brg: sb
			4) soda: soda
			5) fries: fries
		*****************************************************************************	*/

			struct customer_details
			{
				int customer_no=0;
				eating_pref eat_pref;
				int d6b=0,d3b=0,sb=0,fries=0,soda=0;

			};

			customer_details customer_array[20];						// array of structure containing customer details , so as to contain
																//	the details of all the customers.


			/****************************************************************************
								ORDER BOOK
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
					int customer_no=0;
					eating_pref eat_pref;
					int d6b=0,d3b=0,sb=0,fries=0,soda=0;

				};

				order_book order_book_details[100];						// array of structure containing customer details , so as to contain
																	//	the details of all the customers.
