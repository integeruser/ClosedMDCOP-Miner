#include <memory>
#include <ostream>

#include "prettyprint.hpp"

#include "object.hpp"


Object::Object(EventType event_type, ObjectId id, float x, float y, TimeSlot time_slot) :
    event_type( event_type ), id( id ), x( x ), y( y ), time_slot( time_slot ) {
}

std::ostream& operator<< (std::ostream& os, const Object& object) {
    return os << "<" << object.event_type << object.id << ">";
}
