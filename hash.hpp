#pragma once

#include <stdexcept>
#include <type_traits>
#include <string>


// Example of hash table implementation. This was just an exercise,
// it would be better to use stdlib map in a real program.


// HashFn provides default hash functions for default key types
// using template specialization.
template <typename K>
struct HashFn {
    virtual unsigned int hash( K key ) = 0;
};


// Template specialization of HashFn for default key types. This
// will be used by the hash class templates to automatically
// resolve for these specializations when a Hasher class is not given
// when instantiating the Hash. If there is no specialization for the
// key type given and no Hasher class is given, a compilation error
// will occur.
template<>
struct HashFn<int> {
    // Thomas Wang's integer hash found on:
    // http://burtleburtle.net/bob/hash/integer.html
    unsigned int hash( int _key ) {
        unsigned int key = _key;
        key = (key+0x7ed55d16) + (key<<12);
        key = (key^0xc761c23c) ^ (key>>19);
        key = (key+0x165667b1) + (key<<5);
        key = (key+0xd3a2646c) ^ (key<<9);
        key = (key+0xfd7046c5) + (key<<3);
        key = (key^0xb55a4f09) ^ (key>>16);

        return key;
    }
};


template<>
struct HashFn<std::string> {
    // djb2 hash function found on:
    // http://www.cse.yorku.ca/~oz/hash.html
    unsigned int hash( std::string str ) {
        unsigned int hash = 5381;
        int c;

        for( auto ch : str ) {
            c = int(ch);
            hash = ((hash << 5) + hash) + c;
        }

        return hash;
    }
};


// Users can also extend from HashFn as a way of providing their
// own hash functions for any key type, including user-defined classes.
// They can then use these classes as arguments to the Hash class
// templates below. Alternatively, users can provide their own template
// specializations above.
struct MyIntHashFn : public HashFn<int> {
    unsigned int hash( int key ) {
        return key;
    }
};


// Template for a generic hash table. The static assert guarantees
// that the Hasher provides a method that hashes keys of type K to int.
template <typename K, typename V, class Hasher = HashFn<K>>
struct IHash {
    static_assert( std::is_base_of<HashFn<K>, Hasher>::value,
            "IHash: Hasher does not hash type of key given!" );

    virtual void put( K key, V val ) = 0;
    virtual V get( K key ) = 0;
    virtual void resize( int newBuckets ) = 0;
    virtual void remove( K key ) = 0;

    int probeLength( int desired, int current ) {
        return (current >= desired) ?
            ( current - desired ) : ( current + this->numBuckets - desired );
    }

    int hash( K key ) {
        return hasher.hash( key ) % numBuckets;
    }

    float getLoadFactor( void ) {
        return float(numEntries) / float(numBuckets);
    }

    int numEntries;
    int numBuckets;
    float loadThreshold;
    Hasher hasher;
};

// Note: You will see in the implementation classes that they use
// a default template argument Hasher = HashFn<K>. This means
// that the user can define a template specialization for type K
// and use the class without specifying the Hasher class as a
// template argument, as explained above.

