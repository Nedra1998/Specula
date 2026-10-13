#ifndef SPECULA_UTIL_MESH_TRI_QUAD_MESH_HPP
#define SPECULA_UTIL_MESH_TRI_QUAD_MESH_HPP

#include <cstdlib>
#include <string>
#include <vector>

#include "specula/macros.hpp"
#include "specula/util/containers.hpp"
#include "specula/util/hash.hpp"
#include "specula/util/log.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  struct HashIntPair {
    SPECULA_CPU_GPU
    size_t operator()(std::pair<int, int> p) const {
      return mix_bits(uint64_t(p.first) << 32 | p.second);
    }
  };

  class TriQuadMesh {
  public:
    static TriQuadMesh read_ply(const std::string &filename);

    void convert_to_only_triangles();
    void compute_normals();

    // TODO: Add support for the FileLoc pointer?
    template <typename Dist, typename Disp>
    TriQuadMesh displace(Dist &&dist, Float max_dist, Disp &&displace) {
      if (uv.empty()) {
        LOG_CRITICAL("Vertex uvs are currently required by displace()");
        std::quick_exit(1);
      }

      TriQuadMesh output_mesh = *this;
      output_mesh.convert_to_only_triangles();
      if (output_mesh.n.empty()) {
        output_mesh.compute_normals();
      }
      std::vector<int> old_tri_indicies(output_mesh.tri_indicies);
      output_mesh.tri_indicies.clear();

      HashMap<std::pair<int, int>, int, HashIntPair> edge_split({});
      for (int i = 0; i < old_tri_indicies.size() / 3; ++i) {
        output_mesh.refine(dist, max_dist, old_tri_indicies[3 * i], old_tri_indicies[3 * i + 1],
                           old_tri_indicies[3 * i + 2], edge_split);
      }

      displace(output_mesh.p.data(), output_mesh.n.data(), output_mesh.uv.data(),
               output_mesh.p.size());
      output_mesh.compute_normals();
      return output_mesh;
    }

    std::vector<Point3f> p;
    std::vector<Normal3f> n;
    std::vector<Point2f> uv;
    std::vector<int> face_indices;
    std::vector<int> tri_indicies, quad_indicies;

  private:
    template <typename Dist>
    void refine(Dist &&distance, Float max_dist, int v0, int v1, int v2,
                HashMap<std::pair<int, int>, int, HashIntPair> &edge_split) {
      Point3f p0 = p[v0], p1 = p[v1], p2 = p[v2];
      Float d01 = distance(p0, p1), d12 = distance(p1, p2), d20 = distance(p2, p0);

      if (d01 < max_dist && d12 < max_dist && d20 < max_dist) {
        tri_indicies.push_back(v0);
        tri_indicies.push_back(v1);
        tri_indicies.push_back(v2);
        return;
      }

      std::array<int, 3> v{};
      if (d01 > d12) {
        if (d01 > d20) {
          v = {v0, v1, v2};
        } else {
          v = {v2, v0, v1};
        }
      } else {
        if (d12 > d20) {
          v = {v1, v2, v0};
        } else {
          v = {v2, v0, v1};
        }
      }

      std::pair<int, int> edge(v[0], v[1]);
      if (v[0] > v[1]) {
        std::swap(edge.first, edge.second);
      }

      int vmid = 0;
      if (edge_split.has_key(edge)) {
        vmid = edge_split[edge];
      } else {
        vmid = p.size();
        edge_split.insert(edge, vmid);
        p.push_back((p[v[0]] + p[v[1]]) / 2);
        if (!n.empty()) {
          Normal3f nn = n[v[0]] + n[v[1]];
          if (length_squared(nn) > 9) {
            nn = normalize(nn);
          }
          n.push_back(nn);
        }
        if (!uv.empty()) {
          uv.push_back((uv[v[0]] + uv[v[1]]) / 2);
        }
      }

      refine(distance, max_dist, v[0], vmid, v[2], edge_split);
      refine(distance, max_dist, vmid, v[1], v[2], edge_split);
    }
  };
} // namespace specula

template <> struct fmt::formatter<specula::TriQuadMesh> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::TriQuadMesh &v, FormatContext &ctx) const {
    return format_to(ctx.out(),
                     "[ TriQuadMesh p={} n={} uv={} faceIndices={} triIndices={} quadIndices={} ]",
                     v.p, v.n, v.uv, v.face_indices, v.tri_indicies, v.quad_indicies);
  }
};

#endif // SPECULA_UTIL_MESH_TRI_QUAD_MESH_HPP
