#pragma once

#include "hash.hpp"
#include "perfcheck.hpp"


// Template for hash using linear probing with tombstoning or
// lazy deletion.
template<typename K, typename V, class Hasher = HashFn<K>>
struct LazyLPHash : public IHash<K, V, Hasher> {
    PERF_INIT;

    struct HashEntry {
        HashEntry() {}
        HashEntry( K key, V val ) : key(key), val(val) {}

        K key;
        V val;
        bool occupied = false;
        bool deleted = false;
    }; 

    LazyLPHash( int _numBuckets, float _loadThreshold ) {
        this->numBuckets = _numBuckets;
        this->loadThreshold = _loadThreshold;
        this->numEntries = 0;

        this->buckets = new HashEntry[this->numBuckets];
    }

    LazyLPHash() : LazyLPHash(10, 0.7) {}

    ~LazyLPHash() {
        delete [] buckets;
    }

    void resize(int newBuckets ) {
        HashEntry * old = buckets;
        int oldBuckets = this->numBuckets;
        this->numBuckets = newBuckets;

        this->buckets = new HashEntry[this->numBuckets];

        this->numEntries = 0;

        // discard deleted entries
        for( int i = 0; i < oldBuckets; ++i ) {
            if( !old[i].deleted && old[i].occupied ) {
                put( old[i].key, old[i].val );
            }
        }

        delete [] old;
    }

    int lookup( K key ) {
        int idx = this->hash( key );

        // we either get an empty slot or the slot with our key
        while( this->buckets[idx].deleted ||
                ( this->buckets[idx].occupied &&
                  this->buckets[idx].key != key ) ) {
            idx = ( idx + 1 ) % this->numBuckets;
        }

        return idx;
    }

    void put( K key, V val ) {
        if( this->getLoadFactor() >= this->loadThreshold ) {
            resize( this->numBuckets * 2 );
        }

        int idx = lookup( key );

        // either the entry has the same key or is empty/deleted
        this->buckets[idx].occupied = true;
        this->buckets[idx].key = key;
        this->buckets[idx].val = val;

        if( this->buckets[idx].deleted ) { 
            this->buckets[idx].deleted = false;
        } else {
            ++this->numEntries;
        }
    }

    V get( K key ) {
        int idx = lookup( key );

        if( this->buckets[idx].occupied ) {
            return this->buckets[idx].val;
        }

        throw std::runtime_error("Key doesn't exist.");
    }

    void remove( K key ) {
        int idx = lookup( key );

        if( this->buckets[idx].occupied ) {
            this->buckets[idx].occupied = false;
            this->buckets[idx].deleted = true;

            // numEntries is not decremented, as we want to count
            // deleted entries in the load factor
        }

        // Key does not exist, nothing removed
    }

    void get_dib_stats() {
        for( int i = 0; i < this->numBuckets; ++i ) {
            if( this->buckets[i].occupied ) {
                PERF_ADD( this->probeLength( this->hash( this->buckets[i].key ), i ) );
            }
        }

        PERF_LOG;
        PERF_CLEAR;
    }

    HashEntry * buckets;
};
