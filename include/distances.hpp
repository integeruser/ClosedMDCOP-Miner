#ifndef DISTANCES_HPP
#define DISTANCES_HPP

#include <memory>

#include "object.hpp"


struct INeighborRelation {
    virtual ~INeighborRelation() {}

    virtual bool neighbors(const std::shared_ptr<Object>&, const std::shared_ptr<Object>&) = 0;
};



struct EuclideanDistance : public INeighborRelation {
    const float squared_dt;
    
    EuclideanDistance(float);
    
    virtual bool neighbors(const std::shared_ptr<Object>&, const std::shared_ptr<Object>&);
};



struct LatLonDistance : public INeighborRelation {
    const float dt;
    
    LatLonDistance(float);
    
    virtual bool neighbors(const std::shared_ptr<Object>&, const std::shared_ptr<Object>&);
};


#endif  // DISTANCES_HPP
