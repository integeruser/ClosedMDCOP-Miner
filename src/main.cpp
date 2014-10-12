#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "prettyprint.hpp"

#include "algorithm.hpp"
#include "dataset.hpp"
#include "distances.hpp"


bool validate_arguments(std::string dataset_file_path, int first_time_slot, int time_slot_count, std::string distance, float dt, float p, float time) {
    std::ifstream dataset_file ( dataset_file_path );
    if ( !dataset_file ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Failed to open dataset_file: " << dataset_file_path << std::endl;
        return false;
    }
    if ( first_time_slot < 0 ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Invalid first_time_slot: " << first_time_slot << std::endl;
        return false;
    }
    if ( time_slot_count <= 0 ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Invalid time_slot_count: " << time_slot_count << std::endl;
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
    if ( p <= 0 || p > 1 ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Invalid p: " << p << std::endl;
        return false;
    }
    if ( time <= 0 || time > 1 ) {
        std::cerr << std::setw( 5 ) << std::left << " " << "ERROR: Invalid time: " << time << std::endl;
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
        
        std::cerr << "Usage: ClosedMDCOP-Miner dataset_file_path first_time_slot time_slot_count distance dt p time" << std::endl;
        std::cerr << "Parameters:" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "dataset_file_path: the dataset file" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "first_time_slot: the starting time slot" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "time_slot_count: the number of time slots to mine" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "distance: the distance function to use ('euclidean' or 'latlon')" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "dt: the maximum distance for considering two objects as neighbors (0 < dt)" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "p: the spatial prevalence threshold (0 < p <= 1)" << std::endl;
        std::cerr << std::setw( 5 ) << std::left << " " << "time: the time prevalence threshold (0 < time <= 1)" << std::endl;
        std::cerr << "Example: ClosedMDCOP-Miner dataset.txt 0 3 latlon 2 0.3 0.2" << std::endl;
        return EXIT_FAILURE;
    }
    
    std::string dataset_file_path = argv[1];
    int first_time_slot = std::stoi( argv[2] );
    int time_slot_count = std::stoi( argv[3] );
    std::string distance = argv[4];
    float dt = std::stof( argv[5] );
    float p = std::stof( argv[6] );
    float time = std::stof( argv[7] );
    if ( !validate_arguments( dataset_file_path, first_time_slot, time_slot_count, distance, dt, p, time ) ) { return EXIT_FAILURE; }

    std::cout << std::setw( 5 ) << std::left << " " << "dataset_file_path: " << dataset_file_path << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "first_time_slot: " << first_time_slot << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "time_slot_count: " << time_slot_count << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "distance: " << distance << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "dt: " << dt << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "p: " << p << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "time: " << time << std::endl;
    std::cout << std::endl;
    
    // construct dataset
    std::cout << "Constructing dataset from '" << dataset_file_path << "'..." << std::endl;
    std::ifstream dataset_file ( dataset_file_path );
    Dataset dataset = construct_dataset( dataset_file );
    print_dataset_info( dataset );
    std::cout << std::endl;
    
    // re-validate time slots arguments after dataset parsing
    std::cout << "Clamping time slots..." << std::endl;
    int dataset_time_slot_count = (int) dataset.objects_by_time_slot.size();
    first_time_slot = std::min( first_time_slot, dataset_time_slot_count-1 );
    time_slot_count = std::min( time_slot_count, dataset_time_slot_count-first_time_slot );
    std::cout << std::setw( 5 ) << std::left << " " << "first_time_slot: " << first_time_slot << std::endl;
    std::cout << std::setw( 5 ) << std::left << " " << "time_slot_count: " << time_slot_count << std::endl;
    std::cout << std::endl;
    
    std::shared_ptr<INeighborRelation> r;
    if ( distance == "euclidean" ) { r = std::make_shared<EuclideanDistance>( dt ); }
    else { r = std::make_shared<LatLonDistance>( dt ); }
    
    // run the algorithm and print results
    std::cout << "Starting ClosedMDCOP-Miner..." << std::endl;
    std::map<size_t, std::set<Pattern>> cmdp = mine_closed_mdcops( dataset.event_types, dataset,
                                                                   { (TimeSlot) first_time_slot, (unsigned) time_slot_count },
                                                                   r, p, time );
    std::cout << std::endl;

    if ( cmdp.size() == 0 ) {
        std::cout << "No Closed Mixed-Drove Spatiotemporal Co-Occurrence Patterns found." << std::endl;
    }
    else {
        std::cout << "Closed Mixed-Drove Spatiotemporal Co-Occurrence Patterns: " << std::endl;
        for ( const auto& pair : cmdp ) {
            const size_t size = pair.first;
            const std::set<Pattern>& patterns = pair.second;
            std::cout << std::setw( 5 ) << std::left << " " << "size=" << size << " (" << patterns.size() << "): " << patterns << std::endl;
        }
    }
    
    return EXIT_SUCCESS;
}
