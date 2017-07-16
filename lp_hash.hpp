#pragma once

#include "hash.hpp"
#include "perfcheck.hpp"


// Template for hash using linear probing without lazy deletion.
// Improves the load factor as deletes do not count as entries as in
// LazyLPHash, but removing may require shifting successive occupied
// entries so they are not missed from terminating early from the 
// removed entries.
template<typename K, typename V, class Hasher = HashFn<K>>
struct LPHash : public IHash<K, V, Hasher> {
    PERF_INIT;

    struct HashEntry {
        HashEntry() {}
        HashEntry( K key, V val ) : key(key), val(val) {}

        K key;
        V val;
        unsigned int hash;
        bool occupied = false;
    }; 

    LPHash( int _numBuckets, float _loadThreshold ) {
        this->numBuckets = _numBuckets;
        this->loadThreshold = _loadThreshold;
        this->numEntries = 0;

        this->buckets = new HashEntry[this->numBuckets];
    }

    LPHash() : LPHash(10, 0.7) {}

    ~LPHash() {
        delete [] buckets;
    }

    void resize(int newBuckets ) {
        HashEntry * old = buckets;
        int oldBuckets = this->numBuckets;
        this->numBuckets = newBuckets;

        this->buckets = new HashEntry[this->numBuckets];

        this->numEntries = 0;

        for( int i = 0; i < oldBuckets; ++i ) {
            if( old[i].occupied ) {
                put( old[i].key, old[i].val );
            }
        }

        delete [] old;
    }

    int lookup( K key, int& hash ) {
        int idx = this->hash( key );

        // save this for checking hash during deletions
        hash = idx;

        while( this->buckets[idx].occupied &&
                this->buckets[idx].key != key ) {
            idx = ( idx + 1 ) % this->numBuckets;
        }

        return idx;
    }

    int lookup( K key ) {
        int temp;
        return lookup( key, temp );
    }

    void put( K key, V val ) {
        if( this->getLoadFactor() >= this->loadThreshold ) {
            resize( this->numBuckets * 2 );
        }

        int hash;
        int idx = lookup( key, hash );

        // either the entry has the same key or is empty
        this->buckets[idx].occupied = true;
        this->buckets[idx].key = key;
        this->buckets[idx].val = val;
        this->buckets[idx].hash = hash;

        ++this->numEntries;
    }

    V get( K key ) {
        int idx = lookup( key );

        if( this->buckets[idx].occupied ) {
            return this->buckets[idx].val;
        }

        throw std::runtime_error("Key doesn't exist.");
    }

    void remove( K key ) {
        // i is the empty entry
        int i = lookup( key );

        // Key does not exist, nothing removed
        if( !this->buckets[i].occupied ) {
            return;
        }

        // Key exists, mark it empty
        this->buckets[i].occupied = false;

        int j = i;

        for(;;) {
            // j is the next entry which may or may not replace i
            j = ( j + 1 ) % this->numBuckets;

            // the next entry was empty, terminate
            if( !this->buckets[j].occupied ) break;

            // k is where j should be if there was space at time of insertion
            int k = this->buckets[j].hash;

            /*
               Logic is as follows. Originally, an entry was meant to be placed
               at k, but was instead placed at position j. If empty
               position i is cyclically in the range of [k, j), then we should
               replace it with j.

               Scenarios for replacement:
               [...k...i...j...]
               [...j...k...i...]
               [...i...j...k...]

               k <= i <= j is the straightforward condition.
               Otherwise, account for wrap-around.
            */

            bool c1 = k <= i;
            bool c2 = i <= j;

            if( ( c1 && c2 ) ||  (j < k && ( c1 || c2 ) ) ) {
                // move entry j into the empty entry i
                this->buckets[i] = this->buckets[j];

                // entry j is now empty, and we iterate on j
                i = j;
                this->buckets[i].occupied = false;
            }
        }

        --this->numEntries;
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
