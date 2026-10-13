#include "specula/util/mesh/tri_quad_mesh.hpp"

#include <cstdlib>
#include <vector>

#include <rply.h>

#include "specula/util/vecmath.hpp"

struct FaceCallbackContext {
  int face[4];
  std::vector<int> tri_indicies, quad_indicies;
};

static void rply_message_callback(p_ply ply, const char *message) { LOG_WARN("rply: {}", message); }

static int rply_vertex_callback(p_ply_argument argument) {
  specula::Float *buffer = nullptr;
  long index = 0, flags = 0;

  ply_get_argument_user_data(argument, (void **)&buffer, &flags);
  ply_get_argument_element(argument, nullptr, &index);

  int stride = (flags & 0x0F0) >> 4;
  int offset = flags & 0x00F;

  buffer[index * stride + offset] = (float)ply_get_argument_value(argument);
  return 1;
}

static int rply_face_callback(p_ply_argument argument) {
  FaceCallbackContext *ctx = nullptr;
  long flags = 0;
  ply_get_argument_user_data(argument, (void **)&ctx, &flags);

  long length = 0, value_index = 0;
  ply_get_argument_property(argument, nullptr, &length, &value_index);

  if (length != 3 && length != 4) {
    LOG_WARN("plymesh: ignoring face with {} verticies (only triangles and quads are supported)",
             length);
    return 1;
  } else if (value_index < 0) {
    return 1;
  }

  if (value_index >= 0) {
    ctx->face[value_index] = (int)ply_get_argument_value(argument);
  }

  if (value_index == length - 1) {
    if (length == 3) {
      for (int i = 0; i < 3; ++i) {
        ctx->tri_indicies.push_back(ctx->face[i]);
      }
    } else {
      ASSERT_EQ(length, 4);
      ctx->quad_indicies.push_back(ctx->face[0]);
      ctx->quad_indicies.push_back(ctx->face[1]);
      ctx->quad_indicies.push_back(ctx->face[3]);
      ctx->quad_indicies.push_back(ctx->face[2]);
    }
  }

  return 1;
}

static int rply_face_index_callback(p_ply_argument argument) {
  std::vector<int> *face_indices = nullptr;
  long flags = 0;
  ply_get_argument_user_data(argument, (void **)&face_indices, &flags);
  face_indices->push_back((int)ply_get_argument_value(argument));
  return 1;
}

specula::TriQuadMesh specula::TriQuadMesh::read_ply(const std::string &filename) {
  TriQuadMesh mesh;

  p_ply ply = ply_open(filename.c_str(), rply_message_callback, 0, nullptr);
  if (ply == nullptr) {
    LOG_ERROR("Couldn't open PLY file \"{}\"", filename);
    std::quick_exit(1);
  }

  if (ply_read_header(ply) == 0) {
    LOG_ERROR("Unabled to read the header of PLY file \"{}\"", filename);
    std::quick_exit(1);
  }

  p_ply_element element = nullptr;
  size_t vertex_count = 0, face_count = 0;

  while ((element = ply_get_next_element(ply, element)) != nullptr) {
    const char *name = nullptr;
    long n_instances = 0;

    ply_get_element_info(element, &name, &n_instances);
    if (strcmp(name, "vertex") == 0) {
      vertex_count = n_instances;
    }
    if (strcmp(name, "face") == 0) {
      face_count = n_instances;
    }
  }

  if (vertex_count == 0 || face_count == 0) {
    LOG_ERROR("{}: PLY file is invalid, no face / vertex elements found", filename);
    std::quick_exit(1);
  }

  mesh.p.resize(vertex_count);
  if (ply_set_read_cb(ply, "vertex", "x", rply_vertex_callback, mesh.p.data(), 0x30) == 0 ||
      ply_set_read_cb(ply, "vertex", "y", rply_vertex_callback, mesh.p.data(), 0x31) == 0 ||
      ply_set_read_cb(ply, "vertex", "z", rply_vertex_callback, mesh.p.data(), 0x32) == 0) {
    LOG_ERROR("{}: vertex coordinate property not found", filename);
    std::quick_exit(1);
  }

  mesh.n.resize(vertex_count);
  if (ply_set_read_cb(ply, "vertex", "nx", rply_vertex_callback, mesh.n.data(), 0x30) == 0 ||
      ply_set_read_cb(ply, "vertex", "ny", rply_vertex_callback, mesh.n.data(), 0x31) == 0 ||
      ply_set_read_cb(ply, "vertex", "nz", rply_vertex_callback, mesh.n.data(), 0x32) == 0) {
    mesh.n.resize(0);
  }

  mesh.uv.resize(vertex_count);
  if ((ply_set_read_cb(ply, "vertex", "u", rply_vertex_callback, mesh.uv.data(), 0x20) != 0 &&
       ply_set_read_cb(ply, "vertex", "v", rply_vertex_callback, mesh.uv.data(), 0x21) != 0) ||
      (ply_set_read_cb(ply, "vertex", "s", rply_vertex_callback, mesh.uv.data(), 0x20) != 0 &&
       ply_set_read_cb(ply, "vertex", "t", rply_vertex_callback, mesh.uv.data(), 0x21) != 0) ||
      (ply_set_read_cb(ply, "vertex", "texture_u", rply_vertex_callback, mesh.uv.data(), 0x20) !=
           0 &&
       ply_set_read_cb(ply, "vertex", "texture_v", rply_vertex_callback, mesh.uv.data(), 0x21) !=
           0) ||
      (ply_set_read_cb(ply, "vertex", "texture_s", rply_vertex_callback, mesh.uv.data(), 0x20) !=
           0 &&
       ply_set_read_cb(ply, "vertex", "texture_t", rply_vertex_callback, mesh.uv.data(), 0x21) !=
           0)) {
  } else {
    mesh.uv.resize(0);
  }

  FaceCallbackContext ctx;
  ctx.tri_indicies.reserve(face_count * 3);
  ctx.quad_indicies.reserve(face_count * 4);
  if (ply_set_read_cb(ply, "face", "vertex_indices", rply_face_callback, &ctx, 0) == 0) {
    LOG_ERROR("{}: vertex indices not found in PLY file", filename);
    std::quick_exit(1);
  }
  if (ply_set_read_cb(ply, "face", "face_indices", rply_face_callback, &mesh.face_indices, 0) !=
      0) {
    mesh.face_indices.reserve(face_count);
  }

  if (ply_read(ply) == 0) {
    LOG_ERROR("{}: unable to read the contents of PLY file", filename);
    std::quick_exit(1);
  }

  mesh.tri_indicies = std::move(ctx.tri_indicies);
  mesh.quad_indicies = std::move(ctx.quad_indicies);

  ply_close(ply);

  for (int idx : mesh.tri_indicies) {
    if (idx < 0 || idx > mesh.p.size()) {
      LOG_ERROR("plymesh: vertex index {} is out of bounds, valid range is [0..{})", idx,
                mesh.p.size());
      std::quick_exit(1);
    }
  }

  for (int idx : mesh.quad_indicies) {
    if (idx < 0 || idx > mesh.p.size()) {
      LOG_ERROR("plymesh: vertex index {} is out of bounds, valid range is [0..{})", idx,
                mesh.p.size());
      std::quick_exit(1);
    }
  }

  return mesh;
}

void specula::TriQuadMesh::convert_to_only_triangles() {
  if (quad_indicies.empty()) {
    return;
  }

  tri_indicies.reserve(tri_indicies.size() + 3 * quad_indicies.size() / 2);

  for (size_t i = 0; i < quad_indicies.size(); i += 4) {
    tri_indicies.push_back(quad_indicies[i]);
    tri_indicies.push_back(quad_indicies[i + 1]);
    tri_indicies.push_back(quad_indicies[i + 3]);

    tri_indicies.push_back(quad_indicies[i]);
    tri_indicies.push_back(quad_indicies[i + 3]);
    tri_indicies.push_back(quad_indicies[i + 2]);
  }

  quad_indicies.clear();
}

void specula::TriQuadMesh::compute_normals() {
  // TODO: Handle the case with quads
  ASSERT_EQ(0, quad_indicies.size());

  n.resize(p.size());
  for (auto &nn : n) {
    nn = Normal3f(0, 0, 0);
  }

  for (size_t i = 0; i < tri_indicies.size(); i += 3) {
    int v[3] = {tri_indicies[i], tri_indicies[i + 1], tri_indicies[i + 2]};
    Vector3f v10 = p[v[1]] - p[v[0]];
    Vector3f v21 = p[v[2]] - p[v[1]];

    Normal3f vn(cross(v10, v21));
    if (length_squared(vn) > 0) {
      vn = normalize(vn);
      n[v[0]] += vn;
      n[v[1]] += vn;
      n[v[2]] += vn;
    }
  }

  for (auto &nn : n) {
    if (length_squared(nn) > 0) {
      nn = normalize(nn);
    }
  }
}
