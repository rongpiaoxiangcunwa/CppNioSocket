#ifndef _ConcurrentHashMap_hpp_
#define _ConcurrentHashMap_hpp_

#include <vector>
#include <iostream>
#include <map>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/functional/hash.hpp>
#include "AtomicInteger.hpp"

using std::vector;
using std::map;

typedef boost::shared_mutex Lock;
typedef boost::unique_lock<Lock> WriteLock;
typedef boost::shared_lock<Lock> ReadLock;

template<class K, class V>
class Segment
{
public:
	bool insert(const K& key, const V& val) {
		WriteLock wrLock(lock_);
		return map_.insert(std::make_pair(key, val)).second;
	}

	std::pair<V, bool> get(const K& key)  {
		ReadLock rdLock(lock_);
		typename map<K, V>::const_iterator mit = map_.find(key);
		if (mit != map_.end()) {
			return std::make_pair(mit->second, true);
		}
		return std::make_pair(V(), false);
	}

	bool erase(const K& key) {
		WriteLock wrLock(lock_);
		return map_.erase(key) > 0;
	}

	int size() {
		return map_.size();
	}

	void print() {
		std::cout << size() << std::endl;
	}
	
private:
	map<K, V> map_;
	Lock lock_;
};


	
template<class K, 
		 class V, 
		 class HASH = boost::hash<K> >
class ConcurrentHashMap
{
protected:
	#define HASH_BITS  0x7fffffff

	/////////////////////////////
public:	
	ConcurrentHashMap(int initCapcity = 16, const HASH &hasher = HASH()) : capacity_(initCapcity), hasher_(hasher) {
		for(int i = 0; i < initCapcity; i++) buckets_.push_back(new Segment<K,V>());
	}

	~ConcurrentHashMap() {
		for(int i = 0; i < capacity_; i++) delete buckets_[i];
	}
	
	bool insert(const K& key, const V& value) {
		bool ret = getBucket(key).insert(key, value);
		if (ret) counts_++;
		return ret;
	}

	std::pair<V, bool> get(const K& key) {
		return getBucket(key).get(key);
	}

	bool erase(const K &key) {
		bool ret = getBucket(key).erase(key);
		if (ret) counts_--;
		return ret;
	}

	int size() {
		return counts_.get();
	}

	void print() {
		for (int i = 0; i < capacity_; i++) std::cout << i << " " <<  buckets_[i]->size() << std::endl;
	}

private:
	Segment<K,V>& getBucket(const K &key) {
		std::size_t hash = spread(key);
		int i = (capacity_ - 1) & hash;
		return *(buckets_[i]);
	}
	
	std::size_t spread(const K& key) {
		std::size_t h = hasher_(key);
		//return h;
		return (h ^ (h >> 16)) & HASH_BITS;
	}

private:
	int capacity_;
	vector< Segment<K, V>* > buckets_;
	AtomicInteger<int> counts_;
	HASH hasher_;
};

#endif
