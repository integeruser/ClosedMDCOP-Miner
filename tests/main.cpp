#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <set>
#include <map>
#include <utility>
#include <vector>

#include "catch.hpp"
#include "prettyprint.hpp"

#include "algorithm.hpp"
#include "distances.hpp"
#include "object.hpp"


extern std::map<Pattern, SubPatterns> apriori_gen(const std::set<Pattern>&);
TEST_CASE( "apriori_gen", "[algorithm]" ) {
    EventType a{ "A" };
    EventType b{ "B" };
    EventType c{ "C" };
    EventType d{ "D" };
    
    SECTION( "" ) {
        std::set<Pattern> mdp{};
        
        std::map<Pattern, SubPatterns> candidate_patterns = apriori_gen( mdp );
        
        std::map<Pattern, SubPatterns> expected_candidate_patterns{};
        REQUIRE( expected_candidate_patterns == candidate_patterns );
    }
    SECTION( "" ) {
        std::set<Pattern> mdp{ { a }, { b }, { c }, { d } };
        
        std::map<Pattern, SubPatterns> candidate_patterns = apriori_gen( mdp );
        
        std::map<Pattern, SubPatterns> expected_candidate_patterns{
            { { a, b }, { { a }, { b } } },
            { { a, c }, { { a }, { c } } },
            { { a, d }, { { a }, { d } } },
            { { b, c }, { { b }, { c } } },
            { { b, d }, { { b }, { d } } },
            { { c, d }, { { c }, { d } } },
        };
        REQUIRE( expected_candidate_patterns == candidate_patterns );
    }
    SECTION( "" ) {
        std::set<Pattern> mdp{ { a, b }, { b, c } };
        
        std::map<Pattern, SubPatterns> candidate_patterns = apriori_gen( mdp );
        
        std::map<Pattern, SubPatterns> expected_candidate_patterns{};
        REQUIRE( expected_candidate_patterns == candidate_patterns );
    }
    SECTION( "" ) {
        std::set<Pattern> mdp{ { a, b }, { b, c }, { a, c } };
        
        std::map<Pattern, SubPatterns> candidate_patterns = apriori_gen( mdp );
        
        std::map<Pattern, SubPatterns> expected_candidate_patterns{
            { { a, b, c }, { { a, b }, { a, c } } }
        };
        REQUIRE( expected_candidate_patterns == candidate_patterns );
    }
    SECTION( "" ) {
        std::set<Pattern> mdp{ { "1", "2", "3" }, { "1", "2", "4" }, { "1", "3", "4" }, { "1", "3", "5" }, { "2", "3", "4" } };
        
        std::map<Pattern, SubPatterns> candidate_patterns = apriori_gen( mdp );
        
        std::map<Pattern, SubPatterns> expected_candidate_patterns{
            { { "1", "2", "3", "4" }, { { "1", "2", "3" }, { "1", "2", "4" } } },
        };
        REQUIRE( expected_candidate_patterns == candidate_patterns );
    }
}


extern std::map<Pattern, Table> gen_co_occ_inst(const std::map<Pattern, SubPatterns>&, const std::map<Pattern, Table>&, const std::shared_ptr<INeighborRelation>);
TEST_CASE( "gen_co_occ_inst", "[algorithm]" ) {
    const EventType a{ "A" };
    const EventType b{ "B" };
    const EventType c{ "C" };
    
    const std::shared_ptr<Object> a1 = std::make_shared<Object>( a, 1, 1.1f, 1, 0 );
    const std::shared_ptr<Object> a2 = std::make_shared<Object>( a, 2, 2.8f, 2, 0 );
    const std::shared_ptr<Object> a3 = std::make_shared<Object>( a, 3, 3.2f, 2, 0 );
    const std::shared_ptr<Object> a4 = std::make_shared<Object>( a, 4, 2, 3, 0 );
    
    const std::shared_ptr<Object> b1 = std::make_shared<Object>( b, 1, 0, 0.2f, 0 );
    const std::shared_ptr<Object> b2 = std::make_shared<Object>( b, 2, 5, 0.2f, 0 );
    const std::shared_ptr<Object> b3 = std::make_shared<Object>( b, 3, 6.5, 2, 0 );
    const std::shared_ptr<Object> b4 = std::make_shared<Object>( b, 4, 3, 0.5f, 0 );
    const std::shared_ptr<Object> b5 = std::make_shared<Object>( b, 5, 7, 4, 0 );
    
    const std::shared_ptr<Object> c1 = std::make_shared<Object>( c, 1, 3.3f, 0.5f, 0 );
    const std::shared_ptr<Object> c2 = std::make_shared<Object>( c, 2, 0, 2, 0 );
    const std::shared_ptr<Object> c3 = std::make_shared<Object>( c, 3, 6.7f, 3, 0 );
    
    SECTION( "" ) {
        const Table table4{
            { { a1 }, b1 },
            { { a2 }, b4 },
            { { a3 }, b4 },
        };
        
        const Table table5{
            { { a1 }, c2 },
            { { a3 }, c1 },
        };
        
        const Table table6{
            { { b2 }, c1 },
            { { b4 }, c1 },
            { { b5 }, c3 },
        };
        
        const std::map<Pattern, SubPatterns> candidate_patterns{
            { { a, b, c }, { { a, b }, { a, c } } }
        };
        
        const std::map<Pattern, Table> prev_t{
            { { a, b }, table4 },
            { { a, c }, table5 },
            { { b, c }, table6 },
        };
        
        const std::shared_ptr<INeighborRelation> d = std::make_shared<EuclideanDistance>( 0.45f );
        
        const std::map<Pattern, Table> t = gen_co_occ_inst( candidate_patterns, prev_t, d );

        std::map<Pattern, Table> expected_t;
        expected_t[{ a, b, c }].insert( { { a3, b4 }, { c1 } } );
        REQUIRE( expected_t == t );
    }
}


extern std::set<Pattern> find_spatial_prev_co_occ(const std::map<EventType, std::set<std::shared_ptr<Object>>>&, const std::map<Pattern, Table>&, float, std::map<Pattern, std::vector<float>>&);
TEST_CASE( "find_spatial_prev_co_occ", "[algorithm]" ) {
    const EventType a{ "A" };
    const EventType b{ "B" };
    
    const std::shared_ptr<Object> a1 = std::make_shared<Object>( a, 1, 0, 0, 0 );
    const std::shared_ptr<Object> a2 = std::make_shared<Object>( a, 2, 0, 0, 0 );
    const std::shared_ptr<Object> b1 = std::make_shared<Object>( b, 1, 0, 0, 0 );
    const std::shared_ptr<Object> b2 = std::make_shared<Object>( b, 2, 0, 0, 0 );
    
    const Pattern p1{ a, b };
    
    const Table table1{
        { { a1 }, b1 },
    };
    
    const std::map<EventType, std::set<std::shared_ptr<Object>>> objects_by_type{
        { a, { a1, a2 } },
        { b, { b1, b2 } },
    };
    
    SECTION( "" ) {
        const std::map<Pattern, Table> t1{
            { p1, table1 },
        };
        
        SECTION( "" ) {
            const float spt = 0.4;
            
            std::map<Pattern, std::vector<float>> indexes_by_pattern;
            
            const std::set<Pattern> result = find_spatial_prev_co_occ( objects_by_type, t1, spt, indexes_by_pattern );
            
            const std::set<Pattern> expected_result{ p1 };
            REQUIRE( expected_result == result );
        }
        SECTION( "" ) {
            const float spt = 0.5;
            
            std::map<Pattern, std::vector<float>> indexes_by_pattern;
            
            const std::set<Pattern> result = find_spatial_prev_co_occ( objects_by_type, t1, spt, indexes_by_pattern );
            
            const std::set<Pattern> expected_result{ p1 };
            REQUIRE( expected_result == result );
        }
        SECTION( "" ) {
            const float spt = 0.6;
            
            std::map<Pattern, std::vector<float>> indexes_by_pattern;
            
            const std::set<Pattern> result = find_spatial_prev_co_occ( objects_by_type, t1, spt, indexes_by_pattern );
            
            const std::set<Pattern> expected_result{};
            REQUIRE( expected_result == result );
        }
        SECTION( "" ) {
            const float spt = 1;
            
            std::map<Pattern, std::vector<float>> indexes_by_pattern;
            
            const std::set<Pattern> result = find_spatial_prev_co_occ( objects_by_type, t1, spt, indexes_by_pattern );
            
            const std::set<Pattern> expected_result{};
            REQUIRE( expected_result == result );
        }
    }
}


extern void find_time_index(std::map<Pattern, float>& tp, const std::set<Pattern>& sp, const unsigned time_slot_count);
TEST_CASE( "find_time_index", "[algorithm]" ) {
    const EventType a{ "A" };
    const EventType b{ "B" };
    const EventType c{ "C" };
    
    SECTION( "" ) {
        const Pattern p1{ a, b };
        const Pattern p2{ b, c };
        
        const unsigned time_slot_count = 2;
        
        std::map<Pattern, float> tp{
            { p1, 1.f/time_slot_count },
            { p2, 1.f/time_slot_count },
        };
        
        const std::set<Pattern> sp{
            p1,
        };
        
        find_time_index( tp, sp, time_slot_count );
        
        const std::map<Pattern, float> expected_tp{
            { p1, 1.f },
            { p2, 0.5f },
        };
        REQUIRE( expected_tp == tp );
    }
}


extern std::set<Pattern> find_time_prev_co_occ(std::map<Pattern, float>& tp, const float tpt, const unsigned time_slot_count, const unsigned time_slot);
TEST_CASE( "find_time_prev_co_occ", "[algorithm]" ) {
    const EventType a{ "A" };
    const EventType b{ "B" };
    const EventType c{ "C" };
    
    SECTION( "" ) {
        const Pattern p1{ a, b };
        const Pattern p2{ b, c };
        
        std::map<Pattern, float> tp{
            { p1, 0.5f },
            { p2, 0.4f },
        };

        const unsigned time_slot_count = 1;
        
        const unsigned time_slot = 0;
        
        SECTION( "" ) {
            const float tpt = 1.f;
            
            const std::set<Pattern> mdp = find_time_prev_co_occ( tp, tpt, time_slot_count, time_slot );
            
            const std::set<Pattern> expected_mdp{};
            REQUIRE( expected_mdp == mdp );
        }
        SECTION( "" ) {
            const float tpt = 0.4f;
            
            const std::set<Pattern> mdp = find_time_prev_co_occ( tp, tpt, time_slot_count, time_slot );
            
            const std::set<Pattern> expected_mdp{ p1, p2 };
            REQUIRE( expected_mdp == mdp );
        }
        SECTION( "" ) {
            const float tpt = 0.5f;
            
            const std::set<Pattern> mdp = find_time_prev_co_occ( tp, tpt, time_slot_count, time_slot );
            
            const std::set<Pattern> expected_mdp{ p1 };
            REQUIRE( expected_mdp == mdp );
        }
    }
}
