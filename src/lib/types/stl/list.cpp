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

void StoreFreeList(StoreList list)
{
	StlList& stlList = *static_cast<StlList *>(list);
	int n = static_cast<int>(stlList.size());
	for(int i = 0; i < n; i++) {
		StoreFree(stlList[i]);
	}
	delete &stlList;
}
