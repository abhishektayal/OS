/*
 * carljr_func.cc
 *
 *  Created on: Sep 12, 2010
 *      Author: Abhi
 */


void ordertaker(int ordertakernumber)
{
while(1)
	{


		int i=ordertakernumber;
		customerlinelock->Acquire();

		if(linelength>0)
			{
				customerlineCV->Signal(customerlinelock) ;
				linelength--;
				ordertakerstatus[i]=WAIT;
			}


//Acquires talk lock so that customer and order taker can talk mutually exclusively
customerordertakerlock[i]->Acquire();
customerlinelock->Release();

customerordertakerCV[i]->Wait(customerordertakerlock[i]);

ordermantainlock->Acquire();							// acquring lock to place order

	//calculatemoney();
	DEBUG('t',"CALCULATING MONEY!!!!!!!!!!!!!!!!!!!!!");

//ordermantainlock->Release();							// *******will not release lock untill it processes the order fully******


customerordertakerCV[i]->Signal(customerordertakerlock[i]);
customerordertakerCV[i]->Wait(customerordertakerlock[i]);

if ((order_book_details[index_orderbook]->soda!=0)&&(order_book_details[index_orderbook]->fries==0)&&(order_book_details[index_orderbook]->d6b==0)
		&&(order_book_details[index_orderbook]->d3b==0)&&(order_book_details[index_orderbook]->sb==0))
{
	// decrease from inventory**********

	customer_status="FINISH";
	customerordertakerCV[i]->Signal(customerordertakerlock[i]);
	ordermantainlock->Release();
	customerordertakerlock[i]->Release();
}

else if(order_book_details[index_orderbook]->eat_pref=="TO_GO")
{
		if(1/*food ready*/)						// calculate if food is ready
		{
		DEBUG('t',"food given to the customer");
		customer_status="FINISH";
		customerordertakerCV[i]->Signal(customerordertakerlock[i]);
			ordermantainlock->Release();
			customerordertakerlock[i]->Release();

		}

		else
		{
			token_number=index_orderbook;
			customer_status="PENDING";
			customerordertakerCV[i]->Signal(customerordertakerlock[i]);
			ordermantainlock->Release();
			customerordertakerlock[i]->Release();

		}
}

else if (order_book_details[index_orderbook]->eat_pref=="GET_IN")
{
	token_number=index_orderbook;
	customer_status="PENDING";
	customerordertakerCV[i]->Signal(customerordertakerlock[i]);
	ordermantainlock->Release();
	customerordertakerlock[i]->Release();


}


//else if
//{
	//debug('t',"COMMENT: inside food pack");
//}

if (linelength == 0)
{
ordertakerstatus[i]=FREE;
DEBUG('t',"COMMENT: status set to free");

}




	}


}



void customer(customer_details* details)
{

  int myordertaker;
  int mynumber= details->customer_no;


customerlinelock->Acquire();
linelength++;
customerlineCV->Wait(customerlinelock);

for( j=0;j<3;j++)
	{
		if(ordertakerstatus[j]==WAIT)
		{
			DEBUG('t',"COMMENT: looking for my order taker");
			myordertaker=j;
			ordertakerstatus[j]=BUSY;
			break;
		}
	}
customerlinelock->Release();
customerordertakerlock[j]->Acquire();

//place the order
DEBUG('t',"COMMENT: inside place order");

//ordermantainlock->Acquire();							// acquring lock to place order

//{
//	index_orderbook++;

//	order_book_details[index_orderbook]=customer_details[mynumber];				// Placing order

//}
//ordermantainlock->Release();					// releasing lock


customerordertakerCV[j]->Signal(customerordertakerlock[j]);
customerordertakerCV[j]->Wait(customerordertakerlock[j]);

// give money

DEBUG('t',"COMMENT: giving money !!!!!!!!!!!!");

customerordertakerCV[j]->Signal(customerordertakerlock[j]);
customerordertakerCV[j]->Wait(customerordertakerlock[j]);

if (details->eat_pref == "TO_GO")
{
	if(customer_status=="FINISH")
	{
		customerordertakerlock[j]->Release();
		currentThread->Finish();						// customer leaves the restaurant

	}
	else
	{
		token_number=index_orderbook;
		customerordertakerlock[j]->Release();
		togo_waitinglock->Acquire();
		togo_waitingCV->Wait(togo_waitinglock);
		// wait for token no to be broadcasted
		togo_waitinglock->Release();

	}


}

else if(details->eat_pref == "GET_IN")
{
	if(customer_status=="PENDING")
	{
		token_number=index_orderbook;
	}

	customerordertakerlock[j]->Release();
	// FIND PLACE() **************************
}


}




/*
 *
 int calculatemoney(order_book*)

{
	int total;
	total= order_book;

}
*/





