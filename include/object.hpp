#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <memory>
#include <ostream>
#include <string>


using EventType = std::string;
using ObjectId = unsigned;
using TimeSlot = unsigned;


struct Object {
    const EventType event_type;
    const ObjectId id;
    const float x, y;
    const TimeSlot time_slot;
    
    Object(EventType, unsigned, float, float, unsigned);
};

std::ostream& operator<<(std::ostream&, const Object&);


#endif  // OBJECT_HPP
