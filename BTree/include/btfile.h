#ifndef _BTFILE_H
#define _BTFILE_H

#include "btindex.h"
#include "btleaf.h"
#include "index.h"
#include "btfilescan.h"
#include "bt.h"

class BTreeFile: public IndexFile {
	
public:
	
	friend class BTreeFileScan;

	BTreeFile(Status& status, const char* filename);
	~BTreeFile();
	
	Status DestroyFile();
	
	Status Insert(const int key, const RecordID rid); 
	Status Delete(const int key, const RecordID rid);
    
	IndexFileScan* OpenScan(const int* lowKey, const int* highKey);
	
	Status Print();
	Status DumpStatistics();


	

	

private:
	
	// You may add members and methods here.

	PageID rootPid;

	SortedPage * rootpage;
	char* filename;
	
	Status PrintTree(PageID pid);
	Status PrintNode(PageID pid);

	//added
	Status RecursiveDestory(PageID targetPid);
	Status LeafSplit(BTLeafPage * OriginLeaf,BTLeafPage * NewLeaf,PageID &newpid, const int key,const RecordID rid);
	Status IndexSplit(BTIndexPage * OriginIndex,BTIndexPage * NewIndex,PageID &newpid, const int key,const PageID pageid);
	PageID Returnroot(){
		return this->rootPid;
	}
	Status search(const int* key,PageID& targetPid);
	Status recursive_search(const int* key,PageID& targetPid,PageID currentPid);
	

	


};


#endif // _BTFILE_H
