#pragma once

#include <map>
#include <memory>

#include "physics/trajectory.hpp"

namespace principia {
namespace physics {

// This class represent a pair of transformations of a trajectory from
// |FromFrame| to |ToFrame| with an intermediate representation in
// |ThroughFrame|.  Note that the trajectory in |ToFrame| is not the trajectory
// of a body since its past changes from moment to moment.
template<typename FromFrame, typename ThroughFrame, typename ToFrame>
class Transforms {
  static_assert(FromFrame::is_inertial && ToFrame::is_inertial,
                "Both FromFrame and ToFrame must be inertial");

 public:
  // A factory method where |ThroughFrame| is defined as follows: it has the
  // same axes as |FromFrame| and the body of |centre_trajectory| is the origin
  // of |ThroughFrame|.
  static std::unique_ptr<Transforms> BodyCentredNonRotating(
      Trajectory<FromFrame> const& from_centre_trajectory,
      Trajectory<ToFrame> const& to_centre_trajectory);

  // A factory method where |ThroughFrame| is defined as follows: its X axis
  // goes from the primary to the secondary bodies, its Y axis is in the plane
  // of the velocities of the bodies in their barycentric frame, on the same
  // side of the X axis as the velocity of the primary body, its Z axis is such
  // that it is right-handed.  The barycentre of the bodies is the origin of
  // |ThroughFrame|.
  static std::unique_ptr<Transforms> BarycentricRotating(
      Trajectory<FromFrame> const& from_primary_trajectory,
      Trajectory<ToFrame> const& to_primary_trajectory,
      Trajectory<FromFrame> const& from_secondary_trajectory,
      Trajectory<ToFrame> const& to_secondary_trajectory);

  typename Trajectory<FromFrame>::template TransformingIterator<ThroughFrame>
  first(Trajectory<FromFrame> const* from_trajectory);

  typename Trajectory<ThroughFrame>:: template TransformingIterator<ToFrame>
  second(Trajectory<ThroughFrame> const* through_trajectory);

 private:
  typename Trajectory<FromFrame>::template Transform<ThroughFrame> first_;
  typename Trajectory<ThroughFrame>::template Transform<ToFrame> second_;

  // A cache for the result of the |first_| transform.  The map is keyed by
  // time, and therefore assumes that the transform is not called twice with the
  // same time and different degrees of freedom.
  // NOTE(phl): This assumes that |first()| is only called once.
  std::map<Instant const, DegreesOfFreedom<ThroughFrame>> first_cache_;
};

}  // namespace physics
}  // namespace principia

#include "physics/transforms_body.hpp"
