#ifndef __SSG_SAMPLE_FILE_HPP__
#define __SSG_SAMPLE_FILE_HPP__
#include <string>
#include <vector>
#include "sample_t.hpp"
#include "params.hpp"

namespace synthetic_sample_generator {

bool generate_a_file(const std::string& ofile_name, const unsigned num_samples,
                     params& p, std::vector<sample_t>& samples);


bool load_a_file(const std::string& ifile_name, std::vector<sample_t>& samples);

bool compare(const std::string& file1, const std::string& file2, const params& p);

} // end of namespce synthetic_sample_generator
#endif // __SSG_SAMPLE_FILE_HPP__

