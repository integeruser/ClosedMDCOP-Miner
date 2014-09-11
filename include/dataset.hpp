#ifndef DATASET_HPP
#define DATASET_HPP

#include <fstream>
#include <map>
#include <memory>
#include <set>

#include "object.hpp"


struct Dataset {
    std::set<EventType> event_types;
    std::set<std::shared_ptr<Object>> objects;
    std::map<EventType, std::set<std::shared_ptr<Object>>> objects_by_event_type;
    std::map<int, std::set<std::shared_ptr<Object>>> objects_by_time_slot;
};


Dataset construct_dataset(std::ifstream&);

void print_dataset_info(const Dataset&);


#endif  // DATASET_HPP
