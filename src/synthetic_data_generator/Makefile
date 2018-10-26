all: gen.cpp sample_t.hpp params.hpp sample_file.hpp sample_file.o params.o file_utils.o
	${CXX} -std=c++11 -Wall -o gen gen.cpp sample_file.o params.o file_utils.o

params.o: sample_t.hpp params.hpp params.cpp
	${CXX} -std=c++11 -Wall -c params.cpp

sample_files.o: sample_t.hpp params.o file_utils.o sample_files.hpp sample_files.cpp
	${CXX} -std=c++11 -Wall -c sample_files.cpp params.o

file_utils.o: file_utils.hpp file_utils.cpp
	${CXX} -std=c++11 -Wall -c file_utils.cpp


clean:
	@rm -f gen *.o

cleandata:
	@rm -f sample_file.*.txt sample_file.*.bin
