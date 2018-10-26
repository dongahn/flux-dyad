#ifndef __SSG_PARAMS_HPP__
#define __SSG_PARAMS_HPP__
#include "sample_t.hpp"
#include <string>
#include <random>
#include <chrono>

namespace synthetic_sample_generator {

struct params
{
  void set_seeds(unsigned seed1, unsigned seed2, unsigned seed3);
  void set_seeds();
  bool init_generate(int argc, char** argv);
  bool init_compare(int argc, char** argv);
  sample_t generate_a_sample();

  unsigned n_samples;
  unsigned n_per_file;
  unsigned n_files;
  unsigned n_full_files;
  unsigned n_remainder;
  std::string out_dir;

  val_t scale_mean;
  val_t scale_var;
  val_t bias_mean;
  val_t bias_var;
  sample_t epsilon;

  bool is_binary;

  unsigned X_seed;
  unsigned scale_seed;
  unsigned bias_seed;

  std::default_random_engine X_gen;
  std::default_random_engine scale_gen;
  std::default_random_engine bias_gen;

  using dist_param_t = std::normal_distribution<val_t>::param_type;

  std::normal_distribution<val_t> X_distribution;
  std::normal_distribution<val_t> scale_distribution;
  std::normal_distribution<val_t> bias_distribution;
};

} // end of namespce synthetic_sample_generator
#endif // __SSG_PARAMS_T_HPP__
