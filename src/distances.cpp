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
    // see http://stackoverflow.com/questions/27928/how-do-i-calculate-distance-between-two-latitude-longitude-points
    const float lat1 = object1->x;
    const float lon1 = object1->y;
    const float lat2 = object2->x;
    const float lon2 = object2->y;
    
    static const float R = 6371;
    float d_lat = deg_to_rad( lat2-lat1 );
    float d_lon = deg_to_rad( lon2-lon1 );
    
    float a = sinf( d_lat/2.f ) * sinf( d_lat/2.f ) + cosf( deg_to_rad( lat1 ) ) * cosf( deg_to_rad( lat2 ) )
        * sinf( d_lon/2.f ) * sinf( d_lon/2.f );
    float c = 2*atan2f( sqrtf( a ), sqrtf( 1-a ) );
    
    float d = R*c;
    return d <= distance_threshold;
}
