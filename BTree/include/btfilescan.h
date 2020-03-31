#ifndef _BTREE_FILESCAN_H
#define _BTREE_FILESCAN_H

#include "btfile.h"

class BTreeFile;

class BTreeFileScan : public IndexFileScan {
	
public:
	
	friend class BTreeFile;

	Status GetNext(RecordID& rid,  int& key);
	Status DeleteCurrent();

	~BTreeFileScan();
	
private:
	const int * highKey;
	const int * lowKey;
	PageID currentPid;
	PageID LeftmostPid;
	RecordID dataRid;
	RecordID currentRid;
	RecordID previousRid;

	BTreeFile * bt;

	int startflag;
	int endflag;
	int current_key;

	void setLowkey(const int* lowkey){
		this->lowKey=lowkey;

	}
	void setHighkey(const int * highkey){
		this->highKey=highkey;
	}

	void setPid(PageID pid){
		this->currentPid=pid;
	}
	void setLeftmost(PageID LMPid){
		this->LeftmostPid=LMPid;
	}
	void setflags(){
		this->startflag=0;
		this->endflag=0;
	}
	void setTree(BTreeFile* input_bt){
		this->bt=input_bt;

	}
	/*void displaykey(){
		cout<<"key:"<<current_key<<endl;
		cout<<"PageNo:"<<dataRid.PageNo<<endl;
		cout<<":"<<dataRid.SlotNo<<endl;
	}*/

	


};

#endif
