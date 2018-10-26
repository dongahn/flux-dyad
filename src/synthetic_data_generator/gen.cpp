#include <iostream>
#include <string>
#include <vector>

#include "sample_t.hpp"
#include "params.hpp"
#include "sample_file.hpp"

using namespace synthetic_sample_generator;


void show_help(const std::string executable_name)
{
  std::cout << "Usage1: " << executable_name << "out_dir num_samples num_samples_per_file scale_mean scale_variance bias_mean bias_variance mode" << std::endl;
  std::cout << "        Generate regression samples [X, Y] as many as 'num_samples'," << std::endl;
  std::cout << "        where X is random, and Y is roughly \"scale_mean * X + bias_mean\"." << std::endl;
  std::cout << "        Pack samples into files, each of which contains 'num_samples_per_file' samples, written into 'out_dir'." << std::endl;
  std::cout << "        mode 0: text, mode 1: binary" << std::endl;
  std::cout << "Usage2: " << executable_name << " file1 file2 scale_mean scale_variance bias_mean bias_variance" << std::endl;
  std::cout << "        Compare file1 and file2. Only supports files generated using static seeds" << std::endl;
  std::cout << "Usage3: " << executable_name << " file" << std::endl;
  std::cout << "        Read a file and print it out" << std::endl;
}

int main(int argc, char** argv)
{
  params p;
  p.set_seeds(311, 511, 911);

  if (argc == 2) {
    std::vector<sample_t> samples;
    load_a_file(argv[1], samples);
    return 0;
  } else if (argc == 7) {
    const std::string file1 = argv[1];
    const std::string file2 = argv[2];

    if (!p.init_compare(argc, argv)) {
      return 0;
    }

    bool is_same = compare(file1, file2, p);

    std::cout << file1 << " and " << file2 << " are " << (is_same? "same" : "different") << std::endl;
    return 0;
  } else if (argc != 9) {
    show_help(argv[0]);
    return 0;
  }

  if (!p.init_generate(argc, argv)) {
    return 0;
  }

  std::vector<sample_t> samples;
  samples.reserve(p.n_per_file);

  for (unsigned nf = 0u; nf < p.n_full_files; ++nf) {
    const std::string ofile_name = p.out_dir + "sample_file." + std::to_string(nf) + (p.is_binary ? ".bin" : ".txt");
    generate_a_file(ofile_name, p.n_per_file, p, samples);
  }

  if (p.n_remainder > 0u) {
    const std::string ofile_name = p.out_dir + "sample_file." + std::to_string(p.n_full_files) + (p.is_binary ? ".bin" : ".txt");
    generate_a_file(ofile_name, p.n_remainder, p, samples);
  }

  return 0;
}
