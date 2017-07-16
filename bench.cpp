#include "all_hash.hpp"
#include <iostream>
#include <cassert>
#include <exception>
#include <random>
#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>
#include <set>

#define assertex( code ) \
{ \
    bool ex = false; \
    try { \
        code; \
    } catch( const exception& e ) { \
        ex = true; \
    } \
    assert(ex); \
}

using namespace std;

int main()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, 100000);
    auto get_num = bind( dis, gen );

    double table_size = 10000;
    double load_factor = 0.98;
    int max_entries = table_size * load_factor;

    LazyLPHash<int, int> llp(table_size, load_factor);
    LPHash<int, int> lp (table_size, load_factor);
    RHHash<int, int> rh (table_size, load_factor);

    vector<int> inserts;
    vector<int> deletes;
    vector<int> diff;

    for(int i = 0; i < max_entries; ++i) {
        inserts.push_back( get_num() );
    }

    for(int i = 0; i < max_entries / 2; ++i) {
        deletes.push_back( get_num() );
    }

    set<int> i(inserts.begin(), inserts.end());
    set<int> d(deletes.begin(), deletes.end());

    set_difference(
            i.begin(), i.end(),
            d.begin(), d.end(),
            inserter(diff, diff.begin())
            );

    for( auto i : inserts ) {
        llp.put( i, i );
        lp.put( i, i );
        rh.put( i, i );
    }

    for( auto i : inserts ) {
        assert(llp.get(i) == i );
        assert(lp.get(i) == i );
        assert(rh.get(i) == i );
    }

    llp.get_dib_stats();
    lp.get_dib_stats();
    rh.get_dib_stats();

    for( auto i : deletes ) {
        llp.remove( i );
        lp.remove( i );
        rh.remove( i );
    }

    for( auto i : deletes ) {
        assertex(llp.get( i ) );
        assertex(lp.get( i ) );
        assertex(rh.get( i ) );
    }

    llp.get_dib_stats();
    lp.get_dib_stats();
    rh.get_dib_stats();

    for( auto i : diff ) {
        assert(llp.get(i) == i );
        assert(lp.get(i) == i );
        assert(rh.get(i) == i );
    }

    return 0;
}
