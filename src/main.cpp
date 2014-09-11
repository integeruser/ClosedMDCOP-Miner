#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "prettyprint.hpp"

#include "algorithm.hpp"
#include "dataset.hpp"
#include "distances.hpp"


bool validate_arguments(std::string dataset_file_path, int time_slot_s, int time_slot_n, std::string distance, float dt, float spt, float tpt) {
    std::ifstream dataset_file ( dataset_file_path );
    if ( !dataset_file ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Failed to open dataset_file: " << dataset_file_path << std::endl;
        return false;
    }
    if ( time_slot_s < 0 ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Invalid time_slot_s: " << time_slot_s << std::endl;
        return false;
    }
    if ( time_slot_n <= 0 ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Invalid time_slot_n: " << time_slot_n << std::endl;
        return false;
    }
    if ( distance != "euclidean" && distance != "latlon" ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Invalid distance: " << distance << std::endl;
        return false;
    }
    if ( dt <= 0 ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Invalid dt: " << dt << std::endl;
        return false;
    }
    if ( spt <= 0 || spt > 1 ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Invalid spt: " << spt << std::endl;
        return false;
    }
    if ( tpt <= 0 || tpt > 1 ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Invalid tpt: " << tpt << std::endl;
        return false;
    }

    return true;
}

int main(int argc, const char *argv[]) {
    // validate arguments
    std::cout << "Validating arguments..." << std::endl;
    
    if ( argc != 1+7 ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Invalid number of arguments" << std::endl;
        std::cerr << std::endl;
        
        std::cerr << "Usage: ClosedMDCOP dataset_file_path time_slot_s time_slot_n distance distance_threshold p time" << std::endl;
        std::cerr << "Parameters:" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "dataset_file_path: the dataset file" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "time_slot_s: the starting time slot" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "time_slot_n: the number of time slots to mine" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "distance: the distance function to use ('euclidean' or 'latlon')" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "dt: the maximum distance for considering two objects as neighbors" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "spt: the spatial prevalence threshold (0 < p <= 1)" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "tpt: the time prevalence threshold (0 < time <= 1)" << std::endl;
        std::cerr << "Example: ClosedMDCOP dataset.txt 0 3 latlon 2 0.3 0.2" << std::endl;
        return EXIT_FAILURE;
    }
    
    std::string dataset_file_path = argv[1];
    int time_slot_s = std::stoi( argv[2] );
    int time_slot_n = std::stoi( argv[3] );
    std::string distance = argv[4];
    float dt = std::stof( argv[5] );
    float spt = std::stof( argv[6] );
    float tpt = std::stof( argv[7] );
    if ( !validate_arguments( dataset_file_path, time_slot_s, time_slot_n, distance, dt, spt, tpt ) ) { return EXIT_FAILURE; }

    std::cout << std::setw( 5 ) << std::left << " " << "dataset_file_path: " << dataset_file_path << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "time_slot_s: " << time_slot_s << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "time_slot_n: " << time_slot_n << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "distance: " << distance << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "dt: " << dt << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "spt: " << spt << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "tpt: " << tpt << std::endl;
    std::cout << std::endl;
    
    // construct dataset
    std::cout << "Constructing dataset from '" << dataset_file_path << "'..." << std::endl;
    std::ifstream dataset_file ( dataset_file_path );
    Dataset dataset = construct_dataset( dataset_file );
    print_dataset_info( dataset );
    std::cout << std::endl;
    
    // re-validate time slots arguments after dataset parsing
    std::cout << "Clamping time slots..." << std::endl;
    int time_slot_count = (int) dataset.objects_by_time_slot.size();
    time_slot_s = std::min( time_slot_s, time_slot_count-1 );
    time_slot_n = std::min( time_slot_n, time_slot_count-time_slot_s );
    std::cout << std::setw( 5 ) << std::left << " " << "time_slot_s: " << time_slot_s << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "time_slot_n: " << time_slot_n << std::endl;
    std::cout << std::endl;
    
    std::shared_ptr<INeighborRelation> d;
    if ( distance == "euclidean" ) { d = std::make_shared<EuclideanDistance>( dt ); }
    else { d = std::make_shared<LatLonDistance>( dt ); }
    
    // run the algorithm and print results
    std::cout << "Starting ClosedMDCOP..." << std::endl;
    std::map<int, std::set<Pattern>> cmdp = mine_closed_mdcops( dataset.event_types, dataset, { time_slot_s, time_slot_n }, d, spt, tpt );
    std::cout << std::endl;

    std::cout << "Closed Mixed-Drove Spatiotemporal Co-Occurrence Patterns: " << std::endl;
    for ( const auto& pair : cmdp ) {
        const int size = pair.first;
        const std::set<Pattern>& patterns = pair.second;
        std::cout << std::setw( 5 ) << std::left << " " << "size=" << size << ": " << patterns << std::endl;
    }
    
    return EXIT_SUCCESS;
}
