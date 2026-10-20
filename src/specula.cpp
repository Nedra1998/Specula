#include "specula/specula.hpp"

#include <ImfThreading.h>

#include "specula/options.hpp"
#include "specula/types.hpp"
#include "specula/util/buffer_cache.hpp"
#include "specula/util/color.hpp"
#include "specula/util/colorspace.hpp"
#include "specula/util/mesh/triangle_mesh.hpp"
#include "specula/util/parallel/functions.hpp"
#include "specula/util/spectrum/spectra.hpp"

bool specula::initialize(const SpeculaOptions &opts) {
  Imf::setGlobalThreadCount(opts.n_threads != 0 ? opts.n_threads : available_cores());
  parallel_init(opts.n_threads != 0 ? opts.n_threads : available_cores());

  // TODO: implement windows specific setup

  if (opts.use_gpu) {
#ifdef SPECULA_BUILD_GPU_RENDERER
    // TODO: Impliment GPU specific setup
#else
    LOG_CRITICAL("OPTIONS::use_gpu set with non-GPU build");
    return false;
#endif
  } else {
    Allocator alloc;
    ColorEncoding::init(alloc);
    spectra::init(alloc);
    RgbToSpectrumTable::init(alloc);
    RgbColorSpace::init(alloc);
  }

  init_buffer_caches();

  return true;
}

bool specula::cleanup() {
  parallel_cleanup();
  return true;
}
