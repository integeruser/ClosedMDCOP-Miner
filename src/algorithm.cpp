#ifdef DEBUG
#define SPACES(i) std::setw( i ) << std::left << " "
#define PRINT(os) do { std::cout << os << std::flush; } while ( false )
#define PRINTLN(os) do { std::cout << os << std::endl; } while ( false )
#else
#define SPACES(i)
#define PRINT(os) do {} while ( false )
#define PRINTLN(os) do {} while ( false )
#endif

#include <algorithm>
#include <cassert>
#include <limits>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "prettyprint.hpp"

#include "algorithm.hpp"


std::map<Pattern, std::pair<Pattern, Pattern>> gen_candidate_co_occ(const std::set<Pattern>& mdp) {
    PRINTLN( SPACES( 10 ) << "-> " << __FUNCTION__ << ": " << mdp );
    assert ( std::adjacent_find( mdp.cbegin(), mdp.cend(), [](const Pattern& pattern1, const Pattern& pattern2) {
        return pattern1.size() != pattern2.size();
    } ) == mdp.end() );

    /* apriori-gen */

    std::map<Pattern, std::pair<Pattern, Pattern>> c;
    
    // join step
    for ( auto i = mdp.cbegin(); i != mdp.cend(); ++i ) {
        for ( auto j = i; j != mdp.cend(); ++j ) {
            const Pattern& pattern1 = *(i);
            const Pattern& pattern2 = *(j);
            assert( pattern1.size() == pattern2.size() );

            // check if the first k-1 elements of p1 and p2 are equals
            const std::vector<EventType> v1( pattern1.cbegin(), pattern1.cend() );
            const std::vector<EventType> v2( pattern2.cbegin(), pattern2.cend() );
            if ( std::equal( v1.begin(), v1.end()-1, v2.begin() ) ) {
                // check if the last element of p1 is less then the last element of p2
                if ( v1.back() < v2.back() ) {
                    Pattern p_union;
                    std::set_union( pattern1.cbegin(), pattern1.cend(), pattern2.cbegin(), pattern2.cend(),
                                   std::inserter( p_union, p_union.end() ) );
                    c[p_union].first = pattern1;
                    c[p_union].second = pattern2;
                }
            }
        }
    }
    
    // prune step
    for ( auto i = c.cbegin(); i != c.cend(); ) {
        const Pattern& pattern = (*(i)).first;
        
        auto subset_count = pattern.size();
        int existing_subset_count = 0;
        for ( const Pattern& prev_pattern : mdp ) {
            // check if prev_pattern is subset of pattern
            if ( std::includes( pattern.begin(), pattern.end(), prev_pattern.begin(), prev_pattern.end() ) ) {
                existing_subset_count++;
            }
        }
        
        assert( existing_subset_count <= subset_count );
        if ( existing_subset_count < subset_count ) { c.erase( i++ ); }
        else { ++i; }
    }
    
    PRINTLN( SPACES( 10 ) << "<- " << __FUNCTION__ << ": " << c );
    return c;
}


Table join(Table t1, Table t2, const std::shared_ptr<INeighborRelation> d) {
    PRINTLN( SPACES( 20 ) << "-> " << __FUNCTION__ );

    Table t;
    
    // sort-merge join http://www.dcs.ed.ac.uk/home/tz/phd/thesis/node20.htm
    auto i = t1.begin();
    auto j = t2.begin();
    while ( i != t1.end() && j != t2.end() ) {
        const RowInstance& row_instance1 = *(i);
        const RowInstance& row_instance2 = *(j);
        
        const std::set<std::shared_ptr<Object>>& prev_row_instance1 = row_instance1.first;
        const std::shared_ptr<Object>& object1 = row_instance1.second;
        const std::set<std::shared_ptr<Object>>& prev_row_instance2 = row_instance2.first;
        const std::shared_ptr<Object>& object2 = row_instance2.second;
        
        if ( prev_row_instance1 < prev_row_instance2 ) { ++i; }
        else if ( prev_row_instance1 > prev_row_instance2 ) { ++j; }
        else {
            if ( d->neighbors( object1, object2 ) ) {
                std::set<std::shared_ptr<Object>> tmp( prev_row_instance1 );
                tmp.insert( object1 );
                RowInstance row_instance{ tmp, object2 };
                t.insert( row_instance );
            }

            auto j_p = std::next( j );
            while ( j_p != t2.end() ) {
                const RowInstance& i2_p = *(j_p);
                
                const std::set<std::shared_ptr<Object>>& prev_row_instance2_p = i2_p.first;
                const std::shared_ptr<Object>& object2_p = i2_p.second;
                if ( prev_row_instance1 == prev_row_instance2_p ) {
                    if ( d->neighbors( object1, object2_p ) ) {
                        std::set<std::shared_ptr<Object>> tmp( prev_row_instance1 );
                        tmp.insert( object1 );
                        RowInstance row_instance{ tmp, object2_p };
                        t.insert( row_instance );
                    }
                }
                
                ++j_p;
            }

            auto i_p = std::next( i );
            while ( i_p != t1.end() ) {
                const RowInstance& i1_p = *(i_p);
                
                const std::set<std::shared_ptr<Object>>& prev_row_instance1_p = i1_p.first;
                const std::shared_ptr<Object>& object1_p = i1_p.second;
                if ( prev_row_instance1_p == prev_row_instance2 ) {
                    if ( d->neighbors( object1_p, object2 ) ) {
                        std::set<std::shared_ptr<Object>> tmp( prev_row_instance1_p );
                        tmp.insert( object1_p );
                        RowInstance row_instance{ tmp, object2 };
                        t.insert( row_instance );
                    }
                }
                
                ++i_p;
            }
            
            ++i;
            ++j;
        }
    }

    PRINTLN( SPACES( 20 ) << "<- " << __FUNCTION__ );
    return t;
}

std::map<Pattern, Table> gen_co_occ_inst(const std::map<Pattern, std::pair<Pattern, Pattern>>& c,
                                         const std::map<Pattern, Table>& prev_t,
                                         const std::shared_ptr<INeighborRelation> d) {
    PRINTLN( SPACES( 15 ) << "-> " << __FUNCTION__ );

    std::map<Pattern, Table> t;
    
    for( const auto& pair : c ) {
        const Pattern& candidate_pattern = pair.first;
        const std::pair<Pattern, Pattern>& generating_patterns = pair.second;
        
        const Table& table1 = prev_t.at( generating_patterns.first );
        const Table& table2 = prev_t.at( generating_patterns.second );
        t[candidate_pattern] = join( table1, table2, d );
    }

    PRINTLN( SPACES( 15 ) << "<- " << __FUNCTION__ );
    return t;
}


std::map<Pattern, bool> find_spatial_prev_co_occ(const std::map<Pattern, Table>& t, float p,
                                                 const std::map<EventType, std::set<std::shared_ptr<Object>>>& objects_by_type) {
    PRINTLN( SPACES( 15 ) << "-> " << __FUNCTION__ );
    assert( p >= 0 && p <=1 );
    
    std::map<Pattern, bool> sp;
    
    for ( const auto& pair : t ) {
        // for each pattern
        const Table& table = pair.second;
        
        // divide objects per object type
        std::map<EventType, std::set<unsigned>> object_ids_by_object_type;
        for ( const RowInstance& row_instance : table ) {
            const std::set<std::shared_ptr<Object>>& objects = row_instance.first;
            for ( const std::shared_ptr<Object>& object : objects ) {
                object_ids_by_object_type[object->event_type].insert( object->id );
            }
            
            const std::shared_ptr<Object>& object = row_instance.second;
            if ( object ) { object_ids_by_object_type[object->event_type].insert( object->id ); }
        }
        
        // compute partecipation ratio
        std::map<EventType, float> partecipation_ratio_by_object_type;
        for ( const auto& pair2 : object_ids_by_object_type ) {
            const EventType& type = pair2.first;
            
            float numerator = pair2.second.size();
            float denominator = objects_by_type.at( type ).size();
            assert( numerator > 0 );
            assert( denominator > 0 );
            
            float partecipation_ratio = numerator/denominator;
            assert( partecipation_ratio >= 0 && partecipation_ratio <= 1 );
            
            partecipation_ratio_by_object_type[type] = partecipation_ratio;
        }
        
        // compute partecipation index
        float partecipation_index = std::numeric_limits<float>::max();
        for ( const auto& pair2 : partecipation_ratio_by_object_type ) {
            const float partecipation_ratio = pair2.second;
            if ( partecipation_ratio < partecipation_index ) { partecipation_index = partecipation_ratio; }
        }
        
        // check if partecipation index is above the threshold
        const Pattern& pattern = pair.first;
        if ( partecipation_index != std::numeric_limits<float>::max() && partecipation_index >= p ) {
            sp[pattern] = true;
        }
        else { sp[pattern] = false; }
        PRINTLN( SPACES( 20 ) << pattern << ", P.I. " << partecipation_index );
    }
    
    PRINTLN( SPACES( 15 ) << "<- " << __FUNCTION__ << ": " << sp );
    return sp;
}


std::map<Pattern, float> find_time_index(const std::map<int, std::map<Pattern, bool>>& sp) {
    PRINTLN( SPACES( 15 ) << "-> " << __FUNCTION__ );

    std::map<Pattern, float> tp;
    
    for ( const auto& pair : sp ) {
        // for each time slot
        for ( const auto& pair2 : pair.second ) {
            const Pattern& pattern = pair2.first;
            const bool spatial_prevalent = pair2.second;
            
            if ( !tp.count( pattern ) ) { tp[pattern] = 0.f; }
            if ( spatial_prevalent ) { ++tp[pattern]; }
        }
    }
    
    auto time_slot_count = sp.size();
    for ( auto&& pair : tp ) {
        pair.second /= time_slot_count;
        PRINTLN( SPACES( 20 ) << pair.first << ", T.I. " << pair.second );
    }
    
    PRINTLN( SPACES( 15 ) << "<- " << __FUNCTION__ << ": " << tp );
    return tp;
}


std::set<Pattern> find_time_prev_co_occ(const std::map<Pattern, float>& tp, float time) {
    PRINTLN( SPACES( 15 ) << "-> " << __FUNCTION__ << ": " << time );
    assert( time >= 0 && time <= 1 );
    
    std::set<Pattern> mdp;
    
    for ( const auto& pair : tp ) {
        // for each pattern
        const float time_prevalence_index = pair.second;
        assert( time_prevalence_index >= 0 && time_prevalence_index <= 1 );
        
        if ( time <= time_prevalence_index ) {
            const Pattern& pattern = pair.first;
            mdp.insert( pattern );
        }
    }
    
    PRINTLN( SPACES( 15 ) << "<- " << __FUNCTION__ << ": " << mdp );
    return mdp;
}


std::map<int, std::set<Pattern>> mine_closed_mdcops(const std::set<EventType>& e, const Dataset& st, const std::pair<int, int> tf,
                                                    const std::shared_ptr<INeighborRelation> d, const float spt, const float tpt) {
    assert( !e.empty() );
    assert( !st.objects_by_time_slot.empty() );
    
    int first_time_slot = tf.first, time_slot_count = tf.second;
    assert( first_time_slot >= 0 );
    assert( time_slot_count > 0 );
    assert( first_time_slot + time_slot_count <= st.objects_by_time_slot.size() );

    assert( spt > 0 && spt <= 1 );
    assert( tpt > 0 && tpt <= 1 );
    
    // initialization
    int k = 1;
    
    std::map<int, std::map<int, std::map<Pattern, bool>>> sp;
    for ( int time_slot = first_time_slot; time_slot < first_time_slot + time_slot_count; ++time_slot ) {
        for ( const auto& pair : st.objects_by_event_type ) {
            const EventType& type = pair.first;
            Pattern pattern{ type };
            
            sp[k][time_slot][pattern] = true;
        }
    }
    
    std::map<int, std::map<int, std::map<Pattern, Table>>> t;
    for ( int time_slot = first_time_slot; time_slot < first_time_slot + time_slot_count; ++time_slot ) {
        for ( const auto& pair : st.objects_by_event_type ) {
            const EventType& type = pair.first;
            Pattern pattern{ type };
            
            Table table;
            for ( const std::shared_ptr<Object>& object : st.objects_by_event_type.at( type ) ) {
                if ( object->time_slot == time_slot ) {
                    RowInstance row_instance{ std::set<std::shared_ptr<Object>>{}, { object } };
                    table.insert( row_instance );
                }
            }
            t[k][time_slot][pattern] = table;
        }
    }
    
    std::map<int, std::set<Pattern>> mdp;
    for ( const auto& type : e ) {
        Pattern pattern{ type };
        mdp[k].insert( pattern );
    }

    // algorithm
    while ( !mdp[k].empty() ) {
        std::cout << std::setw( 5 ) << std::left << " " << "Iterating for k=" << k << " (computing k=" << k+1 << ")..." << std::endl;
        
        // 1. generate candidate co-occurrence patterns
        std::map<int, std::map<Pattern, std::pair<Pattern, Pattern>>> c{{ 0, gen_candidate_co_occ( mdp[k] ) }};
            
        // for each time slot
        for ( int time_slot = first_time_slot; time_slot < first_time_slot + time_slot_count; ++time_slot ) {
            std::cout << std::setw( 10 ) << std::left << " " << "Iterating for time_slot=" << time_slot << "..." << std::endl;
            
            // 2. generate spatial co-occurrence row_instances
            const auto& curr_c = c.count( time_slot ) ? c[time_slot] : c[0];
            t[k+1][time_slot] = gen_co_occ_inst( curr_c, t[k][time_slot], d );
            
            // 3. find spatial prevalent co-occurrence patterns
            sp[k+1][time_slot] = find_spatial_prev_co_occ( t[k+1][time_slot], spt, st.objects_by_event_type );
            
            // 4. form a time prevalence table
            const auto tp = find_time_index( sp[k+1] );

            // 5. find mixed-drove co-occurrence patterns
            // prune patterns that will not be time prevalent even if they are spatial prevalent in the remaining time slots
            static const float step = 1.f/time_slot_count;
            float threshold = tpt;
            if ( step * (time_slot_count-time_slot) >= threshold && time_slot_count-time_slot > 1 ) { threshold = 0; }
            mdp[k+1] = find_time_prev_co_occ( tp, threshold );
            
            // candidates for the next time slot are mdcops of the current time slot
            for ( const auto& pattern : mdp[k+1] ) {
                c[time_slot+1][pattern] = c[time_slot][pattern];
            }
        }
        
        std::cout << std::setw( 5 ) << std::left << " " << "MDCOPs found: " << mdp[k+1] << std::endl;
        ++k;
    }
    
    // remove first and last patterns (input pattern and empty pattern)
    mdp.erase( 1 );
    mdp.erase( k );
    return mdp;
}
