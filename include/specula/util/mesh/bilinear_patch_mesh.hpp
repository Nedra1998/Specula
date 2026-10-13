#ifndef SPECULA_UTIL_MESH_BILINEAR_PATCH_MESH_HPP
#define SPECULA_UTIL_MESH_BILINEAR_PATCH_MESH_HPP

#include "specula/util/sampling.hpp"
#include "specula/util/transform.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  class BilinearPatchMesh {
  public:
    BilinearPatchMesh(const Transform &render_from_object, bool reverse_orientation,
                      std::vector<int> vertex_indices, std::vector<Point3f> p,
                      std::vector<Normal3f> n, std::vector<Point2f> uv,
                      std::vector<int> face_indices, PiecewiseConstant2D *image_dist,
                      Allocator alloc);

    bool reverse_orientation, transform_swaps_handedness;
    int n_patches, n_vertices;
    const int *vertex_indices = nullptr;
    const Point3f *p = nullptr;
    const Normal3f *n = nullptr;
    const Point2f *uv = nullptr;
    const int *face_indices = nullptr;
    PiecewiseConstant2D *image_distribution;
  };
} // namespace specula

template <> struct fmt::formatter<specula::BilinearPatchMesh> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::BilinearPatchMesh &v, FormatContext &ctx) const {
    return format_to(
        ctx.out(),
        "[ BilinearPatchmesh reverseOrientation={} transformSwapsHandedness={} nPatches={} "
        "nVertices={} vertexIndices={} p={} n={} uv={} faceIndices={} ]",
        v.reverse_orientation, v.transform_swaps_handedness, v.n_patches, v.n_vertices,
        v.vertex_indices
            ? fmt::format("{}", specula::pstd::make_span(v.vertex_indices, 4 * v.n_patches))
            : "(nullptr)",
        v.p ? fmt::format("{}", specula::pstd::make_span(v.p, v.n_vertices)) : "(nullptr)",
        v.n ? fmt::format("{}", specula::pstd::make_span(v.n, v.n_vertices)) : "(nullptr)",
        v.uv ? fmt::format("{}", specula::pstd::make_span(v.uv, v.n_vertices)) : "(nullptr)",
        v.face_indices ? fmt::format("{}", specula::pstd::make_span(v.face_indices, v.n_patches))
                       : "(nullptr)");
  }
};

#endif // SPECULA_UTIL_MESH_BILINEAR_PATCH_MESH_HPP
