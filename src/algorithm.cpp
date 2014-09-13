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
#include "dataset.hpp"
#include "object.hpp"


#ifdef DEBUG
#define SPACES(i) std::setw( i ) << std::left << " "
#define PRINT(os) do { std::cout << os << std::flush; } while ( false )
#define PRINTLN(os) do { std::cout << os << std::endl; } while ( false )
#else
#define SPACES(i)
#define PRINT(os) do {} while ( false )
#define PRINTLN(os) do {} while ( false )
#endif


bool exist_all_subsets(const Pattern& superset_pattern, const std::set<Pattern>& patterns) {
    // check if patterns contains all subsets of superset_pattern
    
    const size_t subset_count = superset_pattern.size();
    
    unsigned existing_subset_count = 0;
    for ( const Pattern& pattern : patterns ) {
        if ( std::includes( superset_pattern.begin(), superset_pattern.end(), pattern.begin(), pattern.end() ) ) {
            // pattern is included in superset_pattern: increment the count of subsets found
            ++existing_subset_count;
        }
    }
    
    assert( existing_subset_count <= subset_count );
    return existing_subset_count == subset_count;
}
bool exist_all_subsets(const Pattern& superset_pattern, const std::map<Pattern, SubPatterns>& patterns_with_subpatterns) {
    std::set<Pattern> patterns;
    
    for ( const auto& pair : patterns_with_subpatterns ) {
        const Pattern& pattern = pair.first;
        
        patterns.insert( pattern );
    }
    
    return exist_all_subsets( superset_pattern, patterns );
}

std::map<Pattern, SubPatterns> apriori_gen(const std::set<Pattern>& patterns) {
    PRINTLN( SPACES( 15 ) << "-> " << __FUNCTION__ << ": " << patterns );

    // given a set of patterns of size k, generate superset patterns of size k+1
    
    std::map<Pattern, SubPatterns> superset_patterns;
    
    // join step
    for ( auto i = patterns.cbegin(); i != patterns.cend(); ++i ) {
        const Pattern& pattern1 = *i;
        
        for ( auto j = i; j != patterns.cend(); ++j ) {
            const Pattern& pattern2 = *j;
            assert( pattern1.size() == pattern2.size() );
            
            // check if the first k-1 event types of pattern1 and pattern2 are identical
            
            assert( pattern1.size() == pattern2.size() );
            const size_t k = pattern1.size();
            
            bool first_k_event_types_identical = true;
            auto m = pattern1.cbegin(), n = pattern2.cbegin();
            for ( int o = 0; o < k-1; ++o ) {
                const EventType event_type1 = *m;
                const EventType event_type2 = *n;
                
                if ( event_type1 != event_type2 ) {
                    first_k_event_types_identical = false;
                    break;
                }
                
                ++m;
                ++n;
            }
            if ( first_k_event_types_identical ) {
                const EventType last_event_type1 = *m;
                const EventType last_event_type2 = *n;
                
                // check if the last event type of pattern1 is less then the last element of pattern2
                if ( last_event_type1 < last_event_type2 ) {
                    // join pattern1 and pattern2
                    Pattern superset_pattern;
                    std::set_union( pattern1.begin(), pattern1.end(), pattern2.begin(), pattern2.end(),
                                   std::inserter( superset_pattern, superset_pattern.end() ) );
                    
                    superset_patterns.insert( { superset_pattern, { pattern1, pattern2 } } );
                }
            }
        }
    }
    
    // prune step: delete all generated patterns of size k+1 if at least one of its subsets of size k doesn't exist in patterns
    for ( auto i = superset_patterns.cbegin(); i != superset_patterns.cend(); ) {
        const Pattern& superset_pattern = (*i).first;
        
        if ( exist_all_subsets( superset_pattern, patterns ) ) { ++i; }
        else { superset_patterns.erase( i++ ); }
    }
    
    PRINTLN( SPACES( 15 ) << "<- " << __FUNCTION__ << ": " << superset_patterns );
    return superset_patterns;
}

std::map<TimeSlot, std::map<Pattern, SubPatterns>> gen_candidate_co_occ(const std::map<TimeSlot, std::map<Pattern, SubPatterns>> prev_c,
                                                                        const std::set<Pattern>& mdp) {
    PRINTLN( SPACES( 10 ) << "-> " << __FUNCTION__ );
    assert ( std::adjacent_find( mdp.cbegin(), mdp.cend(), [](const Pattern& pattern1, const Pattern& pattern2) {
        return pattern1.size() != pattern2.size();
    } ) == mdp.end() );
    
    // generate patterns of size k+1 using mdcops of size k
    std::map<Pattern, SubPatterns> candidate_patterns = apriori_gen( mdp );

    std::map<TimeSlot, std::map<Pattern, SubPatterns>> c;

    // prev_c contains - for each time slot - the candidate patterns found to be spatial prevalent patterns
    
    // for each time slot
    for ( const auto& pair : prev_c ) {
        const TimeSlot time_slot = pair.first;
        const std::map<Pattern, SubPatterns>& prev_candidate_patterns = pair.second;
        
        // for each candidate pattern of size k+1, check if all its subsets of size k exist
        for ( const auto& pair : candidate_patterns ) {
            const Pattern& candidate_pattern = pair.first;
            const SubPatterns& subpatterns = pair.second;

            if ( exist_all_subsets( candidate_pattern, prev_candidate_patterns ) ) { c[time_slot].insert( { candidate_pattern, subpatterns } ); }
        }
    }
    
    PRINTLN( SPACES( 10 ) << "<- " << __FUNCTION__ << ": " << c );
    return c;
}


TableInstance join(const TableInstance& table1, const TableInstance& table2, const std::shared_ptr<INeighborRelation> d) {
    PRINTLN( SPACES( 20 ) << "-> " << __FUNCTION__ );

    // each table is a map which contains all the instances of size k:
    //  - the key is a set of objects that represent the first common k-1 objects of some instances
    //  - the value is a set of objects which represent the k-nth element of each instance
    // e.g
    // key: { a1, b1, c1 }
    // values { d1, d2, d5 }
    // the instances referenced by this key are: { a1, b1, c1, d1 }, { a1, b1, c1, d2 }, { a1, b1, c1, d5 }
    
    TableInstance table;
    
    for( const auto& pair1 : table1 ) {
        const std::set<std::shared_ptr<Object>>& pair1_first_common_objects = pair1.first;
        const std::set<std::shared_ptr<Object>>& pair1_last_objects = pair1.second;
        
        for( const auto& pair2 : table2 ) {
            const std::set<std::shared_ptr<Object>>& pair2_first_common_objects = pair2.first;
            const std::set<std::shared_ptr<Object>>& pair2_last_objects = pair2.second;
            
            if ( pair1_first_common_objects == pair2_first_common_objects ) {
                // for each possible combinations, check which objects are neighbors
                for ( const std::shared_ptr<Object>& object1 : pair1_last_objects ) {
                    std::set<std::shared_ptr<Object>> new_first_common_objects{ pair1_first_common_objects };
                    new_first_common_objects.insert( object1 );
                    
                    for ( const std::shared_ptr<Object>& object2 : pair2_last_objects ) {
                        assert( object1->event_type != object2->event_type );
                        
                        if ( d->neighbors( object1, object2 ) ) {
                            table[new_first_common_objects].insert( object2 );
                        }
                    }
                }
            }
        }
    }
    
    PRINTLN( SPACES( 20 ) << "<- " << __FUNCTION__ );
    return table;
}

std::map<Pattern, TableInstance> gen_co_occ_inst(const std::map<Pattern, SubPatterns>& c, const std::map<Pattern, TableInstance>& prev_t,
                                                 const std::shared_ptr<INeighborRelation> d) {
    PRINTLN( SPACES( 15 ) << "-> " << __FUNCTION__ );

    // generate instances by joining the tables of the two subpatterns
    
    std::map<Pattern, TableInstance> t;
    
    for( const auto& pair : c ) {
        const Pattern& candidate_pattern = pair.first;
        
        const SubPatterns& subpatterns = pair.second;
        const TableInstance& subpatterns_table1 = prev_t.at( subpatterns.first );
        const TableInstance& subpatterns_table2 = prev_t.at( subpatterns.second );
        t[candidate_pattern] = join( subpatterns_table1, subpatterns_table2, d );
    }

    PRINTLN( SPACES( 15 ) << "<- " << __FUNCTION__ );
    return t;
}


std::set<Pattern> find_spatial_prev_co_occ(const std::map<EventType, std::set<std::shared_ptr<Object>>>& objects_by_event_type,
                                           const std::map<Pattern, TableInstance>& t, float spt,
                                           std::map<Pattern, std::vector<float>>& indexes_by_pattern) {
    PRINTLN( SPACES( 15 ) << "-> " << __FUNCTION__ );
    assert( spt > 0 && spt <=1 );
    
    std::set<Pattern> sp;
    
    for ( const auto& pair : t ) {
        // for each pattern
        const TableInstance& table = pair.second;
        
        // divide objects per object type
        std::map<EventType, std::set<unsigned>> ids_by_event_type;
        for ( const auto& pair : table ) {
            const std::set<std::shared_ptr<Object>>& instances_common_objects = pair.first;
            
            for ( const std::shared_ptr<Object>& object : instances_common_objects ) {
                ids_by_event_type[object->event_type].insert( object->id );
            }

            const std::set<std::shared_ptr<Object>>& instances_last_objects = pair.second;
            for ( const std::shared_ptr<Object>& object : instances_last_objects ) {
                ids_by_event_type[object->event_type].insert( object->id );
            }
        }
        
        // compute partecipation ratio
        std::map<EventType, float> partecipation_ratio_by_event_type;
        for ( const auto& pair2 : ids_by_event_type ) {
            const EventType& event_type = pair2.first;
            
            float numerator = pair2.second.size();
            float denominator = objects_by_event_type.at( event_type ).size();
            assert( numerator > 0 );
            assert( denominator > 0 );
            
            float partecipation_ratio = numerator/denominator;
            assert( partecipation_ratio >= 0 && partecipation_ratio <= 1 );
            
            partecipation_ratio_by_event_type[event_type] = partecipation_ratio;
        }
        
        // compute partecipation index
        float partecipation_index = std::numeric_limits<float>::max();
        for ( const auto& pair2 : partecipation_ratio_by_event_type ) {
            const float partecipation_ratio = pair2.second;
            if ( partecipation_ratio < partecipation_index ) { partecipation_index = partecipation_ratio; }
        }
        
        // check if partecipation index is above the threshold
        const Pattern& pattern = pair.first;
        if ( partecipation_index != std::numeric_limits<float>::max() && partecipation_index >= spt ) { sp.insert( pattern ); }
        PRINTLN( SPACES( 20 ) << pattern << ", P.I. " << partecipation_index );
        
        // update the indexes table
        indexes_by_pattern[pattern].push_back( partecipation_index );
    }
    
    PRINTLN( SPACES( 15 ) << "<- " << __FUNCTION__ << ": " << sp );
    return sp;
}


void find_time_index(std::map<Pattern, float>& tp, const std::set<Pattern>& sp, const unsigned time_slot_count) {
    PRINTLN( SPACES( 15 ) << "-> " << __FUNCTION__ );
    
    for ( const Pattern& pattern : sp ) {
        assert( tp.count( pattern ) );
        tp[pattern] += 1.f/time_slot_count;
    }
    
    PRINTLN( SPACES( 15 ) << "<- " << __FUNCTION__ );
}


std::set<Pattern> find_time_prev_co_occ(std::map<Pattern, float>& tp, const float tpt,
                                        const unsigned time_slot_count, const unsigned time_slot) {
    PRINTLN( SPACES( 15 ) << "-> " << __FUNCTION__ << ": " << tp );
    assert( tpt > 0 && tpt <= 1 );
    assert( time_slot_count > time_slot );
    
    // a (spatial prevalent) pattern is time prevalent if its time index is greater or equal than the threshold tpt

    std::set<Pattern> mdp;
    
    for ( auto i = tp.cbegin(); i != tp.cend(); ) {
        const Pattern& pattern = (*i).first;
        
        const float pattern_time_index = (*i).second;
        assert( pattern_time_index >= 0 && pattern_time_index <= 1 );
        
        if ( pattern_time_index >= tpt ) {
            // the pattern is time prevalent
            mdp.insert( pattern );
            ++i;
        }
        else {
            // check if the pattern can be time prevalent in the remaining time slots
            const unsigned remaining_time_slots = time_slot_count-time_slot-1;
            const float pattern_max_possible_time_index = pattern_time_index + (1.f/time_slot_count * remaining_time_slots);
            if ( pattern_max_possible_time_index >= tpt ) {
                // the pattern is not time prevalent in this time slot, but can be time prevalent in the remainings time slots
                mdp.insert( pattern );
                ++i;
            }
            else {
                // even if the pattern is time prevalent in all the remaining time slots, its time index will not be greater or equal than tpt
                // the pattern can be pruned from the time prevalence table
                tp.erase( i++ );
            }
        }
    }
    
    PRINTLN( SPACES( 15 ) << "<- " << __FUNCTION__ << ": " << mdp );
    return mdp;
}


void prune_non_closed(std::map<size_t, std::set<Pattern>>& cmdp, const size_t l,
                      const std::map<Pattern, std::vector<float>>& indexes_by_pattern) {
    // prune non closed patterns
    
    if ( l <= 2 ) { return; }
    
    // for each pattern of size l-1
    for ( auto i = cmdp[l-1].cbegin(); i != cmdp[l-1].cend(); ) {
        const Pattern& pattern = *i;
        
        // check if exist at least one superset with identical partecipation indexes
        bool exist_superset_identical_partecipation_indexes = false;
        
        for ( const Pattern& pattern2 : cmdp[l] ) {
            if ( std::includes( pattern2.cbegin(), pattern2.cend(), pattern.cbegin(), pattern.cend() ) ) {
                const std::vector<float>& pattern_partecipation_indexes = indexes_by_pattern.at( pattern );
                const std::vector<float>& pattern2_partecipation_indexes = indexes_by_pattern.at( pattern2 );
                
                if ( pattern_partecipation_indexes == pattern2_partecipation_indexes ) {
                    exist_superset_identical_partecipation_indexes = true;
                    break;
                }
            }
        }
        
        // if a superset with identical partecipation indexes exists, the pattern is not closed: delete it
        if ( exist_superset_identical_partecipation_indexes ) { cmdp[l-1].erase( i++ ); }
        else { ++i; }
    }
}


std::map<size_t, std::set<Pattern>> mine_closed_mdcops(const std::set<EventType>& e, const Dataset& st,
                                                       const std::pair<unsigned, unsigned> tf,
                                                       const std::shared_ptr<INeighborRelation> r,
                                                       const float spt, const float tpt) {
    unsigned first_time_slot = tf.first, time_slot_count = tf.second;
    assert( first_time_slot >= 0 );
    assert( time_slot_count > 0 );
    assert( first_time_slot + time_slot_count <= st.objects_by_time_slot.size() );

    assert( spt > 0 && spt <= 1 );
    assert( tpt > 0 && tpt <= 1 );
    
    // initialization
    int k = 1;  // current pattern size
    
    std::map<size_t, std::set<Pattern>> cmdp;  // closed mdcops
    for ( const auto& event_type : e ) {
        const Pattern pattern{ event_type };
        cmdp[k].insert( pattern );
    }
    
    std::map<size_t, std::map<TimeSlot, std::map<Pattern, SubPatterns>>> c;
    for ( int time_slot = first_time_slot; time_slot < first_time_slot + time_slot_count; ++time_slot ) {
        for ( const auto& event_type : e ) {
            const Pattern pattern{ event_type };
            c[k][time_slot].insert( { pattern, {} } );
        }
    }
    
    std::map<size_t, std::map<TimeSlot, std::map<Pattern, TableInstance>>> t;  // pattern instances
    for ( int time_slot = first_time_slot; time_slot < first_time_slot + time_slot_count; ++time_slot ) {
        for ( const auto& pair : st.objects_by_event_type ) {
            const EventType& event_type = pair.first;
            
            const Pattern pattern{ event_type };
            
            TableInstance table;
            for ( const std::shared_ptr<Object>& object : st.objects_by_event_type.at( event_type ) ) {
                if ( object->time_slot == time_slot ) {
                    table[std::set<std::shared_ptr<Object>>{}].insert( object );
                }
            }
            
            t[k][time_slot][pattern] = table;
        }
    }
    
    std::map<Pattern, std::vector<float>> indexes_by_pattern;  // pattern spatial indexes
    
    // algorithm
    while ( !cmdp[k].empty() ) {
        // compute mdcops of size k+1
        std::cout << std::setw( 5 ) << std::left << " " << "Iterating for k=" << k << " (computing k=" << k+1 << ")..." << std::endl;
        
        // 1. generate candidate patterns of size k+1 from mdcops of size k
        c[k+1] = gen_candidate_co_occ( c[k], cmdp[k] );
        
        // initialize the time prevalence table
        std::map<Pattern, float> tp;
        for ( const auto& pair : c[k+1] ) {
            for ( const auto& pair2 : pair.second ) {
                const Pattern& candidate_pattern = pair2.first;
                tp[candidate_pattern] = 0.f;
            }
        }
        
        // for each time slot
        for ( TimeSlot time_slot = first_time_slot; time_slot < first_time_slot+time_slot_count; ++time_slot ) {
            std::cout << std::setw( 10 ) << std::left << " " << "Iterating for time_slot=" << time_slot << "..." << std::endl;
            PRINTLN( SPACES( 15 ) << "c: " << c[k+1][time_slot] );
            
            // 2. given a set of candidate patterns, generate spatial row instances of size k by reusing instances of size k+1
            t[k+1][time_slot] = gen_co_occ_inst( c[k+1][time_slot], t[k][time_slot], r );
            
            // 3. find the spatial prevalent patterns from the row instances
            const std::set<Pattern> sp = find_spatial_prev_co_occ( st.objects_by_event_type, t[k+1][time_slot], spt, indexes_by_pattern );
            
            // from all candidate patterns used in this time slot, remove the candidates which are not spatial prevalent
            for ( auto i = c[k+1][time_slot].begin(); i != c[k+1][time_slot].end(); ) {
                const Pattern& pattern = (*i).first;
                
                if ( !sp.count( pattern ) ) { c[k+1][time_slot].erase( i++ ); }
                else { ++i; }
            }
            
            // 4. update the time prevalence table using the spatial prevalent patterns
            find_time_index( tp, sp, time_slot_count );

            // 5. find time prevalent patterns from the time prevalence table (also prune patterns from the time prevalence table that will
            // not be time prevalent even if they are spatial prevalent in the remaining time slots)
            cmdp[k+1] = find_time_prev_co_occ( tp, tpt, time_slot_count, time_slot );
        }
        
        // after processing the last time slot, cmdp[k+1] contains all the mdcops of size k+1
        std::cout << std::setw( 5 ) << std::left << " " << "MDCOPs found: " << cmdp[k+1] << std::endl;

        // having mdcops of size k+1, it is possible to prune all mdcops of size k which are not closed mdcops
        prune_non_closed( cmdp, k+1, indexes_by_pattern );
        
        ++k;
    }
        
    // remove patterns of length 1 and of length k (empty set)
    cmdp.erase( 1 );
    cmdp.erase( k );
    return cmdp;
}
