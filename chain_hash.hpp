#pragma once

#include "hash.hpp"


// Template for a hash table using separate chaining for collision
// resolution.
template<typename K, typename V, class Hasher = HashFn<K>>
struct ChainedHash : public IHash<K, V, Hasher> {
    struct HashNode {
        HashNode( K key, V val ) : key(key), val(val), next(nullptr) {}

        K key;
        V val;
        HashNode * next;
    }; 

    ChainedHash( int _numBuckets, float _loadThreshold ) {
        this->numBuckets = _numBuckets;
        this->loadThreshold = _loadThreshold;
        this->numEntries = 0;

        this->buckets = new HashNode* [this->numBuckets];

        for( int i = 0; i < this->numBuckets; ++i ) {
            buckets[i] = nullptr;
        }
    }

    ChainedHash() : ChainedHash(10, 0.7) {}

    ~ChainedHash() {
        HashNode * next;

        for( int i = 0; i < this->numBuckets; ++i ) {
            if( this->buckets[i] ) next = this->buckets[i]->next;

            while( this->buckets[i] ) {
                delete this->buckets[i];
                this->buckets[i] = next;
            }
        }

        delete [] buckets;
    }

    void resize(int newBuckets ) {
        HashNode ** old = this->buckets;
        int oldBuckets = this->numBuckets;
        this->numBuckets = newBuckets;

        this->buckets = new HashNode* [this->numBuckets];

        for( int i = 0; i < this->numBuckets; ++i ) {
            this->buckets[i] = nullptr;
        }

        HashNode * next;

        this->numEntries = 0;

        for( int i = 0; i < oldBuckets; ++i ) {
            while( old[i] ) {
                put( old[i]->key, old[i]->val );

                next = old[i]->next;
                delete old[i];

                old[i] = next;
            }
        }

        delete [] old;
    }

    void put( K key, V val ) {
        if( this->getLoadFactor() >= this->loadThreshold ) {
            resize( this->numBuckets * 2 );
        }

        int idx = this.hash( key );

        HashNode * prev = nullptr;
        HashNode * ptr = this->buckets[idx];

        while( ptr ) {
            if( ptr->key == key ) {
                ptr->val = val;
                return;
            }
            prev = ptr;
            ptr = ptr->next;
        }

        if( prev ) {
            prev->next = new HashNode( key, val );
        } else {
            this->buckets[idx] = new HashNode( key, val );
        }

        ++this->numEntries;
    }

    V get( K key ) {
        int idx = this->hash( key );

        HashNode * ptr = this->buckets[idx];

        while( ptr ) {
            if( ptr->key == key ) return ptr->val;
            ptr = ptr->next;
        }

        throw std::runtime_error("Key doesn't exist.");
    }

    void remove( K key ) {
        int idx = this->hash( key );

        HashNode * prev = nullptr;
        HashNode * ptr = this->buckets[idx];

        while( ptr ) {
            if( ptr->key == key ) {
                if( !prev ) {
                    // update buckets if first elem deleted
                    this->buckets[idx] = ptr->next;
                } else {
                    // otherwise fill the gap
                    prev->next = ptr->next;
                }

                delete ptr;
                --this->numEntries;
                return;
            }

            prev = ptr;
            ptr = ptr->next;
        }

        // Key does not exist, nothing removed
    }

    HashNode ** buckets;
};

