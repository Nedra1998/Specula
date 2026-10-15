#include "specula/util/loop_subdiv.hpp"

#include <map>
#include <memory>
#include <set>
#include <vector>

#include <fmt/base.h>

#include "specula/util/containers.hpp"
#include "specula/util/log.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/vecmath.hpp"

#define NEXT(i) (((i) + 1) % 3)
#define PREV(i) (((i) + 2) % 3)

namespace specula {
  struct SdFace;

  struct SdVertex {
    SdVertex(const Point3f &p = Point3f(0, 0, 0)) : p(p) {}

    int valence();
    void one_ring(Point3f *p);

    Point3f p;
    SdFace *start_face = nullptr;
    SdVertex *child = nullptr;
    bool regular = false, boundary = false;
  };

  struct SdFace {
    SdFace() {
      for (int i = 0; i < 3; ++i) {
        v[i] = nullptr;
        f[i] = nullptr;
      }
      for (auto &i : children) {
        i = nullptr;
      }
    }

    int vnum(SdVertex *vert) const {
      for (int i = 0; i < 3; ++i) {
        if (v[i] == vert) {
          return i;
        }
      }
      LOG_CRITICAL("basic logic error in SdFace::venum()");
      return -1;
    }

    SdFace *next_face(SdVertex *vert) { return f[vnum(vert)]; }
    SdFace *prev_face(SdVertex *vert) { return f[PREV(vnum(vert))]; }
    SdVertex *next_vert(SdVertex *vert) { return v[NEXT(vnum(vert))]; }
    SdVertex *prev_vert(SdVertex *vert) { return v[PREV(vnum(vert))]; }
    SdVertex *other_vert(SdVertex *v0, SdVertex *v1) {
      for (auto &vv : v) {
        if (vv != v0 && vv != v1) {
          return vv;
        }
      }
      LOG_CRITICAL("basic logic error in SdFace::other_vert()");
      return nullptr;
    }

    SdVertex *v[3];
    SdFace *f[3];
    SdFace *children[4];
  };

  struct SdEdge {
    SdEdge(SdVertex *v0 = nullptr, SdVertex *v1 = nullptr) {
      v[0] = std::min(v0, v1);
      v[1] = std::max(v0, v1);
      f[0] = f[1] = nullptr;
    }

    bool operator<(const SdEdge &e2) const {
      if (v[0] == e2.v[0]) {
        return v[1] < e2.v[1];
      }
      return v[0] < e2.v[0];
    }

    SdVertex *v[2];
    SdFace *f[2];
    int f0_edge_num = -1;
  };

  inline int SdVertex::valence() {
    SdFace *f = start_face;
    if (!boundary) {
      int nf = 1;
      while ((f = f->next_face(this)) != start_face) {
        ++nf;
      }
      return nf;
    } else {
      int nf = 1;
      while ((f = f->next_face(this)) != nullptr) {
        ++nf;
      }
      f = start_face;
      while ((f = f->prev_face(this)) != nullptr) {
        ++nf;
      }
      return nf + 1;
    }
  }

  inline void SdVertex::one_ring(Point3f *p) {
    if (!boundary) {
      SdFace *face = start_face;
      do {
        *p++ = face->next_vert(this)->p;
        face = face->next_face(this);
      } while (face != start_face);
    } else {
      SdFace *face = start_face, *f2 = nullptr;
      while ((f2 = face->next_face(this)) != nullptr) {
        face = f2;
      }
      *p++ = face->next_vert(this)->p;
      do {
        *p++ = face->prev_vert(this)->p;
        face = face->prev_face(this);
      } while (face != nullptr);
    }
  }

  inline Float beta(int valence) {
    if (valence == 3) {
      return 3.0f / 16.0f;
    } else {
      return 3.0f / (8.0f * valence);
    }
  }

  inline Float loop_gamma(int valence) { return 1.0f / (valence + 3.0f / (8.0f * beta(valence))); }

  inline static Point3f weight_one_ring(SdVertex *vert, Float beta) {
    int valence = vert->valence();
    InlinedVector<Point3f, 16> pring(valence);
    vert->one_ring(pring.data());

    Point3f p = (1 - valence * beta) * vert->p;
    for (int i = 0; i < valence; ++i) {
      p += beta * pring[i];
    }
    return p;
  }

  inline static Point3f weight_boundry(SdVertex *vert, Float beta) {
    int valence = vert->valence();
    InlinedVector<Point3f, 16> pring(valence);
    vert->one_ring(pring.data());

    Point3f p = (1 - 2 * beta) * vert->p;
    p += beta * pring[0];
    p += beta * pring[valence - 1];
    return p;
  }
} // namespace specula

specula::TriangleMesh *specula::loop_subdivide(const Transform *render_from_object,
                                               bool reverse_orientation, int n_levels,
                                               pstd::span<const int> vertex_indices,
                                               pstd::span<const Point3f> p, Allocator alloc) {
  std::vector<SdVertex *> vertices;
  std::vector<SdFace *> faces;

  std::unique_ptr<SdVertex[]> verts = std::make_unique<SdVertex[]>(p.size());
  vertices.reserve(p.size());
  for (int i = 0; i < p.size(); ++i) {
    verts[i] = SdVertex(p[i]);
    vertices.push_back(&verts[i]);
  }

  size_t n_faces = vertex_indices.size() / 3;
  std::unique_ptr<SdFace[]> fs = std::make_unique<SdFace[]>(n_faces);
  faces.reserve(n_faces);
  for (int i = 0; i < n_faces; ++i) {
    faces.push_back(&fs[i]);
  }

  const int *vp = vertex_indices.data();
  for (size_t i = 0; i < n_faces; ++i, vp += 3) {
    SdFace *f = faces[i];
    for (int j = 0; j < 3; ++j) {
      SdVertex *v = vertices[vp[j]];
      f->v[j] = v;
      v->start_face = f;
    }
  }

  std::set<SdEdge> edges;
  for (int i = 0; i < n_faces; ++i) {
    SdFace *f = faces[i];
    for (int edge_num = 0; edge_num < 3; ++edge_num) {
      int v0 = edge_num, v1 = NEXT(edge_num);
      SdEdge e(f->v[v0], f->v[v1]);
      if (!edges.contains(e)) {
        e.f[0] = f;
        e.f0_edge_num = edge_num;
        edges.insert(e);
      } else {
        e = *edges.find(e);
        e.f[0]->f[e.f0_edge_num] = f;
        f->f[edge_num] = e.f[0];
        edges.erase(e);
      }
    }
  }

  for (size_t i = 0; i < p.size(); ++i) {
    SdVertex *v = vertices[i];
    SdFace *f = v->start_face;
    do {
      f = f->next_face(v);
    } while ((f != nullptr) && f != v->start_face);
    v->boundary = (f == nullptr);
    if (!v->boundary && v->valence() == 6) {
      v->regular = true;
    } else if (v->boundary && v->valence() == 4) {
      v->regular = true;
    } else {
      v->regular = false;
    }
  }

  std::vector<SdFace *> f = faces;
  std::vector<SdVertex *> v = vertices;
  pstd::pmr::monotonic_buffer_resource resource;
  Allocator vertex_face_allocator(&resource);

  for (int i = 0; i < n_levels; ++i) {
    std::vector<SdFace *> new_faces;
    std::vector<SdVertex *> new_vertices;

    new_vertices.reserve(v.size());
    for (SdVertex *vertex : v) {
      vertex->child = vertex_face_allocator.new_object<SdVertex>();
      vertex->child->regular = vertex->regular;
      vertex->child->boundary = vertex->boundary;
      new_vertices.push_back(vertex->child);
    }

    new_faces.reserve(f.size() * 4);
    for (SdFace *face : f) {
      for (auto &child : face->children) {
        child = vertex_face_allocator.new_object<SdFace>();
        new_faces.push_back(child);
      }
    }

    for (SdVertex *vertex : v) {
      if (!vertex->boundary) {
        if (vertex->regular) {
          vertex->child->p = weight_one_ring(vertex, 1.0f / 16.0f);
        } else {
          vertex->child->p = weight_one_ring(vertex, beta(vertex->valence()));
        }
      } else {
        vertex->child->p = weight_boundry(vertex, 1.0f / 8.0f);
      }
    }

    std::map<SdEdge, SdVertex *> edge_verts;
    for (SdFace *face : f) {
      for (int k = 0; k < 3; ++k) {
        SdEdge edge(face->v[k], face->v[NEXT(k)]);
        SdVertex *vert = edge_verts[edge];
        if (vert == nullptr) {
          vert = vertex_face_allocator.new_object<SdVertex>();
          new_vertices.push_back(vert);
          vert->regular = true;
          vert->boundary = (face->f[k] == nullptr);
          vert->start_face = face->children[3];

          if (vert->boundary) {
            vert->p = 0.5f * edge.v[0]->p;
            vert->p += 0.5 * edge.v[1]->p;
          } else {
            vert->p = 3.0f / 8.0f * edge.v[0]->p;
            vert->p += 3.0f / 8.0f * edge.v[1]->p;
            vert->p += 1.0f / 8.0f * face->other_vert(edge.v[0], edge.v[1])->p;
            vert->p += 1.0f / 8.0f * face->f[k]->other_vert(edge.v[0], edge.v[1])->p;
          }
          edge_verts[edge] = vert;
        }
      }
    }

    for (SdVertex *vertex : v) {
      int vert_num = vertex->start_face->vnum(vertex);
      vertex->child->start_face = vertex->start_face->children[vert_num];
    }

    for (SdFace *face : f) {
      for (int j = 0; j < 3; ++j) {
        face->children[3]->f[j] = face->children[NEXT(j)];
        face->children[j]->f[NEXT(j)] = face->children[3];

        SdFace *f2 = face->f[j];
        face->children[j]->f[j] = f2 != nullptr ? f2->children[f2->vnum(face->v[j])] : nullptr;
        f2 = face->f[PREV(j)];
        face->children[j]->f[PREV(j)] =
            f2 != nullptr ? f2->children[f2->vnum(face->v[j])] : nullptr;
      }
    }

    for (SdFace *face : f) {
      for (int j = 0; j < 3; ++j) {
        face->children[j]->v[j] = face->v[j]->child;

        SdVertex *vert = edge_verts[SdEdge(face->v[j], face->v[NEXT(j)])];
        face->children[j]->v[NEXT(j)] = vert;
        face->children[NEXT(j)]->v[j] = vert;
        face->children[3]->v[j] = vert;
      }
    }

    f = new_faces;
    v = new_vertices;
  }

  std::vector<Point3f> p_limit(v.size());
  for (size_t i = 0; i < v.size(); ++i) {
    if (v[i]->boundary) {
      p_limit[i] = weight_boundry(v[i], 1.0f / 5.0f);
    } else {
      p_limit[i] = weight_one_ring(v[i], loop_gamma(v[i]->valence()));
    }
  }
  for (size_t i = 0; i < v.size(); ++i) {
    v[i]->p = p_limit[i];
  }

  std::vector<Normal3f> ns;
  ns.reserve(v.size());
  std::vector<Point3f> p_ring(16, Point3f());
  for (SdVertex *vertex : v) {
    Vector3f s(0, 0, 0), t(0, 0, 0);
    int valence = vertex->valence();
    if (valence > (int)p_ring.size()) {
      p_ring.resize(valence);
    }
    vertex->one_ring(&p_ring[0]);
    if (!vertex->boundary) {
      for (int j = 0; j < valence; ++j) {
        s += std::cos(2 * PI * j / valence) * Vector3f(p_ring[j]);
        t += std::sin(2 * PI * j / valence) * Vector3f(p_ring[j]);
      }
    } else {
      s = p_ring[valence - 1] - p_ring[0];
      if (valence == 2) {
        t = Vector3f(p_ring[0] + p_ring[1] - 2 * vertex->p);
      } else if (valence == 3) {
        t = p_ring[1] - vertex->p;
      } else if (valence == 4) {
        t = Vector3f(-1 * p_ring[0] + 2 * p_ring[1] + 2 * p_ring[2] + -1 * p_ring[3] +
                     -2 * vertex->p);
      } else {
        Float theta = PI / float(valence - 1);
        t = Vector3f(std::sin(theta) * (p_ring[0] + p_ring[valence - 1]));
        for (int k = 1; k < valence - 1; ++k) {
          Float wt = (2 * std::cos(theta) - 2) * std::sin((k)*theta);
          t += Vector3f(wt * p_ring[k]);
        }
        t = -t;
      }
    }

    ns.emplace_back(cross(s, t));
  }

  {
    size_t ntris = f.size();
    std::vector<int> verts(3 * ntris);
    int *vp = verts.data();
    size_t tot_verts = v.size();
    std::map<SdVertex *, int> used_verts;
    for (size_t i = 0; i < tot_verts; ++i) {
      used_verts[v[i]] = i;
    }

    for (size_t i = 0; i < ntris; ++i) {
      for (auto *vv : f[i]->v) {
        *vp = used_verts[vv];
        ++vp;
      }
    }

    return alloc.new_object<TriangleMesh>(*render_from_object, reverse_orientation, verts, p_limit,
                                          std::vector<Vector3f>(), ns, std::vector<Point2f>(),
                                          std::vector<int>(), alloc);
  }
}
