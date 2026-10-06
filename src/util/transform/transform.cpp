#include "specula/util/transform/transform.hpp"

#include <math.h>

#include "specula/macros.hpp"
#include "specula/util/log.hpp"
#include "specula/util/math.hpp"
#include "specula/util/vecmath.hpp"

SPECULA_CPU_GPU specula::Bounds3f specula::Transform::operator()(const Bounds3f &b) const {
  Bounds3f bt;
  for (int i = 0; i < 8; ++i) {
    bt = bunion(bt, (*this)(b.corner(i)));
  }
  return bt;
}

SPECULA_CPU_GPU specula::Transform::operator specula::Quaternion() const {
  Float trace = m[0][0] + m[1][1] + m[2][2];
  Quaternion quat;
  if (trace > 0.0f) {
    Float s = std::sqrt(trace + 1.0f);
    quat.w = s / 2.0f;
    s = 0.5f / s;

    quat.v.x = (m[2][1] - m[1][2]) * s;
    quat.v.y = (m[0][2] - m[2][0]) * s;
    quat.v.z = (m[1][0] - m[0][1]) * s;
  } else {
    const int nxt[3] = {1, 2, 0};
    Float q[3];
    int i = 0;
    if (m[1][1] > m[0][0]) {
      i = 1;
    }
    if (m[2][2] > m[i][i]) {
      i = 2;
    }

    int j = nxt[i];
    int k = nxt[j];
    Float s = safe_sqrt((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);
    q[i] = s * 0.5f;
    if (s != 0.5f) {
      s = 0.5f / s;
    }
    quat.w = (m[k][j] - m[j][k]) * s;
    q[j] = (m[j][i] + m[i][j]) * s;
    q[k] = (m[k][i] + m[i][k]) * s;

    quat.v.x = q[0];
    quat.v.y = q[1];
    quat.v.z = q[2];
  }

  return quat;
}

SPECULA_CPU_GPU specula::Transform specula::Transform::operator*(const Transform &t2) const {
  return {m * t2.m, t2.minv * minv};
}

SPECULA_CPU_GPU bool specula::Transform::swap_handedness() const {
  SquareMatrix<3> s(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1],
                    m[2][2]);
  return determinant(s) < 0;
}

SPECULA_CPU_GPU [[nodiscard]] specula::Point3fi
specula::Transform::apply_inverse(const Point3fi &p) const {
  auto x = Float(p.x), y = Float(p.y), z = Float(p.z);
  Float xp = minv[0][0] * x + minv[0][1] * y + minv[0][2] * z + minv[0][3];
  Float yp = minv[1][0] * x + minv[1][1] * y + minv[1][2] * z + minv[1][3];
  Float zp = minv[2][0] * x + minv[2][1] * y + minv[2][2] * z + minv[2][3];
  Float wp = minv[3][0] * x + minv[3][1] * y + minv[3][2] * z + minv[3][3];

  Vector3f perror;
  if (p.is_exact()) {
    perror.x = gamma(3) * (std::abs(minv[0][0] * x) + std::abs(minv[0][1] * y) +
                           std::abs(minv[0][2] * z) + std::abs(minv[0][3]));
    perror.y = gamma(3) * (std::abs(minv[1][0] * x) + std::abs(minv[1][1] * y) +
                           std::abs(minv[1][2] * z) + std::abs(minv[1][3]));
    perror.z = gamma(3) * (std::abs(minv[2][0] * x) + std::abs(minv[2][1] * y) +
                           std::abs(minv[2][2] * z) + std::abs(minv[2][3]));
  } else {
    Vector3f pin_error = p.error();

    perror.x =
        (gamma(3) + 1) * (std::abs(minv[0][0]) * pin_error.x + std::abs(minv[0][1]) * pin_error.y +
                          std::abs(minv[0][2]) * pin_error.z) +
        gamma(3) * (std::abs(minv[0][0] * x) + std::abs(minv[0][1] * y) + std::abs(minv[0][2] * z) +
                    std::abs(minv[0][3]));
    perror.y =
        (gamma(3) + 1) * (std::abs(minv[1][0]) * pin_error.x + std::abs(minv[1][1]) * pin_error.y +
                          std::abs(minv[1][2]) * pin_error.z) +
        gamma(3) * (std::abs(minv[1][0] * x) + std::abs(minv[1][1] * y) + std::abs(minv[1][2] * z) +
                    std::abs(minv[1][3]));
    perror.z =
        (gamma(3) + 1) * (std::abs(minv[2][0]) * pin_error.x + std::abs(minv[2][1]) * pin_error.y +
                          std::abs(minv[2][2]) * pin_error.z) +
        gamma(3) * (std::abs(minv[2][0] * x) + std::abs(minv[2][1] * y) + std::abs(minv[2][2] * z) +
                    std::abs(minv[2][3]));
  }

  if (wp == 1) {
    return {Point3f(xp, yp, zp), perror};
  } else {
    return Point3fi(Point3f(xp, yp, zp), perror) / wp;
  }
}

void specula::Transform::decompose(Vector3f *t, SquareMatrix<4> *r, SquareMatrix<4> *s) const {
  t->x = m[0][3];
  t->y = m[1][3];
  t->z = m[2][3];

  SquareMatrix<4> mat = m;
  for (int i = 0; i < 3; ++i) {
    mat[i][3] = mat[3][i] = 0.f;
  }
  mat[3][3] = 1.0f;

  Float norm = NAN;
  int count = 0;
  *r = mat;
  do {
    SquareMatrix<4> rit = invert_or_exit(transpose(*r));
    SquareMatrix<4> rnext = (*r + rit) / 2;

    norm = 0;
    for (int i = 0; i < 3; ++i) {
      Float n = std::abs((*r)[i][0] - rnext[i][0]) + std::abs((*r)[i][1] - rnext[i][1]) +
                std::abs((*r)[i][1] - rnext[i][1]);
      norm = std::max(norm, n);
    }

    *r = rnext;
  } while (++count < 100 && norm > 0.0001);

  // TODO: deal with flip?

  *s = invert_or_exit(*r) * mat;
}

SPECULA_CPU_GPU specula::Transform specula::translate(Vector3f delta) {
  SquareMatrix<4> m(1, 0, 0, delta.x, 0, 1, 0, delta.y, 0, 0, 1, delta.z, 0, 0, 0, 1);
  SquareMatrix<4> minv(1, 0, 0, -delta.x, 0, 1, 0, -delta.y, 0, 0, 1, -delta.z, 0, 0, 0, 1);
  return {m, minv};
}

SPECULA_CPU_GPU specula::Transform specula::scale(Float x, Float y, Float z) {
  SquareMatrix<4> m(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1);
  SquareMatrix<4> minv(1 / x, 0, 0, 0, 0, 1 / y, 0, 0, 0, 0, 1 / z, 0, 0, 0, 0, 1);
  return {m, minv};
}

SPECULA_CPU_GPU specula::Transform specula::rotate_x(Float theta) {
  Float sin_theta = std::sin(radians(theta));
  Float cos_theta = std::cos(radians(theta));
  SquareMatrix<4> m(1, 0, 0, 0, 0, cos_theta, -sin_theta, 0, 0, sin_theta, cos_theta, 0, 0, 0, 0,
                    1);
  return {m, transpose(m)};
}

SPECULA_CPU_GPU specula::Transform specula::rotate_y(Float theta) {
  Float sin_theta = std::sin(radians(theta));
  Float cos_theta = std::cos(radians(theta));
  SquareMatrix<4> m(cos_theta, 0, sin_theta, 0, 0, 1, 0, 0, -sin_theta, 0, cos_theta, 0, 0, 0, 0,
                    1);
  return {m, transpose(m)};
}

SPECULA_CPU_GPU specula::Transform specula::rotate_z(Float theta) {
  Float sin_theta = std::sin(radians(theta));
  Float cos_theta = std::cos(radians(theta));
  SquareMatrix<4> m(cos_theta, -sin_theta, 0, 0, sin_theta, cos_theta, 0, 0, 0, 0, 1, 0, 0, 0, 0,
                    1);
  return {m, transpose(m)};
}

SPECULA_CPU_GPU specula::Transform specula::look_at(Point3f pos, Point3f look, Vector3f up) {
  SquareMatrix<4> world_from_camera;
  world_from_camera[0][3] = pos.x;
  world_from_camera[1][3] = pos.y;
  world_from_camera[2][3] = pos.z;
  world_from_camera[3][3] = 1;

  Vector3f dir = normalize(look - pos);
  if (length(cross(normalize(up), dir)) == 0) {
    LOG_CRITICAL(
        "look_at 'up' vector {} and viewsing direction {} are pointing int he same direction", up,
        dir);
  }

  Vector3f right = normalize(cross(normalize(up), dir));
  Vector3f new_up = cross(dir, right);

  world_from_camera[0][0] = right.x;
  world_from_camera[1][0] = right.y;
  world_from_camera[2][0] = right.z;
  world_from_camera[3][0] = 0.0;

  world_from_camera[0][1] = new_up.x;
  world_from_camera[1][1] = new_up.y;
  world_from_camera[2][1] = new_up.z;
  world_from_camera[3][1] = 0.0;

  world_from_camera[0][2] = dir.x;
  world_from_camera[1][2] = dir.y;
  world_from_camera[2][2] = dir.z;
  world_from_camera[3][2] = 0.0;

  SquareMatrix<4> camera_from_world = invert_or_exit(world_from_camera);
  return {camera_from_world, world_from_camera};
}

SPECULA_CPU_GPU specula::Transform specula::orthographic(Float znear, Float zfar) {
  return scale(1, 1, 1 / (zfar - znear)) * translate(Vector3f(0, 0, -znear));
}

SPECULA_CPU_GPU specula::Transform specula::perspective(Float fov, Float znear, Float zfar) {
  SquareMatrix<4> persp(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, zfar / (zfar - znear),
                        -zfar * znear / (zfar - znear), 0, 0, 1, 0);
  Float inv_tan_ang = 1 / std::tan(radians(fov) / 2);
  return scale(inv_tan_ang, inv_tan_ang, 1) * Transform(persp);
}
