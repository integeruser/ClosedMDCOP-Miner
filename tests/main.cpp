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


extern std::map<Pattern, std::pair<Pattern, Pattern>> gen_candidate_co_occ(const std::set<Pattern>&);
TEST_CASE( "gen_candidate_co_occ", "[algorithms]" ) {
    SECTION( "" ) {
        std::set<Pattern> mdp{};
        
        std::map<Pattern, std::pair<Pattern, Pattern>> c = gen_candidate_co_occ( mdp );
        
        std::map<Pattern, std::pair<Pattern, Pattern>> expected_c{};
        REQUIRE( expected_c == c );
    }
    SECTION( "" ) {
        std::set<Pattern> mdp{ { "A" }, { "B" }, { "C" }, { "D" } };
        
        std::map<Pattern, std::pair<Pattern, Pattern>> c = gen_candidate_co_occ( mdp );
        
        std::map<Pattern, std::pair<Pattern, Pattern>> expected_c{
            { { "A", "B" }, { { "A" }, { "B" } } },
            { { "A", "C" }, { { "A" }, { "C" } } },
            { { "A", "D" }, { { "A" }, { "D" } } },
            { { "B", "C" }, { { "B" }, { "C" } } },
            { { "B", "D" }, { { "B" }, { "D" } } },
            { { "C", "D" }, { { "C" }, { "D" } } },
        };
        REQUIRE( expected_c == c );
    }
    SECTION( "" ) {
        std::set<Pattern> mdp{ { "A", "B" }, { "B", "C" } };
        
        std::map<Pattern, std::pair<Pattern, Pattern>> c = gen_candidate_co_occ( mdp );
        
        std::map<Pattern, std::pair<Pattern, Pattern>> expected_c{};
        REQUIRE( expected_c == c );
    }
    SECTION( "" ) {
        std::set<Pattern> mdp{ { "A", "B" }, { "B", "C" }, { "A", "C" } };
        
        std::map<Pattern, std::pair<Pattern, Pattern>> c = gen_candidate_co_occ( mdp );
        
        std::map<Pattern, std::pair<Pattern, Pattern>> expected_c{
            { { "A", "B", "C" }, { { "A", "B" }, { "A", "C" } } }
        };
        REQUIRE( expected_c == c );
    }
    SECTION( "" ) {
        std::set<Pattern> mdp{ { "1", "2", "3" }, { "1", "2", "4" }, { "1", "3", "4" }, { "1", "3", "5" }, { "2", "3", "4" } };
        
        std::map<Pattern, std::pair<Pattern, Pattern>> c = gen_candidate_co_occ( mdp );
        
        std::map<Pattern, std::pair<Pattern, Pattern>> expected_c{
            { { "1", "2", "3", "4" }, { { "1", "2", "3" }, { "1", "2", "4" } } }
        };
        REQUIRE( expected_c == c );
    }
}


std::map<Pattern, Table> gen_co_occ_inst(const std::map<Pattern, std::pair<Pattern, Pattern>>&, const std::map<Pattern, Table>&, const std::shared_ptr<INeighborRelation>);
TEST_CASE( "gen_co_occ_inst", "[algorithm]" ) {
    std::shared_ptr<Object> a1 = std::make_shared<Object>( "A", 1, 1.1f, 1, 0 );
    std::shared_ptr<Object> a2 = std::make_shared<Object>( "A", 2, 2.8f, 2, 0 );
    std::shared_ptr<Object> a3 = std::make_shared<Object>( "A", 3, 3.2f, 2, 0 );
    std::shared_ptr<Object> a4 = std::make_shared<Object>( "A", 4, 2, 3, 0 );
    
    std::shared_ptr<Object> b1 = std::make_shared<Object>( "B", 1, 0, 0.2f, 0 );
    std::shared_ptr<Object> b2 = std::make_shared<Object>( "B", 2, 5, 0.2f, 0 );
    std::shared_ptr<Object> b3 = std::make_shared<Object>( "B", 3, 6.5, 2, 0 );
    std::shared_ptr<Object> b4 = std::make_shared<Object>( "B", 4, 3, 0.5f, 0 );
    std::shared_ptr<Object> b5 = std::make_shared<Object>( "B", 5, 7, 4, 0 );
    
    std::shared_ptr<Object> c1 = std::make_shared<Object>( "C", 1, 3.3f, 0.5f, 0 );
    std::shared_ptr<Object> c2 = std::make_shared<Object>( "C", 2, 0, 2, 0 );
    std::shared_ptr<Object> c3 = std::make_shared<Object>( "C", 3, 6.7f, 3, 0 );
    
    SECTION( "" ) {
        Table t4;
        t4.insert( { { a1 }, b1 } );
        t4.insert( { { a2 }, b4 } );
        t4.insert( { { a3 }, b4 } );
        
        Table t5;
        t5.insert( { { a1 }, c2 } );
        t5.insert( { { a3 }, c1 } );
        
        Table t6;
        t5.insert( { { b2 }, c1 } );
        t5.insert( { { b4 }, c1 } );
        t5.insert( { { b5 }, c3 } );
        
        std::map<Pattern, std::pair<Pattern, Pattern>> c{
            { { "A", "B", "C" }, { { "A", "B" }, { "A", "C" } } }
        };
        
        std::map<Pattern, Table> prev_t;
        prev_t[{ "A", "B" }] = t4;
        prev_t[{ "A", "C" }] = t5;
        prev_t[{ "B", "C" }] = t6;
        
        std::shared_ptr<INeighborRelation> d = std::make_shared<EuclideanDistance>( 0.45f );
        
        std::map<Pattern, Table> t = gen_co_occ_inst( c, prev_t, d );

        std::map<Pattern, Table> expected_t;
        expected_t[{ "A", "B", "C" }].insert( { { a3, b4 }, { c1 } } );
        REQUIRE( expected_t == t );
    }
}


extern std::map<Pattern, bool> find_spatial_prev_co_occ(const std::map<Pattern, Table>&, float, const std::map<EventType, std::set<std::shared_ptr<Object>>>&);
TEST_CASE( "find_spatial_prev_co_occ", "[algorithm]" ) {
    EventType o1{ "A" };
    EventType o2{ "B" };
    
    std::shared_ptr<Object> e1 = std::make_shared<Object>( o1, 1, 0, 0, 0 );
    std::shared_ptr<Object> e2 = std::make_shared<Object>( o1, 2, 0, 0, 0 );
    std::shared_ptr<Object> e3 = std::make_shared<Object>( o2, 1, 0, 0, 0 );
    std::shared_ptr<Object> e4 = std::make_shared<Object>( o2, 2, 0, 0, 0 );
    
    Pattern p1{ o1, o2 };
    
    Table table1;
    table1.insert( { { e1 }, e3 } );
    
    std::map<EventType, std::set<std::shared_ptr<Object>>> objects_by_type;
    objects_by_type[o1].insert( e1 );
    objects_by_type[o1].insert( e2 );
    objects_by_type[o2].insert( e3 );
    objects_by_type[o2].insert( e4 );
    
    SECTION( "" ) {
        std::map<Pattern, Table> t1;
        t1[p1] = table1;
        
        SECTION( "" ) {
            float p = 0;
            
            std::map<Pattern, bool> result = find_spatial_prev_co_occ( t1, p, objects_by_type );
            
            std::map<Pattern, bool> expected_result{ { p1, true } };
            REQUIRE( expected_result == result );
        }
        SECTION( "" ) {
            float p = 0.4;
            
            std::map<Pattern, bool> result = find_spatial_prev_co_occ( t1, p, objects_by_type );
            
            std::map<Pattern, bool> expected_result{ { p1, true } };
            REQUIRE( expected_result == result );
        }
        SECTION( "" ) {
            float p = 0.5;
            
            std::map<Pattern, bool> result = find_spatial_prev_co_occ( t1, p, objects_by_type );
            
            std::map<Pattern, bool> expected_result{ { p1, true } };
            REQUIRE( expected_result == result );
        }
        SECTION( "" ) {
            float p = 0.6;
            
            std::map<Pattern, bool> result = find_spatial_prev_co_occ( t1, p, objects_by_type );
            
            std::map<Pattern, bool> expected_result{ { p1, false } };
            REQUIRE( expected_result == result );
        }
        SECTION( "" ) {
            float p = 1;
            
            std::map<Pattern, bool> result = find_spatial_prev_co_occ( t1, p, objects_by_type );
            
            std::map<Pattern, bool> expected_result{ { p1, false } };
            REQUIRE( expected_result == result );
        }
    }
}


extern std::map<Pattern, float> find_time_index(const std::map<int, std::map<Pattern, bool>>&);
TEST_CASE( "find_time_index", "[algorithm]" ) {
    EventType o1{ "A" };
    EventType o2{ "B" };
    EventType o3{ "C" };
    
    SECTION( "" ) {
        Pattern p1{ o1, o2 };
        Pattern p2{ o2, o3 };

        std::map<int, std::map<Pattern, bool>> sp;
        sp[0][p1] = true;
        sp[0][p2] = true;
        sp[1][p1] = true;
        
        std::map<Pattern, float> result = find_time_index( sp );

        std::map<Pattern, float> expected_result;
        expected_result[p1] = 1;
        expected_result[p2] = 0.5f;
        
        REQUIRE( expected_result == result );
    }
}


extern std::set<Pattern> find_time_prev_co_occ(const std::map<Pattern, float>&, float);
TEST_CASE( "find_time_prev_co_occ", "[algorithms]" ) {
    EventType o1{ "A" };
    EventType o2{ "B" };
    EventType o3{ "C" };
    
    SECTION( "" ) {
        Pattern p1{ o1, o2 };
        Pattern p2{ o2, o3 };
        
        std::map<Pattern, float> tp;
        tp[p1] = 0.5;
        tp[p2] = 0.4;
       
        SECTION( "" ) {
            float time = 1.f;
            
            std::set<Pattern> result = find_time_prev_co_occ( tp, time );
            
            std::set<Pattern> expected_result{};
            REQUIRE( expected_result == result );
        }
        SECTION( "" ) {
            float time = 0.4f;
            
            std::set<Pattern> result = find_time_prev_co_occ( tp, time );
            
            std::set<Pattern> expected_result{ p1, p2 };
            REQUIRE( expected_result == result );
        }
        SECTION( "" ) {
            float time = 0.5f;
            
            std::set<Pattern> result = find_time_prev_co_occ( tp, time );
            
            std::set<Pattern> expected_result{ p1 };
            REQUIRE( expected_result == result );
        }
    }
}
