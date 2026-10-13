#ifndef SPECULA_UTIL_MESH_TRIANGLE_MESH_HPP
#define SPECULA_UTIL_MESH_TRIANGLE_MESH_HPP

#include "specula/util/transform.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  class TriangleMesh {
  public:
    TriangleMesh(const Transform &render_from_object, bool reverse_orientation,
                 std::vector<int> vertex_indices, std::vector<Point3f> p, std::vector<Vector3f> s,
                 std::vector<Normal3f> n, std::vector<Point2f> uv, std::vector<int> face_indices,
                 Allocator alloc);

    [[nodiscard]] bool write_ply(const std::string &filename) const;

    bool reverse_orientation, transform_swaps_handedness;
    int n_triangles, n_vertices;
    const int *vertex_indices = nullptr;
    const Point3f *p = nullptr;
    const Normal3f *n = nullptr;
    const Vector3f *s = nullptr;
    const Point2f *uv = nullptr;
    const int *face_indices = nullptr;
  };

  bool write_ply(const std::string &filename, pstd::span<const int> tri_indicies,
                 pstd::span<const int> quad_indicies, pstd::span<const Point3f> p,
                 pstd::span<const Normal3f> n, pstd::span<const Point2f> uv,
                 pstd::span<const int> face_indicies);
} // namespace specula

template <> struct fmt::formatter<specula::TriangleMesh> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::TriangleMesh &v, FormatContext &ctx) const {
    return format_to(
        ctx.out(),
        "[ TriangleMesh reverseOrientation={} transformSwapsHandedness={} nTriangles={} "
        "nVertices={} vertexIndices={} p={} n={} s={} uv={} faceIndices={} ]",
        v.reverse_orientation, v.transform_swaps_handedness, v.n_triangles, v.n_vertices,
        v.vertex_indices
            ? fmt::format("{}", specula::pstd::make_span(v.vertex_indices, 3 * v.n_triangles))
            : "(nullptr)",
        v.p ? fmt::format("{}", specula::pstd::make_span(v.p, v.n_vertices)) : "(nullptr)",
        v.n ? fmt::format("{}", specula::pstd::make_span(v.n, v.n_vertices)) : "(nullptr)",
        v.s ? fmt::format("{}", specula::pstd::make_span(v.s, v.n_vertices)) : "(nullptr)",
        v.uv ? fmt::format("{}", specula::pstd::make_span(v.uv, v.n_vertices)) : "(nullptr)",
        v.face_indices ? fmt::format("{}", specula::pstd::make_span(v.face_indices, v.n_triangles))
                       : "(nullptr)");
  }
};

#endif // SPECULA_UTIL_MESH_TRIANGLE_MESH_HPP
