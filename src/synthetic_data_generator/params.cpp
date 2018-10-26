#include "sample_t.hpp"
#include <string>
#include <random>
#include <chrono>
#include "params.hpp"
#include "file_utils.hpp"

namespace synthetic_sample_generator {

void params::set_seeds(unsigned seed1, unsigned seed2, unsigned seed3)
{
  X_seed = seed1;
  scale_seed = seed2;
  bias_seed = seed3;
}

void params::set_seeds()
{
  X_seed = std::chrono::system_clock::now().time_since_epoch().count();
  scale_seed = std::chrono::system_clock::now().time_since_epoch().count();
  bias_seed = std::chrono::system_clock::now().time_since_epoch().count();
}

bool params::init_generate(int argc, char** argv)
{
  if (argc != 9) {
    return false;
  }

  out_dir = argv[1];
  if ((out_dir.size() > 0u) && (out_dir.back() != '/')) {
    out_dir += '/';
  }
  create_dir(out_dir);

  n_samples = static_cast<unsigned>(std::max(atoi(argv[2]), 0));
  n_per_file = static_cast<unsigned>(std::max(atoi(argv[3]), 1));
  n_files = (n_samples + n_per_file - 1) / n_per_file;
  n_full_files = n_samples / n_per_file;
  n_remainder = n_samples - n_per_file * n_full_files;

  scale_mean = static_cast<unsigned>(atof(argv[4]));
  scale_var = static_cast<unsigned>(atof(argv[5]));
  bias_mean = static_cast<unsigned>(atof(argv[6]));
  bias_var = static_cast<unsigned>(atof(argv[7]));

  is_binary = static_cast<bool>(atoi(argv[8]));

  X_gen.seed(X_seed);
  scale_gen.seed(scale_seed);
  bias_gen.seed(bias_seed);

  X_distribution.param(dist_param_t(0.0, 1.0));
  X_distribution.reset();

  scale_distribution.param(dist_param_t(scale_mean, scale_var));
  scale_distribution.reset();

  bias_distribution.param(dist_param_t(bias_mean, bias_var));
  bias_distribution.reset();

  return true;
};


bool params::init_compare(int argc, char** argv)
{
  if (argc != 7) {
    return false;
  }

  scale_mean = static_cast<unsigned>(atof(argv[3]));
  scale_var = static_cast<unsigned>(atof(argv[4]));
  bias_mean = static_cast<unsigned>(atof(argv[5]));
  bias_var = static_cast<unsigned>(atof(argv[6]));

  epsilon.first = std::pow(10, 1 - _SSG_TXT_OUTPUT_PRECISION_);
  epsilon.second = 10 * epsilon.first * (std::abs(scale_mean) + std::abs(bias_mean));

  return true;
};


sample_t params::generate_a_sample()
{
  const val_t X     = X_distribution(X_gen);
  const val_t scale = scale_distribution(scale_gen);
  const val_t bias  = bias_distribution(bias_gen);
    
  const val_t Y = X * scale + bias;

  return sample_t(X, Y);
}

} // end of namespce synthetic_sample_generator
