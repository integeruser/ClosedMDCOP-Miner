#include <cmath>
#include <memory>

#include "distances.hpp"
#include "object.hpp"


EuclideanDistance::EuclideanDistance(float distance_threshold) :
    squared_distance_threshold( distance_threshold*distance_threshold ) {
}

bool EuclideanDistance::neighbors(const std::shared_ptr<Object>& object1, const std::shared_ptr<Object>& object2) {
    float dx = object1->x-object2->x;
    float dy = object1->y-object2->y;
    return (dx*dx + dy*dy) <= squared_distance_threshold;
}



LatLonDistance::LatLonDistance(float distance_threshold) :
    distance_threshold( distance_threshold ) {
}

inline float deg_to_rad(float deg) {
    return deg * 3.14159265358979323846/180;
}

bool LatLonDistance::neighbors(const std::shared_ptr<Object>& object1, const std::shared_ptr<Object>& object2) {
    // see http://www.movable-type.co.uk/scripts/latlong.html
    const float lat1 = object1->x;
    const float lon1 = object1->y;
    const float lat2 = object2->x;
    const float lon2 = object2->y;
    
    static const float R = 6371;  // km
    const float φ1 = deg_to_rad( lat1 );
    const float φ2 = deg_to_rad( lat2 );
    const float Δφ = deg_to_rad( lat2-lat1 );
    const float Δλ = deg_to_rad( lon2-lon1 );
    
    const float a = sinf( Δφ/2 ) * sinf( Δφ/2 ) + cosf( φ1 ) * cosf( φ2 ) * sinf( Δλ/2 ) * sinf( Δλ/2 );
    const float c = 2 * atan2f( sqrtf( a ), sqrtf( 1-a ) );
    
    const float d = R * c;
    return d <= distance_threshold;
}
