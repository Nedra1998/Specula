#include "specula/util/mesh/triangle_mesh.hpp"

#include <limits>
#include <rply.c>

#include <rply.h>

#include "specula/util/buffer_cache.hpp"
#include "specula/util/check.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  STAT_RATIO("Geometry/Triangles per mesh", N_TRIS, N_TRI_MESHES);
  STAT_MEMORY_COUNTER("Memory/Triangles", TRIANGLE_BYTES);
} // namespace specula

static void ply_error_callback(p_ply /*unused*/, const char *message) {
  LOG_ERROR("PLY writing error: {}", message);
}

specula::TriangleMesh::TriangleMesh(const Transform &render_from_object, bool reverse_orientation,
                                    std::vector<int> vertex_indices, std::vector<Point3f> p,
                                    std::vector<Vector3f> s, std::vector<Normal3f> n,
                                    std::vector<Point2f> uv, std::vector<int> face_indices,
                                    Allocator alloc)
    : n_triangles(vertex_indices.size() / 3), n_vertices(p.size()) {
  ASSERT_EQ((vertex_indices.size() % 3), 0);
  ++N_TRI_MESHES;
  N_TRIS += n_triangles;
  TRIANGLE_BYTES += sizeof(*this);
  this->vertex_indices = INT_BUFFER_CACHE->lookup_or_add(vertex_indices, alloc);

  for (Point3f &pt : p) {
    pt = render_from_object(pt);
  }
  this->p = POINT3_BUFFER_CACHE->lookup_or_add(p, alloc);

  this->reverse_orientation = reverse_orientation;
  this->transform_swaps_handedness = render_from_object.swap_handedness();

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

  if (!s.empty()) {
    ASSERT_EQ(n_vertices, s.size());
    for (Vector3f &ss : s) {
      ss = render_from_object(ss);
    }
    this->s = VECTOR3_BUFFER_CACHE->lookup_or_add(s, alloc);
  }

  if (!face_indices.empty()) {
    ASSERT_EQ(n_triangles, face_indices.size());
    this->face_indices = INT_BUFFER_CACHE->lookup_or_add(face_indices, alloc);
  }

  ASSERT_LE(p.size(), std::numeric_limits<int>::max());
  ASSERT_LE(vertex_indices.size(), std::numeric_limits<int>::max());
}

bool specula::TriangleMesh::write_ply(const std::string &filename) const {
  if (s != nullptr) {
    LOG_WARN("{}: PLY mesh will be missing tanget vectors \"S\"", filename);
  }

  return specula::write_ply(
      filename, pstd::span<const int>(vertex_indices, 3 * n_triangles), pstd::span<const int>(),
      pstd::span<const Point3f>(p, n_vertices),
      pstd::span<const Normal3f>(n, (n != nullptr) ? n_vertices : 0),
      pstd::span<const Point2f>(uv, (uv != nullptr) ? n_vertices : 0),
      pstd::span<const int>(face_indices, (face_indices != nullptr) ? n_triangles : 0));
}

bool specula::write_ply(const std::string &filename, pstd::span<const int> tri_indicies,
                        pstd::span<const int> quad_indicies, pstd::span<const Point3f> p,
                        pstd::span<const Normal3f> n, pstd::span<const Point2f> uv,
                        pstd::span<const int> face_indicies) {
  p_ply ply_file = ply_create(filename.c_str(), PLY_DEFAULT, ply_error_callback, 0, nullptr);
  if (ply_file == nullptr) {
    return false;
  }

  int n_verticies = p.size();
  ply_add_element(ply_file, "vertex", n_verticies);
  ply_add_scalar_property(ply_file, "x", PLY_FLOAT);
  ply_add_scalar_property(ply_file, "y", PLY_FLOAT);
  ply_add_scalar_property(ply_file, "z", PLY_FLOAT);
  if (!n.empty()) {
    ply_add_scalar_property(ply_file, "nx", PLY_FLOAT);
    ply_add_scalar_property(ply_file, "ny", PLY_FLOAT);
    ply_add_scalar_property(ply_file, "nz", PLY_FLOAT);
  }
  if (!uv.empty()) {
    ply_add_scalar_property(ply_file, "u", PLY_FLOAT);
    ply_add_scalar_property(ply_file, "v", PLY_FLOAT);
  }

  int n_triangles = tri_indicies.size() / 3, n_quads = quad_indicies.size() / 4;
  ply_add_element(ply_file, "face", n_triangles + n_quads);
  ply_add_list_property(ply_file, "vertex_indicies", PLY_UINT8, PLY_INT);
  if (!face_indicies.empty()) {
    ply_add_scalar_property(ply_file, "face_indicies", PLY_INT);
  }

  ply_write_header(ply_file);

  for (int i = 0; i < n_verticies; ++i) {
    ply_write(ply_file, p[i].x);
    ply_write(ply_file, p[i].y);
    ply_write(ply_file, p[i].z);
    if (!n.empty()) {
      ply_write(ply_file, n[i].x);
      ply_write(ply_file, n[i].y);
      ply_write(ply_file, n[i].z);
    }
    if (!uv.empty()) {
      ply_write(ply_file, uv[i].x);
      ply_write(ply_file, uv[i].y);
    }
  }

  for (int i = 0; i < n_triangles; ++i) {
    ply_write(ply_file, 3);
    ply_write(ply_file, tri_indicies[3 * i]);
    ply_write(ply_file, tri_indicies[3 * i + 1]);
    ply_write(ply_file, tri_indicies[3 * i + 2]);
    if (!face_indicies.empty()) {
      ply_write(ply_file, face_indicies[i]);
    }
  }

  for (int i = 0; i < n_quads; ++i) {
    ply_write(ply_file, 4);
    ply_write(ply_file, quad_indicies[4 * i]);
    ply_write(ply_file, quad_indicies[4 * i + 1]);
    ply_write(ply_file, quad_indicies[4 * i + 2]);
    ply_write(ply_file, quad_indicies[4 * i + 3]);
    if (!face_indicies.empty()) {
      ply_write(ply_file, face_indicies[i]);
    }
  }

  ply_close(ply_file);
  return true;
}
