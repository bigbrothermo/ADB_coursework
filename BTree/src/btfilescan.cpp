#include "minirel.h"
#include "bufmgr.h"
#include "db.h"
#include "new_error.h"
#include "btfile.h"
#include "btfilescan.h"

//-------------------------------------------------------------------
// BTreeFileScan::~BTreeFileScan
//
// Input   : None
// Output  : None
// Purpose : Clean up the B+ tree scan.
//-------------------------------------------------------------------

BTreeFileScan::~BTreeFileScan()
{
    // TODO: add your code here
    int flag =0;

    
}


//-------------------------------------------------------------------
// BTreeFileScan::GetNext
//
// Input   : None
// Output  : rid  - record id of the scanned record.
//           key  - key of the scanned record
// Purpose : Return the next record from the B+-tree index.
// Return  : OK if successful, DONE if no more records to read.
//-------------------------------------------------------------------

Status 
BTreeFileScan::GetNext(RecordID& rid, int& key)
{
    // TODO: add your code here 
    if(this->endflag==1){
    	return DONE;
    }

    //RecordID currentRid;
    //PageID currentPid;
    //RecordID currentRid;
    BTLeafPage *btleaf;

    Status s;



    if(startflag==0){
    	//The scan have not started
    	this->currentPid=this->LeftmostPid;
    	PIN(currentPid,btleaf);

    	while(btleaf->GetFirst(key,dataRid,currentRid)!=OK){// Search the fisrt page
    		this->current_key=key;
    		PageID nextPID=btleaf->GetNextPage();
    		UNPIN(this->currentPid,false);

    		if (nextPID == INVALID_PAGE) {
    			std::cout<<"next page is invalid"<<std::endl;
				endflag = true;
				return DONE;
			}

    		this->currentPid=nextPID;
    		PIN(currentPid,btleaf);

    	}

    	this->current_key=key;

    	if(lowKey==NULL){
    		//start from the leftmost or end;
    		if(highKey==NULL  || key <= *highKey){
    			rid=dataRid;
    			startflag=1;
    			UNPIN(currentPid,false);
    			return OK;

    		}
    		else{
    			endflag=1;
    			std::cout<<"reach highest at first and low is NULL"<<std::endl;
    			UNPIN(currentPid, false);
				return DONE;
    		}
    	}
    	else{
    		if (*lowKey<=key) {
    			//start from the leftmost or end
				if (highKey == NULL || key<=*highKey) {
					rid = dataRid;
					startflag = 1;
					std::cout<<"reach highest at first and low is not NULL"<<std::endl;

					UNPIN(currentPid, false);
					return OK;
				}
				else {
					endflag = 1;
					std::cout<<"reach the highest key"<<std::endl;
					UNPIN(currentPid, false);
					return DONE;
				}
			}
			else{
				// Keep getting the next record until find the key 
				while (btleaf->GetNext(key,dataRid,currentRid) != DONE){
					this->current_key=key;
					if (*lowKey<=key) {
						if (highKey == NULL || key<=*highKey ) {
							rid = dataRid;
							startflag = 1;
							UNPIN(currentPid, false);
							return OK;
						}
						else {
							endflag = 1;
							std::cout<<"reach the highest key"<<std::endl;
							UNPIN(currentPid, false);
							return DONE;
						}
					}
				}
				this->current_key=key;
				endflag = 1;
				std::cout<<"run out of pages"<<std::endl;
				UNPIN(currentPid, false);
				return DONE;
			}	

    	}



    }
    else{

    	//The scan have started
    	std::cout<<"begin next "<<endl;
    	PIN(this->currentPid,btleaf);
    	s=btleaf->GetNext(key,dataRid,currentRid);
    	this->current_key=key;
    	while(s!=DONE && btleaf->GetNextPage()!=INVALID_PAGE){
    		PageID nextPID = btleaf->GetNextPage();
			UNPIN(currentPid, false);
			currentPid = nextPID;

			PIN(currentPid, btleaf);
			s = btleaf->GetFirst(key,dataRid,currentRid);
			this->current_key=key;

    	}
    	this->current_key=key;

    	if (s == DONE && btleaf->GetNextPage() == INVALID_PAGE) {
				UNPIN(currentPid, false);
				std::cout<<"no more pages"<<std::endl;
				endflag = 1;
				return DONE;
		}
		else{
			if(this->highKey!=NULL && *(this->highKey) < key ){
				endflag = 1;
				std::cout<<"reach the highest in further"<<std::endl;
				UNPIN(currentPid, false);
				return DONE;
			}
			else{
				rid = dataRid;
				UNPIN(currentPid, false);
				return OK;

			}
		}





    }


    return OK;
}


//-------------------------------------------------------------------
// BTreeFileScan::DeleteCurrent
//
// Input   : None
// Output  : None
// Purpose : Delete the entry currently being scanned (i.e. returned
//           by previous call of GetNext())
// Return  : OK if successful, DONE if no more record to read.
//-------------------------------------------------------------------


Status 
BTreeFileScan::DeleteCurrent()
{  
    // TODO: add your code here



	if(startflag==0){
		std::cerr<<"The scan have not started"<<std::endl;
		return FAIL;
	}
	else{
		std::cout<<current_key<<"\t"<<dataRid.pageNo<<"\t"<<dataRid.slotNo<<std::endl;
		if(bt->Delete(this->current_key,this->dataRid)!=OK){
			std::cerr<<"Fail to delete record through scan, current record is empty"<<std::endl;
			return FAIL;
		}



	}

    return OK;
}


