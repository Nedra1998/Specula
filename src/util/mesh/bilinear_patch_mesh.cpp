#include "specula/util/mesh/bilinear_patch_mesh.hpp"

#include "specula/util/buffer_cache.hpp"
#include "specula/util/stats.hpp"

namespace specula {
  STAT_RATIO("Geometry/Bilinear patches per mesh", N_BLPS, N_BILINEAR_MESHES);
  STAT_MEMORY_COUNTER("Memory/Bilinear patches", BLP_BYTES);
} // namespace specula

specula::BilinearPatchMesh::BilinearPatchMesh(
    const Transform &render_from_object, bool reverse_orientation, std::vector<int> vertex_indices,
    std::vector<Point3f> p, std::vector<Normal3f> n, std::vector<Point2f> uv,
    std::vector<int> face_indices, PiecewiseConstant2D *image_dist, Allocator alloc)
    : reverse_orientation(reverse_orientation),
      transform_swaps_handedness(render_from_object.swap_handedness()),
      n_patches(vertex_indices.size() / 4), n_vertices(p.size()), image_distribution(image_dist) {
  ASSERT_EQ((vertex_indices.size() % 4), 0);
  ++N_BILINEAR_MESHES;
  N_BLPS += n_patches;

  ASSERT_LE(p.size(), std::numeric_limits<int>::max());
  ASSERT_LE(vertex_indices.size(), std::numeric_limits<int>::max());

  this->vertex_indices = INT_BUFFER_CACHE->lookup_or_add(vertex_indices, alloc);
  BLP_BYTES += sizeof(*this);

  for (Point3f &pt : p) {
    pt = render_from_object(pt);
  }
  this->p = POINT3_BUFFER_CACHE->lookup_or_add(p, alloc);

  if (!uv.empty()) {
    ASSERT_EQ(n_vertices, uv.size());
    this->uv = POINT2_BUFFER_CACHE->lookup_or_add(uv, alloc);
  }

  if (!n.empty()) {
    ASSERT_EQ(n_vertices, n.size());
    for (Normal3f &nn : n) {
      nn = render_from_object(nn);
      if (reverse_orientation) {
        nn = -nn;
      }
    }
    this->n = NORMAL3_BUFFER_CACHE->lookup_or_add(n, alloc);
  }

  if (!face_indices.empty()) {
    ASSERT_EQ(n_patches, face_indices.size());
    this->face_indices = INT_BUFFER_CACHE->lookup_or_add(face_indices, alloc);
  }
}
