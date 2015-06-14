#include <cmath>
#include <memory>

#include "distances.hpp"
#include "object.hpp"


EuclideanDistance::EuclideanDistance(float dt) :
    squared_dt( dt*dt ) {
}

bool EuclideanDistance::neighbors(const std::shared_ptr<Object>& object1, const std::shared_ptr<Object>& object2) {
    float dx = object1->x-object2->x;
    float dy = object1->y-object2->y;
    return (dx*dx + dy*dy) <= squared_dt;
}



LatLonDistance::LatLonDistance(float dt) :
    dt( dt ) {
}

inline float deg_to_rad(float deg) {
    return deg * 3.14159265358979323846/180;
}

bool LatLonDistance::neighbors(const std::shared_ptr<Object>& object1, const std::shared_ptr<Object>& object2) {
    // see http://www.movable-type.co.uk/scripts/latlong.html
    const float lat1 = object1->x, lat2 = object2->x;
    const float lon1 = object1->y, lon2 = object2->y;
    
    static const float R = 6371;  // km
    const float phi1 = deg_to_rad( lat1 );
    const float phi2 = deg_to_rad( lat2 );
    const float dphi = deg_to_rad( lat2-lat1 );
    const float dlambda = deg_to_rad( lon2-lon1 );
    
    const float a = sinf( dphi/2 ) * sinf( dphi/2 ) + cosf( phi1 ) * cosf( phi2 ) * sinf( dlambda/2 ) * sinf( dlambda/2 );
    const float c = 2 * atan2f( sqrtf( a ), sqrtf( 1-a ) );
    
    const float d = R * c;
    return d <= dt;
}
