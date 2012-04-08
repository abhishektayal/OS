#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "list.h"
#include "synch.h"
#include "threadtest.cc"

void customer(int customernumber)
{
int myordertaker;


customerlinelock->Acquire();
linelength++;
customerlineCV->wait(customerlinelock);

for(int j=0;j<3;j++)
{
	if(ordertakerstatus[j]==WAIT)
	{
		debug('t',"COMMENT: looking for my order taker");
		myordertaker=j;
		ordertakerstatus[j]=BUSY;
		break;
	}
}
customerlinelock->Release();
customerordertakerlock[j]->Acquire();

//place the order
debug('t',"COMMENT: inside place order");

cutomerordertakerCV[j]->signal(customerordertakerlock[j]);
customerordertakerCV[j]->wait(customerordertakerlock[j]);
}
