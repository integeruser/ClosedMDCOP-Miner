#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "prettyprint.hpp"

#include "dataset.hpp"
#include "object.hpp"


Dataset construct_dataset(std::ifstream& dataset_file) {
    assert( dataset_file );
    
    Dataset dataset;
    
    // read input file line per line
    std::string line;
    while ( std::getline( dataset_file, line ) ) {
        std::istringstream iss( line );
        
        EventType event_type;
        float x, y;
        TimeSlot time_slot;
        // check if the line is well-formed, otherwise skip it
        if ( (iss >> event_type >> x >> y >> time_slot) ) {
            // generate object id: "A0", "B0", "A1", ...
            ObjectId id = (ObjectId) dataset.objects_by_event_type[event_type].size();
            
            // add object to dataset
            std::shared_ptr<Object> object = std::make_shared<Object>( event_type, id, x, y, time_slot );
            
            dataset.event_types.insert( event_type );
            dataset.objects.insert( object );
            dataset.objects_by_event_type[object->event_type].insert( object );
            dataset.objects_by_time_slot[object->time_slot].insert( object );
        }
    }
    
    return dataset;
}

void print_dataset_info(const Dataset& dataset) {
    std::cout << "Dataset info: " << std::endl;
    
    // print object count
    int object_count = 0;
    for ( const auto& pair: dataset.objects_by_event_type ) { object_count += pair.second.size(); }
    std::cout << std::setw( 5 ) << std::left << " " << "object count: " << object_count << std::endl;
    
    // print object types
    std::cout << std::setw( 5 ) << std::left << " " << "event types: " << dataset.event_types << std::endl;

    // print object count by event type
    std::map<EventType, unsigned long> object_count_by_type;
    for ( const auto& pair : dataset.objects_by_event_type ) { object_count_by_type[pair.first] = pair.second.size(); }
    std::cout << std::setw( 5 ) << std::left << " " << "object count by event type: " << object_count_by_type << std::endl;
    
    // print time slot count
    std::cout << std::setw( 5 ) << std::left << " " << "time slot count: " << dataset.objects_by_time_slot.size() << std::endl;
    
    // print object count by time slot
    std::map<int, unsigned long> object_count_by_time_slot;
    for ( const auto& pair : dataset.objects_by_time_slot ) { object_count_by_time_slot[pair.first] = pair.second.size(); }
    std::cout << std::setw( 5 ) << std::left << " " << "object count by time slot: " << object_count_by_time_slot << std::endl;
}
