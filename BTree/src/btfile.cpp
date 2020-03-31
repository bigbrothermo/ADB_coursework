#include "minirel.h"
#include "bufmgr.h"
#include "db.h"
#include "new_error.h"
#include "btfile.h"
#include "btfilescan.h"
#include <vector>


//-------------------------------------------------------------------
// BTreeFile::BTreeFile
//
// Input   : filename - filename of an index.  
// Output  : returnStatus - status of execution of constructor. 
//           OK if successful, FAIL otherwise.
// Purpose : If the B+ tree exists, open it.  Otherwise create a
//           new B+ tree index.
//-------------------------------------------------------------------

BTreeFile::BTreeFile (Status& returnStatus, const char* filename) 
{
    // TODO: add your code here
    PageID pid;
    Page * page;
    int status;

    int length=strlen(filename);
    char * temp_name= new char[length+1];
    this->filename=strcpy(temp_name,filename);

    status=MINIBASE_DB->GetFileEntry(filename,pid);

    if(status==FAIL){

    	if(MINIBASE_BM->NewPage(pid,page)!=FAIL){

    		if(MINIBASE_DB->AddFileEntry(filename,pid)!=FAIL){

    			//MINIBASE_DB->AddFileEntry(filename,pid);
    			((SortedPage *)page)->Init(pid);
    			((SortedPage *)page)->SetType(LEAF_NODE);
    			returnStatus=OK;
    			this->rootpage=(SortedPage *)page;
    			this->rootPid=pid;
    			return;
    		}
    		else{
    			std::cerr<<"Error in adding file entry"<<std::endl;
    			returnStatus=FAIL;
    			this->rootpage=NULL;
    			this->rootPid=INVALID_PAGE;
    			return;
    		}
    		
    	}
    	else{

    		std::cerr<<"Error in adding file entry"<<std::endl;
    		returnStatus=FAIL;
    		this->rootpage=NULL;
    		this->rootPid=INVALID_PAGE;
    		return;

    	}
    }
    else{
    	//Page * page;
    	MINIBASE_BM->PinPage(pid,page);
    	returnStatus=OK;
    	rootpage=(SortedPage *)page;
    	returnStatus=OK;
    	return;
    }
}


//-------------------------------------------------------------------
// BTreeFile::~BTreeFile
//
// Input   : None 
// Output  : None
// Purpose : Clean Up b00-------------------------------------

BTreeFile::~BTreeFile()
{
    // TODO: add your code here
}


//-------------------------------------------------------------------
// BTreeFile::DestroyFile
//
// Input   : None
// Output  : None
// Return  : OK if successful, FAIL otherwise.
// Purpose : Delete the entire index file by freeing all pages allocated
//           for this BTreeFile.
//-------------------------------------------------------------------

Status 
BTreeFile::DestroyFile()
{
    // TODO: add your code here
    if(rootpage->GetType()==INDEX_NODE){

   	BTIndexPage * btindex=(BTIndexPage *) rootpage;
    if(btindex->returnpage()!=INVALID_PAGE){
    	return FAIL;
    }

  }

  else{
    	BTLeafPage * blindex=(BTLeafPage *) rootpage;
    	if(blindex->returnpage()!=INVALID_PAGE){
    	return FAIL;
    }

    }

    //SortedPage *page;
   	//PIN(this->rootPid,page);// can we directly use this->RootPage?

   Status status=RecursiveDestory(this->rootPid);
   
   MINIBASE_DB->DeleteFileEntry(this->filename);
   FREEPAGE(this->rootPid);
   this->rootPid=INVALID_PAGE;
   this->rootpage=NULL; 
   return OK;
}






Status BTreeFile::RecursiveDestory(PageID targetPid){


	SortedPage *page;
	BTIndexPage * bipage;
	int key;
	
	RecordID rid,outRid;
	PageID t_pid;
	Status status;
   	
   	PIN(targetPid,page);// can we directly use this->RootPage?

   	if(page->GetType()==INDEX_NODE){

   		bipage=(BTIndexPage *) page;


		status=RecursiveDestory(bipage->GetLeftLink());//why need to use getleftlink()?
		if(status==FAIL){
			return FAIL;
		}


   		Status s=bipage->GetFirst(key,t_pid,outRid);
   		if(s!=OK)
   			return FAIL;

   		if(RecursiveDestory(t_pid)!=OK){
   			return FAIL;
   		}


   		while(s!=DONE){

   			s=bipage->GetNext(key,t_pid,outRid);
   		}



   	}
   	else{

   		FREEPAGE(targetPid);

   	}
   	return OK;

}

Status BTreeFile::IndexSplit(BTIndexPage * OriginIndex,BTIndexPage * NewIndex,PageID &newpid, const int key,const PageID pageid){
	NEWPAGE(newpid,NewIndex);
	NewIndex->Init(newpid);
	NewIndex->SetType(INDEX_NODE);

	RecordID targetdataRid,targetoutRid;
	RecordID newRid;
	PageID t_pid; 
	PageID temp_new_pid;
	PageID current_page;
	PageID new_current_page;
	int targetkey;
	Status s;
	int flag=0;


	/*while(OriginIndex->AvailableSpace() > NewIndex->AvailableSpace()&& OriginIndex->GetNext(targetkey,t_pid,targetoutRid)!=DONE){

			if(targetkey> key && flag==0){
				if(NewIndex->Insert(key,t_pid,targetoutRid)){
					UNPIN(OriginIndex->returnpage(),DIRTY);
					return FAIL;
				}
				flag=1;
			}

			s=OriginIndex->GetNext(targetkey,t_pid,targetoutRid);		

	}*/
	int count=0;

	//while(OriginIndex->GetNumOfRecords() * 0.5 > count && s!=DONE){
	while(true){
		RecordID newindexRid;
		int moveKey;
		PageID movepid;
		s=OriginIndex->GetFirst(targetkey,movepid,targetoutRid);
		if(s==DONE) break;
		if(NewIndex->Insert(targetkey,movepid,newindexRid)!=OK){
			UNPIN(newpid,true);
			return FAIL;
		}
		if(OriginIndex->DeleteRecord(targetoutRid)!=OK){
			UNPIN(newpid,true);
			return FAIL;
		}

	}

	/*RecordID insertRid;
	s=OriginIndex->GetFirst(targetkey,current_page,targetoutRid);
	while (OriginIndex->AvailableSpace() > NewIndex->AvailableSpace()) {
		// Check if the key we are currently on is bigger than the one we are trying to insert
		// If so, this is our insert oppertunity so insert the new value into the old (first) page
		if(flag==0 && targetkey > key){
			if (fullPage->Insert(key, pid, insertRid) != OK){
				UNPIN(newpid, true);
				return FAIL;
			}
			didInsert = true;
		}
		else {
			if (OriginIndex->Insert(targetkey,current_page,insertRid) != OK) {
				UNPIN(newpid, true);
				return FAIL;
			}
			if (newIndexPage->DeleteRecord(targetoutRid) != OK){
				UNPIN(newpid, true);
				return FAIL;
			}
			newIndexPage->GetFirst(targetkey,current_page,targetoutRid);
		}
	}*/



	/*while(s!=DONE){
			if(targetkey> key){
				if(OriginIndex->GetNumOfRecords() * 0.5 > count)
					flag=1;// need to insert to origin page
				else
					flag=0;// need to insert to new page
				break;
			}

			s=OriginIndex->GetNext(targetkey,t_pid,targetoutRid);
			//s=OriginIndex->GetFirst(targetkey,t_pid,targetoutRid);
			count++;
	}*/
	RecordID insertRid;
	s=NewIndex->GetFirst(targetkey,current_page,targetoutRid);
	while (OriginIndex->AvailableSpace() > NewIndex->AvailableSpace()) {
		// Check if the key we are currently on is bigger than the one we are trying to insert
		// If so, this is our insert oppertunity so insert the new value into the old (first) page
		if(flag==0 && targetkey > key){
			if (OriginIndex->Insert(key, pageid, insertRid) != OK){
				UNPIN(newpid, true);
				return FAIL;
			}
			flag = 1;
		}
		else {
			if (OriginIndex->Insert(targetkey,current_page,insertRid) != OK) {
				UNPIN(newpid, true);
				return FAIL;
			}
			if (NewIndex->DeleteRecord(targetoutRid) != OK){
				UNPIN(newpid, true);
				return FAIL;
			}
			NewIndex->GetFirst(targetkey, current_page, targetoutRid);
		}
	}

	if(flag==0){
		if(NewIndex->Insert(key,pageid,insertRid)!=OK){
			UNPIN(newpid,true);
			return FAIL;
		}

	}




	/*if(flag==1){
		Status s=OriginIndex->GetFirst(targetkey,t_pid,targetoutRid);
		while(s!=DONE){
			if(count > OriginIndex->GetNumOfRecords() * 0.5){
				if(NewIndex->Insert(targetkey,t_pid,targetoutRid)!=OK){
						UNPIN(newpid,DIRTY);
						return FAIL;
					}

			}

		s=OriginIndex->GetNext(targetkey,t_pid,targetoutRid);
		count++;
		}

		if (OriginIndex->Insert(key, t_pid, newRid) != OK){
				UNPIN(newpid, DIRTY);
				return FAIL;
			}


	}
	//insert to new page
	else{
		count=0;
		Status s=OriginIndex->GetFirst(targetkey,t_pid,targetoutRid);
		while(s!=DONE){
			if(count > OriginIndex->GetNumOfRecords() * 0.5){
				if(NewIndex->Insert(targetkey,t_pid,targetoutRid)!=OK){
						UNPIN(newpid,DIRTY);
						return FAIL;
					}

			}
			count++;
			s=OriginIndex->GetNext(targetkey,t_pid,targetoutRid);

		}
		if (NewIndex->Insert(key, t_pid, newRid) != OK){
				UNPIN(newpid, DIRTY);
				return FAIL;
			}


	}*/


	return OK;
}




Status BTreeFile::LeafSplit(BTLeafPage * OriginLeaf,BTLeafPage * NewLeaf,PageID &newpid, const int key,const RecordID rid){
	/*
	1. a new page
	2. set next page's prev-pointer to the new page
	3. line new page together in the B+ tree
	4. move all of the records from the old page to the new page
	*/

	//initialize a new page
	NEWPAGE(newpid,NewLeaf);
	NewLeaf->Init(newpid);
	NewLeaf->SetType(LEAF_NODE);

	//identify the next page of orinial page and maintain the link list
	BTLeafPage * nextLeaf;
	PIN(OriginLeaf->GetPrevPage(),nextLeaf);
	if(nextLeaf->returnpage()!=INVALID_PAGE){

	PIN(nextLeaf->returnpage(),nextLeaf);
	nextLeaf->SetPrevPage(newpid);

	NewLeaf->SetPrevPage(OriginLeaf->returnpage());
	NewLeaf->SetNextPage(nextLeaf->returnpage());

	UNPIN(nextLeaf->returnpage(),nextLeaf);

	OriginLeaf->SetNextPage(newpid);


	}

	//traverse the orginal page
	RecordID targetdataRid,targetoutRid,newRid;
	int targetkey;

	Status s;
	int flag=0;




	OriginLeaf->GetNumOfRecords();
	int count=0;


	//while(OriginLeaf->AvailableSpace() > NewLeaf->AvailableSpace()&& OriginLeaf->GetNext(targetkey,targetdataRid,targetoutRid)!=DONE){
	/*while( s!=DONE){

			if(targetkey> key){
				if(OriginLeaf->GetNumOfRecords() * 0.5 > count)
					flag=1;// need to insert to origin page
				else
					flag=0;// need to insert to new page
				break;
			}

			s=OriginLeaf->GetNext(targetkey,targetdataRid,targetoutRid);
			count++;
		
	}*/

	// Move all of the records from the old page to the new page
	while(true){
		RecordID newleafRid;
		int moveKey;
		s=OriginLeaf->GetFirst(moveKey,targetdataRid,targetoutRid);
		if(s==DONE) break;
		if(NewLeaf->Insert(moveKey,targetdataRid,newleafRid)!=OK){
			UNPIN(newpid,true);
			return FAIL;
		}
		if(OriginLeaf->DeleteRecord(targetoutRid)!=OK){
			UNPIN(newpid,true);
			return FAIL;
		}

		
	}


	//send back to origin page

	NewLeaf->GetFirst(targetkey,targetdataRid,targetoutRid);
	RecordID insertRid;

	while(OriginLeaf->AvailableSpace() > NewLeaf->AvailableSpace()){
		if(flag==0 && targetkey > key){
			if(OriginLeaf->Insert(key,rid,insertRid)!=OK){
				UNPIN(newpid,true);
				return FAIL;
			}
			flag=1;
		}
		else{
			if(OriginLeaf->Insert(targetkey,targetdataRid,insertRid)!=OK){
				UNPIN(newpid,true);
				return FAIL;
			}
			if(NewLeaf->DeleteRecord(targetoutRid)!=OK){
				UNPIN(newpid,true);
				return FAIL;
			}
			NewLeaf->GetCurrent(targetkey,targetdataRid,targetoutRid);
		}


	}


	//key need to be insert to origin page 
	/*if(flag==1){
		Status s=OriginLeaf->GetFirst(targetkey,targetdataRid,targetoutRid);
		while(s!=DONE){
			if(count > OriginLeaf->GetNumOfRecords() * 0.5){
				if(NewLeaf->Insert(targetkey,targetdataRid,targetoutRid)!=OK){
						UNPIN(newpid,DIRTY);
						return FAIL;
					}


			}

		s=OriginLeaf->GetNext(targetkey,targetdataRid,targetoutRid);
		count++;
		}

		if (OriginLeaf->Insert(key, rid, newRid) != OK){
				UNPIN(newpid, DIRTY);
				return FAIL;
			}


	}
	//insert to new page
	else{
		count=0;
		Status s=OriginLeaf->GetFirst(targetkey,targetdataRid,targetoutRid);
		while(s!=DONE){
			if(count > OriginLeaf->GetNumOfRecords() * 0.5){
				if(NewLeaf->Insert(targetkey,targetdataRid,targetoutRid)!=OK){
						UNPIN(newpid,DIRTY);
						return FAIL;
					}

			}
			count++;
			s=OriginLeaf->GetNext(targetkey,targetdataRid,targetoutRid);

		}
		if (NewLeaf->Insert(key, rid, newRid) != OK){
				UNPIN(newpid, DIRTY);
				return FAIL;
			}


	}
	*/

	if (!flag==0) {
		if (NewLeaf->Insert(key, rid, insertRid) != OK){
			UNPIN(newpid, true);
			return FAIL;
		}
	}


	// Set the output which is the first key of the new (second) page?

	return OK;


}



//-------------------------------------------------------------------
// BTreeFile::Insert
//
// Input   : key - the value of the key to be inserted.
//           rid - RecordID of the record to be inserted.
// Output  : None
// Return  : OK if successful, FAIL otherwise.
// Purpose : Insert an index entry with this rid and key.
// Note    : If the root didn't exist, create it.
//-------------------------------------------------------------------


Status 
BTreeFile::Insert(const int key, const RecordID rid)
{
    // TODO: add your code here

	/*
	whether rootpage exist

	1. split the root need to update the file entry
	2. split the interanl node is a recursive step
	3. be careful to maintain the link list of leaf nodes
		use a stack to record every node,

		If not enough space in the leaf-node, split the leaf node and the upper index node



	*/


	/*


	1. no root page
	2. root page is leaf node (enough space or not)
	3. root page is index node
	   traverse through the index nodes, push their pageIDs into a stack
	   (enough space of not)


	*/


	RecordID newRid;
	PIN(this->rootPid,this->rootpage);
	Status status;
	int inserted_length;
	//KeyDataEntry entry;
	//int key_self=strlen(key);



	if(rootpage->GetType()==LEAF_NODE){
	  //leaf node, directly insert to the page
		inserted_length=sizeof(LeafEntry);
		BTLeafPage * btleaf=(BTLeafPage *) rootpage;
		if(btleaf->AvailableSpace() > inserted_length ){
			//have adequate space to insert the data into leaf
			if(btleaf->Insert(key,rid,newRid)!=OK){
				UNPIN(this->rootPid,CLEAN);
				return FAIL;
			}
			UNPIN(this->rootPid, DIRTY);
		}
		else{
			//do not have adequate space in leaf,need to split
			BTLeafPage * NewLeaf;
			PageID newpid;
			if(LeafSplit(btleaf,NewLeaf,newpid,key,rid)!=OK){
					UNPIN(this->rootPid,CLEAN);
					return FAIL;
			}

			//change root to index node
			PageID newrootid;
			SortedPage * newrootpage;

			NEWPAGE(newrootid,newrootpage);

			BTIndexPage* newindexroot=(BTIndexPage*) newrootpage;
			newindexroot->Init(newrootid);
			newindexroot->SetType(INDEX_NODE);
			newindexroot->SetPrevPage(this->rootPid);

			int firstkey;
			RecordID newdataRid,newoutRid;
			//PageID current_page;
			btleaf->GetFirst(firstkey,newdataRid,newoutRid);
			if(newindexroot->Insert(firstkey,newpid,newoutRid)!=OK){
				UNPIN(newpid,CLEAN);
				UNPIN(this->rootPid,DIRTY);
				return FAIL;
			}

			this->rootPid=newrootid;
			this->rootpage=(SortedPage *)newindexroot;

			UNPIN(newrootid, DIRTY);
            UNPIN(newpid, DIRTY);


		}


	}
	else{
      //index node,

	/*
	1. mark the path of nodes
	2. find the leaf node
	3. if leaf node have adequate space, insert to the leaf 
	4. 

	*/
		BTIndexPage * btindex=(BTIndexPage *) rootpage;
		//inserted_length=strlen(key)+sizeof(RecordID);
		inserted_length=sizeof(LeafEntry);

		PageID currentPid;
		PageID nextPid;

		RecordID outRid;
		int targetkey=0;


		Status s;


		vector <PageID> parentnotes;


		while(btindex->GetType()==INDEX_NODE){

			currentPid=btindex->returnpage();
			parentnotes.push_back(currentPid);

			s=btindex->GetFirst(targetkey,currentPid,outRid);
			while(s!=DONE && key > targetkey){

				s=btindex->GetNext(targetkey,nextPid,outRid);
				// different from template

			}

			UNPIN(currentPid,CLEAN);
			PIN(nextPid,btindex);
		}

		BTLeafPage* btleaf =  (BTLeafPage *) btindex;
		if(btleaf->AvailableSpace() > inserted_length){
			if(btleaf->Insert(key,rid,outRid)!=OK){
				UNPIN(nextPid,CLEAN);
				return FAIL;
			}


		}
		else{
			//the space is not adequate, need split the leaf, and decide whether we need to split each 
			PageID newPid;
			int Newkey;
			BTLeafPage* newleaf;
			RecordID newoutRid,newRid;

			if(LeafSplit(btleaf,newleaf,newPid,key,rid)!=OK){
				UNPIN(newleaf->returnpage(),CLEAN);
				return FAIL;
			}

			btleaf->GetFirst(Newkey,newoutRid,outRid);

			UNPIN(newleaf->returnpage(),DIRTY);



			//traverse the parents nodes to find whether need to split

			int flag =1;
			PageID tempPid;
			PageID parent;
			BTIndexPage* tempbtIndex;


			vector <PageID> :: iterator vec_iter;
				

			while(flag==1){
				
				parent=parentnotes[parentnotes.size()-1];
				vec_iter=parentnotes.end()-1;
				parentnotes.erase(vec_iter);

				PIN(parent,tempbtIndex);

				inserted_length=sizeof(IndexEntry);

				if(tempbtIndex->AvailableSpace()>inserted_length){
					if(tempbtIndex->Insert(Newkey,parent,outRid)!=OK){
						UNPIN(parent,CLEAN);
						return FAIL;

					}
					flag=0;
					UNPIN(parent,DIRTY);

				}

				else{
					PageID newindexPid;
					BTIndexPage * newbtindex;
					int newfirstkey;

					if(IndexSplit(tempbtIndex,newbtindex,newindexPid,Newkey,newPid)!=OK){

						UNPIN(parent,CLEAN);
						return FAIL;
					}

					newbtindex->GetFirst(newfirstkey,newindexPid,outRid);
					parent=newindexPid;



					if(parentnotes.size()==0)
					{

						PageID NewRootPid;
						BTIndexPage* NewRootPage;

						NEWPAGE(NewRootPid,NewRootPage);

						NewRootPage->Init(NewRootPid);
						NewRootPage->SetType(INDEX_NODE);
						NewRootPage->SetPrevPage(parent);

						if (NewRootPage->Insert(Newkey,parent ,outRid ) != OK) {
								UNPIN(NewRootPid, CLEAN);
								return FAIL;
							}

						flag=0;

						//SET ROOT
						this->rootpage=(SortedPage *)NewRootPage;
    					this->rootPid=NewRootPid;
						UNPIN(NewRootPid, DIRTY);


					}
					
				}
			}

		}

	}


    //page->AvailableSpace()
    return OK;
}


//-------------------------------------------------------------------
// BTreeFile::Delete
//
// Input   : key - the value of the key to be deleted.
//           rid - RecordID of the record to be deleted.
// Output  : None
// Return  : OK if successful, FAIL otherwise. 
// Purpose : Delete an index entry with this rid and key.  
// Note    : If the root becomes empty, delete it.
//-------------------------------------------------------------------

Status 
BTreeFile::Delete(const int key, const RecordID rid)
{
    // TODO: add your code here

    if(this->rootPid==INVALID_PAGE){
    	return FAIL;
    }
    SortedPage* t_page;
    PageID t_id;
    RecordID t_rid;
    t_id=this->rootPid;
    PIN(t_id,t_page);

    if(t_page->GetType()==INDEX_NODE){
    	BTIndexPage* btindex=(BTIndexPage *) t_page;
    	vector<PageID> parentnotes;

    	BTIndexPage* CurrentIndexPage=btindex;
    	PageID currentPid;
    	PageID NextPid;
    	RecordID currentRid;
    	RecordID currentdataRid;
    	int current_key;
    	Status s=CurrentIndexPage->GetFirst(current_key,currentPid,currentRid);





    	while(CurrentIndexPage->GetType()==INDEX_NODE && key> current_key){
    		currentPid=CurrentIndexPage->PageNo();
    		parentnotes.push_back(currentPid);

    		s=CurrentIndexPage->GetFirst(current_key,currentPid,currentRid);
    		NextPid=CurrentIndexPage->GetLeftLink();

    		while(s!=DONE && key>current_key){
    			NextPid=currentPid;
    			s=CurrentIndexPage->GetNext(current_key,currentPid,currentRid);

    		}

    		UNPIN(currentPid, CLEAN);

			PIN(NextPid, CurrentIndexPage);	

    	}
    	BTLeafPage* btleaf=(BTLeafPage*) CurrentIndexPage;
    	currentPid=btleaf->PageNo();

    	btleaf->GetFirst(current_key,currentdataRid,currentRid);


    	if(btleaf->Delete(key,currentdataRid,t_rid)!=OK){
    		UNPIN(currentPid,CLEAN);
    		return FAIL;
    	}

    	if (btleaf->IsEmpty()){

				FREEPAGE(currentPid);
			}

		else{
			UNPIN(currentPid,DIRTY);
		}



    }
    else{
    	BTLeafPage * btleaf=(BTLeafPage *)t_page;
    	if(btleaf->Delete(key,rid,t_rid)!=OK){
    		UNPIN(t_id,CLEAN);
    		return FAIL;
    	}

    	if(btleaf->IsEmpty()){
    		FREEPAGE(t_id);
    	}
    	else{
    		UNPIN(t_id,DIRTY);
    	}






    }



    return OK;
}



Status BTreeFile::recursive_search(const int* key,PageID& targetPid,PageID currentPid){

	SortedPage * page;
	Status s;
	PIN(currentPid,page);
	

	if(page->GetType()==INDEX_NODE){
		PageID nextPid;
		BTIndexPage * btindex=(BTIndexPage *) page;
		if(key!=NULL){
		s=btindex->GetPageID(key,nextPid);
		
		UNPIN(currentPid,CLEAN);

		s=recursive_search(key,targetPid,currentPid);
		if(s!=OK){
			return FAIL;
		}
		return OK;
		}
		else{
			
			currentPid=btindex->GetPrevPage();
			s=recursive_search(key,targetPid,currentPid);
			if(s!=OK){
			return FAIL;
			}
			return OK;


		}


	}
	else{
		targetPid=page->PageNo();
		UNPIN(currentPid,CLEAN);


	}

	return OK;

}


Status BTreeFile::search(const int* key,PageID& targetPid){
	
	if(this->rootPid ==INVALID_PAGE){
		targetPid=INVALID_PAGE;
		return DONE;
	}

	Status s;

	s=recursive_search(key,targetPid,this->rootPid);

	if(s!=OK){
		return FAIL;
	}
	return OK;


}




//-------------------------------------------------------------------
// BTreeFile::OpenScan
//
// Input   : lowKey, highKey - pointer to keys, indicate the range
//                             to scan.
// Output  : None
// Return  : A pointer to IndexFileScan class.
// Purpose : Initialize a scan.  
// Note    : Usage of lowKey and highKey :
//
//           lowKey      highKey      range
//			 value	     value	
//           --------------------------------------------------
//           nullptr     nullptr      whole index
//           nullptr     !nullptr     minimum to highKey
//           !nullptr    nullptr      lowKey to maximum
//           !nullptr    =lowKey  	  exact match
//           !nullptr    >lowKey      lowKey to highKey
//-------------------------------------------------------------------



IndexFileScan*
BTreeFile::OpenScan(const int* lowKey, const int* highKey)
{
    // TODO: add your code here
    


	BTreeFileScan *scan= new BTreeFileScan;

	PageID LeftmostPageID;

	if(search(lowKey,LeftmostPageID)!=OK){
		LeftmostPageID=INVALID_PAGE;
	}

	scan->setLowkey(lowKey);
	scan->setHighkey(highKey);
	scan->setLeftmost(LeftmostPageID);
	scan->setflags();
	scan->setTree(this);

	return scan;





    /*

    BTreeFileScan * scan;
    int *tempkey= new int;

    if(lowKey !=NULL){
    	scan->setLowkey(lowKey);

    }
    else{
    	tempkey=0;
    	scan->setLowkey(NULL);

    }

    if(highKey !=NULL){
    	scan->setHighkey(highKey);
    }
    else{
    	scan->setHighkey(NULL);
    }

    if(this->rootPid!=INVALID_PAGE){
    	PageID currentPid;
    	if(lew)
    		

    }*/
    




    return nullptr;
}


//-------------------------------------------------------------------
// BTreeFile::PrintTree
//
// Input   : pageID - root of the tree to print.
// Output  : None
// Return  : None
// Purpose : Print out the content of the tree rooted at pid.
//-------------------------------------------------------------------

Status 
BTreeFile::PrintTree(PageID pageID)
{ 
	if ( pageID == INVALID_PAGE ) {
    	return FAIL;
	}

	SortedPage* page = nullptr;
	PIN(pageID, page);

	NodeType type = (NodeType) page->GetType();
	if (type == INDEX_NODE)
	{
		BTIndexPage* index = (BTIndexPage *) page;
		PageID curPageID = index->GetLeftLink();
		PrintTree(curPageID);

		RecordID curRid;
		int key;		
		Status s = index->GetFirst(key, curPageID, curRid);
		while (s != DONE)
		{
			PrintTree(curPageID);
			s = index->GetNext(key, curPageID, curRid);
		}
	}

	UNPIN(pageID, CLEAN);
	PrintNode(pageID);

	return OK;
}

//-------------------------------------------------------------------
// BTreeFile::PrintNode
//
// Input   : pageID - the node to print.
// Output  : None
// Return  : None
// Purpose : Print out the content of the node pid.
//-------------------------------------------------------------------




Status 
BTreeFile::PrintNode(PageID pageID)
{	
	SortedPage* page = nullptr;
	PIN(pageID, page);

	NodeType type = (NodeType) page->GetType();
	switch (type)
	{
		case INDEX_NODE:
		{
			BTIndexPage* index = (BTIndexPage *) page;
			PageID curPageID = index->GetLeftLink();
			cout << "\n---------------- Content of index node " << pageID << "-----------------------------" << endl;
			cout << "\n Left most PageID:  "  << curPageID << endl;

			RecordID currRid;
			int key, i = 0;

			Status s = index->GetFirst(key, curPageID, currRid); 
			while (s != DONE)
			{	
				i++;
				cout <<  "Key: " << key << "	PageID: " << curPageID << endl;
				s = index->GetNext(key, curPageID, currRid);
			}
			cout << "\n This page contains  " << i << "  entries." << endl;
			break;
		}

		case LEAF_NODE:
		{
			BTLeafPage* leaf = (BTLeafPage *) page;
			cout << "\n---------------- Content of leaf node " << pageID << "-----------------------------" << endl;

			RecordID dataRid, currRid;
			int key, i = 0;

			Status s = leaf->GetFirst(key, dataRid, currRid);
			while (s != DONE)
			{	
				i++;
				cout << "DataRecord ID: " << dataRid << " Key: " << key << endl;
				s = leaf->GetNext(key, dataRid, currRid);
			}
			cout << "\n This page contains  " << i << "  entries." << endl;
			break;
		}
	}
	UNPIN(pageID, CLEAN);

	return OK;
}

//-------------------------------------------------------------------
// BTreeFile::Print
//
// Input   : None
// Output  : None
// Return  : None
// Purpose : Print out this B+ Tree
//-------------------------------------------------------------------

Status 
BTreeFile::Print()
{	
	cout << "\n\n-------------- Now Begin Printing a new whole B+ Tree -----------" << endl;

	if (PrintTree(rootPid) == OK)
		return OK;

	return FAIL;
}

//-------------------------------------------------------------------
// BTreeFile::DumpStatistics
//
// Input   : None
// Output  : None
// Return  : None
// Purpose : Print out the following statistics.
//           1. Total number of leaf nodes, and index nodes.
//           2. Total number of leaf entries.
//           3. Total number of index entries.
//           4. Mean, Min, and max fill factor of leaf nodes and 
//              index nodes.
//           5. Height of the tree.
//-------------------------------------------------------------------
Status 
BTreeFile::DumpStatistics()
{	
	// TODO: add your code here	
	return OK;
}
