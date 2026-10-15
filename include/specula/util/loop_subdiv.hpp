#ifndef SRC_UTIL_LOOP_SUBDIV_HPP
#define SRC_UTIL_LOOP_SUBDIV_HPP

#include "specula/util/mesh.hpp"
#include "specula/util/transform.hpp"

namespace specula {
  TriangleMesh *loop_subdivide(const Transform *render_from_object, bool reverse_orientation,
                               int n_levels, pstd::span<const int> vertex_indices,
                               pstd::span<const Point3f> p, Allocator alloc);
}

#endif // SRC_UTIL_LOOP_SUBDIV_HPP
