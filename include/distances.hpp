#ifndef DISTANCES_HPP
#define DISTANCES_HPP

#include <memory>

#include "object.hpp"


struct INeighborRelation {
    virtual ~INeighborRelation() {}

    virtual bool neighbors(const std::shared_ptr<Object>& object1, const std::shared_ptr<Object>& object2) = 0;
};



struct EuclideanDistance : public INeighborRelation {
    const float squared_distance_threshold;
    
    EuclideanDistance(float distance_threshold);
    
    virtual bool neighbors(const std::shared_ptr<Object>& object1, const std::shared_ptr<Object>& object2);
};



struct LatLonDistance : public INeighborRelation {
    const float distance_threshold;
    
    LatLonDistance(float distance_threshold);
    
    virtual bool neighbors(const std::shared_ptr<Object>& object1, const std::shared_ptr<Object>& object2);
};


#endif  // DISTANCES_HPP
