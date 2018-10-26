#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cmath>
#include "sample_t.hpp"
#include "sample_file.hpp"

namespace synthetic_sample_generator {

bool generate_a_file(const std::string& ofile_name,
                     const unsigned num_samples,
                     params& p,
                     std::vector<sample_t>& samples)
{
    samples.clear();

    for (unsigned ns = 0u; ns < num_samples; ++ns) {
      samples.emplace_back(p.generate_a_sample());
    }

    std::ios_base::openmode ofile_mode = p.is_binary? (std::ios::out | std::ios::binary) : std::ios::out;
    std::ofstream ofile{ofile_name, ofile_mode};

    if (p.is_binary) {
    #if 0
      std::for_each(samples.cbegin(), samples.cend(),
                    print_pair<sample_t, true>(ofile));
    #else
      ofile.write(reinterpret_cast<const char*>(samples.data()), samples.size()*sizeof(sample_t));
    #endif
    } else {
      std::for_each(samples.cbegin(), samples.cend(),
                    print_pair<sample_t, false>(ofile));
    }

    ofile.close();

  #if defined(__SSG_DEBUG__)
    for (const auto& sample: samples) {
      std::cout << sample.first << "\t" << sample.second << std::endl;
    }
  #endif

    return true;
}


bool load_a_file(const std::string& ifile_name,
                 std::vector<sample_t>& samples)
{
  samples.clear();

  const std::size_t found = ifile_name.find_last_of(".");
  const std::string file_ext = ifile_name.substr(found+1);

  const bool is_binary = (file_ext != "txt");

  std::ios_base::openmode ifile_mode = is_binary? (std::ios::in | std::ios::binary) : std::ios::in;
  std::ifstream ifile{ifile_name, ifile_mode};

  if (!ifile.is_open()) {
    std::cerr << "Unable to open file: " << ifile_name << std::endl;
    return false;
  }

  if (is_binary) {
    ifile.seekg(0,  std::ios::end);
    const std::streampos fsize = ifile.tellg();
    const unsigned num_samples = fsize / sizeof(sample_t);
    if (static_cast<std::streampos>(num_samples * sizeof(sample_t)) != fsize) {
      std::cerr << "Corrupted file? " << num_samples << " * " << sizeof(sample_t) << " != " << fsize << std::endl;
      ifile.close();
      return false;
    }
    ifile.seekg(0, std::ios::beg);
    samples.resize(num_samples);
    ifile.read(reinterpret_cast<char*>(samples.data()), num_samples*sizeof(sample_t));
  } else {
    std::string sample;
    while ( getline (ifile, sample) )
    {
      const std::size_t found = sample.find_first_of(",");
      if (found == std::string::npos) {
        std::cerr << "Invalid format: missing comma in \"" << sample << '"' << std::endl;
        ifile.close();
        return false;
      }
      const std::string X_string = sample.substr(0, found);
      const std::string Y_string = sample.substr(found+1);
      std::istringstream X_sstr(X_string);
      std::istringstream Y_sstr(Y_string);
      val_t X, Y;
      X_sstr >> X;
      Y_sstr >> Y;
      samples.emplace_back(std::make_pair(X,Y));
    }
  }

  ifile.close();

#if defined(__SSG_DEBUG__)
  for (const auto& sample: samples) {
    std::cout << sample.first << "\t" << sample.second << std::endl;
  }
#endif
  return true;
}

bool compare(const std::string& file1, const std::string& file2, const params& p)
{
  bool ok = true;

  std::vector<sample_t> samples1;
  std::vector<sample_t> samples2;

  ok = ok && load_a_file(file1, samples1);
  ok = ok && load_a_file(file2, samples2);
  ok = ok && (samples1.size() == samples2.size());

  for(size_t i = 0u; i < samples1.size(); ++i) {
    if ((std::abs(samples1[i].first - samples2[i].first) > p.epsilon.first) || (std::abs(samples1[i].second - samples2[i].second) > p.epsilon.second)) {
      ok = false;

      std::cerr << "At line " << i << std::setprecision(_SSG_TXT_OUTPUT_PRECISION_) << " (" << samples1[i].first << ' ' << samples1[i].second << ") != (" << samples2[i].first << ' ' << samples2[i].second << ')' << std::endl;
      if (std::abs(samples1[i].first - samples2[i].first) > p.epsilon.first) {
        std::cerr << std::abs(samples1[i].first - samples2[i].first) << " > " << p.epsilon.first << std::endl;
      } else {
        std::cerr << std::abs(samples1[i].second - samples2[i].second) << " > " << p.epsilon.second << std::endl;
      }
      break;
    }
  }

  return ok;
}

} // end of namespce synthetic_sample_generator

