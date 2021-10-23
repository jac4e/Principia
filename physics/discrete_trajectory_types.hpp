#pragma once

#include <list>

#include "absl/container/btree_set.h"
#include "base/macros.hpp"
#include "geometry/named_quantities.hpp"
#include "physics/degrees_of_freedom.hpp"
#include "quantities/quantities.hpp"

// An internal header to avoid replicating data structures in multiple places.
// Doesn't export anything outside of its internal namespace.
namespace principia {
namespace physics {

FORWARD_DECLARE_FROM(discrete_trajectory_segment,
                     TEMPLATE(typename Frame) class,
                     DiscreteTrajectorySegment);

namespace internal_discrete_trajectory_types {

using geometry::Instant;
using physics::DegreesOfFreedom;
using quantities::Length;

// |max_dense_intervals| is the maximal number of dense intervals before
// downsampling occurs.  |tolerance| is the tolerance for downsampling with
// |FitHermiteSpline|.
struct DownsamplingParameters {
  std::int64_t max_dense_intervals;
  Length tolerance;
};

template<typename Frame>
struct value_type {
  value_type(Instant const& time,
             DegreesOfFreedom<Frame> const& degrees_of_freedom)
      : time(time), degrees_of_freedom(degrees_of_freedom) {}
  Instant time;
  DegreesOfFreedom<Frame> degrees_of_freedom;
};

struct Precedes {
  Precedes() = default;
  using is_transparent = std::true_type;
  template<typename Frame>
  bool operator()(value_type<Frame> const& left,
                  value_type<Frame> const& right) const {
    return left.time < right.time;
  }
  template<typename Frame>
  bool operator()(Instant const& left, value_type<Frame> const& right) const {
    return left < right.time;
  }
  template<typename Frame>
  bool operator()(value_type<Frame> const& left, Instant const& right) const {
    return left.time < right;
  }
};

template<typename Frame>
using Segments = std::list<DiscreteTrajectorySegment<Frame>>;

template<typename Frame>
using Timeline = absl::btree_set<value_type<Frame>, Precedes>;

}  // namespace internal_discrete_trajectory_types
}  // namespace physics
}  // namespace principia
