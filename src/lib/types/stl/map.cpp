#include <map>
#include <string>

extern "C" {
	#include <store.h>
	#include <types.h>
}

typedef std::map<std::string, Store *> StlMap;

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
