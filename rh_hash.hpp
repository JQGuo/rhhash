#pragma once

#include "hash.hpp"
#include "perfcheck.hpp"
#include <utility> // for swap


// Template for hash using Robin Hood hashing with backwards
// shifting, which is similar to the shifting process to LPHash.

// Key Concepts:
// 1. Termination condition for lookup is based on comparing current
// probe length with probe length of each stored key, for which the
// expected value is comparatively low even for higher load factors.
// 2. Inserts and removes both do extra work in order to reduce
// the probe length of keys in the table by swapping and shifting entries.

// I use the method described here:
// http://codecapsule.com/2013/11/17/robin-hood-hashing-backward-shift-deletion/
// This method uses the shifting method in LPHash instead of
// tombstones, and seems to have much better performance when
// mixing in deletions.

template<typename K, typename V, class Hasher = HashFn<K>>
struct RHHash : public IHash<K, V, Hasher> {
    PERF_INIT;

    struct HashEntry {
        HashEntry() {}
        HashEntry( K key, V val ) : key(key), val(val) {}

        K key;
        V val;
        int hash;
        bool occupied = false;
    }; 

    RHHash( int _numBuckets, float _loadThreshold ) {
        this->numBuckets = _numBuckets;
        this->loadThreshold = _loadThreshold;
        this->numEntries = 0;

        this->buckets = new HashEntry[this->numBuckets];
    }

    RHHash() : RHHash(10, 0.7) {}

    ~RHHash() {
        delete [] buckets;
    }

    void resize(int newBuckets ) {
        HashEntry * old = buckets;
        int oldBuckets = this->numBuckets;
        this->numBuckets = newBuckets;

        buckets = new HashEntry[this->numBuckets];

        this->numEntries = 0;

        for( int i = 0; i < oldBuckets; ++i ) {
            if( old[i].occupied ) {
                put( old[i].key, old[i].val );
            }
        }

        delete [] old;
    }

    void put( K key, V val ) {
        if( this->getLoadFactor() >= this->loadThreshold ) {
            this->resize( this->numBuckets * 2 );
        }

        int hash = this->hash( key );
        int idx = hash;

        int logProbeLength = 0;
        int currentProbeLength = 0;
        int existingProbeLength;

        while( this->buckets[idx].occupied &&
                this->buckets[idx].key != key ) {

            // if the existing element has smaller probe length,
            // aka the distance between its desired and actual indices,
            // we get to evict it (stealing from the rich, giving to the poor)
            existingProbeLength = this->probeLength( this->buckets[idx].hash, idx );

            if( existingProbeLength < currentProbeLength ) {
                std::swap( currentProbeLength, existingProbeLength );
                std::swap( key, this->buckets[idx].key );
                std::swap( val, this->buckets[idx].val );
                std::swap( hash, this->buckets[idx].hash );
            }

            idx = ( idx + 1 ) % this->numBuckets;
            ++currentProbeLength;
            ++logProbeLength;
        }

        // either the entry has the same key or is empty/deleted
        this->buckets[idx].occupied = true;
        this->buckets[idx].key = key;
        this->buckets[idx].val = val;
        this->buckets[idx].hash = hash;

        ++this->numEntries;
    }

    // get compares the current run length and the stored run length
    // to determine when to terminate, along with empty entries
    V get( K key ) {
        int currentProbeLength = 0;
        int existingProbeLength;
        int idx = this->hash( key );

        for(;;) {
            if( !this->buckets[idx].occupied ) break;

            existingProbeLength = this->probeLength( this->buckets[idx].hash, idx );
            if( currentProbeLength > existingProbeLength ) break;

            if( this->buckets[idx].key == key ) {
                return this->buckets[idx].val;
            }

            idx = ( idx + 1 ) % this->numBuckets;
            ++currentProbeLength;
        }

        throw std::runtime_error("Key doesn't exist.");
    }

    // remove also follows the new termination rule
    void remove( K key ) {
        int currentProbeLength = 0;
        int existingProbeLength;
        int i = this->hash( key );

        for(;;) {
            if( !this->buckets[i].occupied ) break;

            existingProbeLength = this->probeLength( this->buckets[i].hash, i );
            if( currentProbeLength > existingProbeLength ) break;

            if( this->buckets[i].key == key ) {

                this->buckets[i].occupied = false;

                int j = i;

                // if our entry is removed, shift all entries over until we
                // find an empty entry, or one with probe length of 0. This
                // reduces the probe length for all shifted entries by 1.
                for(;;) {
                    j = ( j + 1 ) % this->numBuckets;

                    // the next entry was empty
                    if( !this->buckets[j].occupied ) break;

                    existingProbeLength = this->probeLength( this->buckets[j].hash, j );
                    // if probe length is 0, it is already in its desired spot
                    // so break out
                    if( existingProbeLength == 0 ) break;

                    // otherwise move entry j into the empty entry i
                    this->buckets[i] = this->buckets[j];

                    // entry j is now empty, and we iterate on j
                    i = j;
                    this->buckets[i].occupied = false;
                }

                --this->numEntries;
                break;
            }

            i = ( i + 1 ) % this->numBuckets;
            ++currentProbeLength;
        }

        // Key was does not exist, nothing removed.
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
