#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <memory>
#include <ostream>
#include <string>


using EventType = std::string;


struct Object {
    const EventType event_type;
    const unsigned id;
    const float x, y;
    const unsigned time_slot;
    
    Object(EventType, unsigned, float, float, unsigned);
};

std::ostream& operator<< (std::ostream&, const Object&);
std::ostream& operator<< (std::ostream&, const std::shared_ptr<Object>&);


#endif  // OBJECT_HPP
