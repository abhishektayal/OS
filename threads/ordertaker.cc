/*
 * ordertaker.cc
 *
 *  Created on: Sep 11, 2010
 *      Author: Abhi
 */

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "list.h"
#include "synch.h"
#include "threadtest.cc"


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


//Acquires talk lock so that customer and order taker can talk without any hassels
customerordertakerlock[i]->Acquire();
customerlinelock->Release();

customerordertakerCV[i]->wait(customerordertakerlock[i]);




//else if
//{
	//debug('t',"COMMENT: inside food pack");
//}

if (linelength == 0)
{
ordertakertatus[i]=FREE;
debug('t',"COMMENT: status set to free");

}



}



}
