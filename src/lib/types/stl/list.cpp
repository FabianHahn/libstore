#include <cassert>
#include <vector>

extern "C" {
	#include <store.h>
	#include <types.h>
}

typedef std::vector<Store *> StlList;

StoreList StoreCreateList()
{
	return new StlList();
}

void StoreAppendList(StoreList list, Store *element)
{
	StlList& stlList = *static_cast<StlList *>(list);
	stlList.push_back(element);
}

int StoreGetListSize(StoreList list)
{
	StlList& stlList = *static_cast<StlList *>(list);
	return static_cast<int>(stlList.size());
}

Store *StoreGetListElement(StoreList list, int i)
{
	StlList& stlList = *static_cast<StlList *>(list);
	assert(i >= 0);
	assert(i < static_cast<int>(stlList.size()));
	return stlList[i];
}

void StoreFreeList(StoreList list)
{
	StlList& stlList = *static_cast<StlList *>(list);
	int n = static_cast<int>(stlList.size());
	for(int i = 0; i < n; i++) {
		StoreFree(stlList[i]);
	}
	delete &stlList;
}
