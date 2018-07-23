#include <map>
#include <string>

extern "C" {
	#include <store/store.h>
	#include <store/types.h>
}

typedef std::map<std::string, Store *> StlMap;

struct StlMapIterator {
	StlMap::iterator current;
	StlMap::iterator end;
};

StoreMap StoreCreateMap()
{
	return new StlMap();
}

bool StoreInsertMap(StoreMap map, const char *key, Store *value)
{
	StlMap &stlMap = *static_cast<StlMap *>(map);

	std::pair<StlMap::iterator, bool> inserted = stlMap.insert(std::make_pair(std::string(key), value));
	if(!inserted.second) { // entry with that key already exists
		// free the old one, then replace it with the new one
		StoreFree(inserted.first->second);
		inserted.first->second = value;
		return true;
	} else {
		return false;
	}
}

int StoreGetMapSize(StoreMap map)
{
	StlMap &stlMap = *static_cast<StlMap *>(map);
	return static_cast<int>(stlMap.size());
}

Store *StoreGetMapEntry(StoreMap map, const char *key)
{
	StlMap &stlMap = *static_cast<StlMap *>(map);
	StlMap::iterator query = stlMap.find(std::string(key));
	if(query != stlMap.end()) {
		return query->second;
	} else {
		return NULL;
	}
}

StoreMapIterator StoreCreateMapIterator(StoreMap map)
{
	StlMap &stlMap = *static_cast<StlMap *>(map);

	StlMapIterator *iterator = new StlMapIterator();
	iterator->current = stlMap.begin();
	iterator->end = stlMap.end();

	return iterator;
}

const char *StoreGetCurrentMapIteratorKey(StoreMapIterator iterator)
{
	StlMapIterator &stlMapIterator = *static_cast<StlMapIterator *>(iterator);
	if(stlMapIterator.current != stlMapIterator.end) {
		return stlMapIterator.current->first.c_str();
	} else {
		return NULL;
	}
}

struct Store *StoreGetCurrentMapIteratorValue(StoreMapIterator iterator)
{
	StlMapIterator &stlMapIterator = *static_cast<StlMapIterator *>(iterator);
	if(stlMapIterator.current != stlMapIterator.end) {
		return stlMapIterator.current->second;
	} else {
		return NULL;
	}
}

bool StoreIsMapIteratorValid(StoreMapIterator iterator)
{
	StlMapIterator &stlMapIterator = *static_cast<StlMapIterator *>(iterator);
	return stlMapIterator.current != stlMapIterator.end;
}

bool StoreForwardMapIterator(StoreMapIterator iterator)
{
	StlMapIterator &stlMapIterator = *static_cast<StlMapIterator *>(iterator);
	stlMapIterator.current++;
	return stlMapIterator.current != stlMapIterator.end;
}

void StoreFreeMapIterator(StoreMapIterator iterator)
{
	delete static_cast<StlMapIterator *>(iterator);
}

void StoreFreeMap(StoreMap map)
{
	StlMap &stlMap = *static_cast<StlMap *>(map);

	StlMap::iterator end = stlMap.end();
	for(StlMap::iterator iter = stlMap.begin(); iter != end; ++iter) {
		struct Store *store = iter->second;
		StoreFree(store);
	}

	delete &stlMap;
}
