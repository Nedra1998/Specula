#ifndef SPECULA_OPTIONS_HPP
#define SPECULA_OPTIONS_HPP

namespace specula {
  struct BasicSpeculaOptions {
    bool use_gpu = false;
  };

  struct SpeculaOptions : BasicSpeculaOptions {
    unsigned n_threads = 0;
  };
} // namespace specula

#endif // SPECULA_OPTIONS_HPP
