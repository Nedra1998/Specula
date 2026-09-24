#include "specula/util/buffer_cache.hpp"

#include "specula/util/check.hpp"
#include "specula/util/vecmath.hpp"

specula::BufferCache<int> *specula::INT_BUFFER_CACHE;
specula::BufferCache<specula::Point2f> *specula::POINT2_BUFFER_CACHE;
specula::BufferCache<specula::Point3f> *specula::POINT3_BUFFER_CACHE;
specula::BufferCache<specula::Vector3f> *specula::VECTOR3_BUFFER_CACHE;
specula::BufferCache<specula::Normal3f> *specula::NORMAL3_BUFFER_CACHE;

void specula::init_buffer_caches() {
  ASSERT(INT_BUFFER_CACHE == nullptr);
  INT_BUFFER_CACHE = new BufferCache<int>;
  POINT2_BUFFER_CACHE = new BufferCache<Point2f>;
  POINT3_BUFFER_CACHE = new BufferCache<Point3f>;
  VECTOR3_BUFFER_CACHE = new BufferCache<Vector3f>;
  NORMAL3_BUFFER_CACHE = new BufferCache<Normal3f>;
}
