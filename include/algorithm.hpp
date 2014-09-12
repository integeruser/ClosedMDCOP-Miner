#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include <map>
#include <memory>
#include <set>
#include <utility>

#include "dataset.hpp"
#include "distances.hpp"
#include "object.hpp"


using Pattern = std::set<EventType>;
using SubPatterns = std::pair<Pattern, Pattern>;
using TimeSlot = unsigned;
using RowInstance = std::pair<std::set<std::shared_ptr<Object>>, std::shared_ptr<Object>>;
using Table = std::set<RowInstance>;


std::map<size_t, std::set<Pattern>> mine_closed_mdcops(const std::set<EventType>&, const Dataset&, const std::pair<unsigned, unsigned>,
                                                       const std::shared_ptr<INeighborRelation>, const float, const float);


#endif  // ALGORITHM_HPP
