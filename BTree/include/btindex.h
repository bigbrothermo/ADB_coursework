#ifndef BTINDEX_PAGE_H
#define BTINDEX_PAGE_H


#include "minirel.h"
#include "page.h"
#include "sortedpage.h"
#include "bt.h"



class BTIndexPage : public SortedPage {
	
private:

	// No private or public members should be declared.

public:
	
	// You may add public methods here.
	
	Status Insert(const int key, const PageID pid, RecordID& rid);
	Status Delete(const int key, RecordID& rid);

	Status GetFirst(int& key, PageID& pid, RecordID& rid);
	Status GetNext(int& key, PageID& pid, RecordID& rid);
	
	PageID GetLeftLink(void);
	void SetLeftLink(PageID left);
	    
	IndexEntry* GetEntry(int slotNo) 
	{
		return (IndexEntry *)(data + slots[slotNo].offset);
	}

	bool IsAtLeastHalfFull()
	{
		return (AvailableSpace() <= (HEAPPAGE_DATA_SIZE) / 2);
	}

	PageID returnpage(){
		return this->pid;
	}



	Status GetPageID (const int *key, PageID& pid)
{


	IndexEntry* current_Key=new IndexEntry;
	//current_Key->key=*key;
	//current_Key->pid=pid;

	
	// A sequential search is implemented here.  You can modify it
	// to a binary search if you like.
	
	for (int i = numOfSlots - 1; i >= 0; i--)
	{
		current_Key=GetEntry(i);
		
		if (*key-current_Key->key >= 0)
		{	
			//current_Key=GetEntry(i);
			pid=current_Key->pid;
			return OK;
		}
	}
	
	// If we reach this point, then the page we should follow in our 
	// B+tree search must be the leftmost child of this page.
	
	pid = GetLeftLink();
	return OK;
}



};

#endif
