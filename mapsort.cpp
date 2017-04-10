// Copyright 2016 - Stefano Sinigardi, Alessandro Fabbri

/************************************************************************
* This program is free software: you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation, either version 3 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
* This program is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
* GNU General Public License for more details.                          *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with this program.  If not, see <http://www.gnu.org/licenses/>. *
************************************************************************/


#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>  
#include <chrono>
#include <string>

#include <boost/algorithm/string.hpp>

constexpr char SEPARATORS[] = "|";
constexpr size_t SIZE = 15;

#define MAJOR_VERSION		          1
#define MINOR_VERSION		          0

//////// Templated timed function
template<typename TimeT>
struct measure
{
  template<typename F, typename ...Args>
  static typename TimeT::rep execution(F&& func, Args&&... args)
  {
    auto start = std::chrono::steady_clock::now();
    std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
    auto duration = std::chrono::duration_cast<TimeT> (std::chrono::steady_clock::now() - start);
    return duration.count();
  }
};
typedef measure<std::chrono::milliseconds> time_ms;

// a couple of lines as an example
// note that the program assumes that the first two values are ALWAYS the same (it stores just the first!)
// uint    | uint    | long long int | --> unsigned int    | short | double    | double    | unsigned short | float | float | float  | 8xchar  | 32xchar      | unsigned int | 32xchar
// 1234567 | 1234567 | -1            | 2016-01-01 09:00:00 | 2     | 45.12345  | 12.012345 | 3              | 0.0   | 0.0   | 736.0  | AA123BC | 1 Via Roma   | 10000        | Ridente luogo
// 8901234 | 8901234 | 313131313131  | 2016-01-01 10:10:00 | 1     | 43.543210 | 11.678901 | 8              | -1.0  | -1.0  | 1027.0 | DE456FG | 8 Via Milano | 50000        | Passo felice
typedef struct map_data {
  //the order of the variable is such to maximize memory alignment. Reconsider alignment if moving strings to char[]
  long long int lli3;
  double d6;
  double d7;
  float f9;
  float f10;
  float f11;
  unsigned int ui1;
  unsigned int ui14;
  unsigned short us8;
  short s5;

  //what follows is terrible, maybe change it to a fixed char[n] would be ideal for performances and allocation strategies during runtime
  std::string str4;
  std::string str12;
  std::string str13;
  std::string str15;
} map_data;

size_t count_lines(const std::string& input_file) {
  std::ifstream file_in(input_file.c_str());
  if (!file_in) {
    std::cerr << "ERROR: unable to open " << input_file << std::endl;
    exit(2);
  }
  else std::cout << "Counting lines in file " << input_file << "..." << std::endl;

  std::string line;
  size_t counter = 0;

  while (std::getline(file_in, line)) counter++;
  std::cout << "Found " << counter << " lines" << std::endl;

  return counter;
}

void map_parse(const std::string& input_file, std::vector<map_data>& records, size_t& counter, std::vector<std::string>& errors) {
  std::ifstream file_in(input_file.c_str());
  if (!file_in) {
    std::cerr << "ERROR: unable to open " << input_file << std::endl;
    exit(2);
  }
  else std::cout << "Parsing " << input_file << "..." << std::endl;

  std::string line;
  std::vector<std::string> tokens;
  counter = 0;
  time_t splitting_time = 0;

  while (std::getline(file_in, line)) {
    splitting_time += time_ms::execution(boost::algorithm::split<std::vector<std::string>, std::string, decltype(boost::is_any_of(SEPARATORS))>, tokens, line, boost::is_any_of(SEPARATORS), boost::algorithm::token_compress_off);

    if (tokens.size() != SIZE) {
      errors.push_back(line);
      continue;
    }

    map_data record;
    try {
      record.ui1 = std::stoul(tokens[0]);
      record.lli3 = std::stoll(tokens[2]);
      record.str4 = tokens[3];
      record.s5 = (short)std::stoi(tokens[4]);
      record.d6 = std::stod(tokens[5]);
      record.d7 = std::stod(tokens[6]);
      record.us8 = (unsigned short)std::stoul(tokens[7]);
      record.f9 = (float)std::stod(tokens[8]);
      record.f10 = (float)std::stod(tokens[9]);
      record.f11 = (float)std::stod(tokens[10]);
      record.str12 = tokens[11];
      record.str13 = tokens[12];
      record.ui14 = std::stoul(tokens[13]);
      record.str15 = tokens[14];
    }
    catch (const std::exception& e) {
      std::cerr << "Warning: " << e.what() << std::endl;
      errors.push_back(line);
      continue;
    }
    records.push_back(std::move(record));
  }
  file_in.close();

  std::cout << "TIMING: Splitting took (ms)" << splitting_time << std::endl;
}


void map_sort(std::vector<map_data>& records, const size_t& counter) {
  std::cout << "Sorting..." << std::endl;
  std::sort(records.begin(), records.begin() + counter, [](map_data a, map_data b) {
    if (a.ui1 < b.ui1)
      return true;
    else if (a.ui1 == b.ui1)
      return (a.str4 < b.str4);
    else
      return false;
  });
}


void map_write(const std::string& input_file, const std::vector<map_data>& records, const size_t& counter, const std::vector<std::string>& errors) {
  std::ofstream fileout;
  std::string file_basename;
  std::vector<std::string> tokens;

  long int lat = 0;
  long int lon = 0;

  boost::split(tokens, input_file, boost::is_any_of("."));
  for (size_t t = 0; t < tokens.size() - 1; t++) {
    if (t == 0 && tokens[t] == "") {
      file_basename += ".";
      continue;
    }
    file_basename += tokens[t];
    if (t < tokens.size() - 2) file_basename += ".";
  }

  if (counter) {
    fileout.open(file_basename + "_sorted.txt");
    if (!fileout) {
      std::cerr << "ERROR: Unable to create output file stream for sorted data" << std::endl;
      exit(100);
    }
    else {
      std::cout << "Writing output file..." << std::endl;
    }
	for (size_t index = 0; index < counter; index++) {

		lat = long((records[index].d6) * 1000000);
		lon = long((records[index].d7) * 1000000);

		fileout << records[index].ui1 << '\t' << records[index].ui1 << '\t' << records[index].str4 << '\t' << records[index].s5 << '\t' <<
			/*records[index].d6*/ lat << '\t' << /*records[index].d7*/ lon << '\t' << records[index].us8 << '\t' << records[index].f9 << '\t' << records[index].f10 << '\t' <<
			records[index].f11  /*<< '\t' << records[index].str12 << '|' << records[index].str13 << '|' << records[index].ui14 << '|' << records[index].str15 */ << std::endl;
	}
    fileout.close();
  }

  if (errors.size()) {
    fileout.open(file_basename + "_err.txt");
    if (!fileout) {
      std::cerr << "ERROR: Unable to create output file stream for errors" << std::endl;
      exit(200);
    }
    for (auto e : errors)
      fileout << e << std::endl;
    fileout.close();
  }
}


//////// MAIN
void usage(const std::string& progname) {
  std::vector<std::string> tokens;
  boost::split(tokens, progname, boost::is_any_of("/\\"));
  std::cerr << "Usage: " << tokens.back() << " filename " << std::endl;
  std::cerr << "it will sort your map file based on the defined routine" << std::endl;
}

int main(int argc, char *argv[]) {
  std::cout << "mapsort v" << MAJOR_VERSION << "." << MINOR_VERSION << std::endl;
  std::string input_name;
  if (argc == 2) {
    input_name = argv[1];
    if (input_name.substr(0, 2) == ".\\" || input_name.substr(0, 2) == "./") input_name = input_name.substr(2, input_name.size() - 2);
  }
  else {
    usage(argv[0]);
    exit(-2);
  }

  std::vector<std::string> errors;
  std::cout << "Sorting : " << argv[1] << std::endl;
  size_t lines = count_lines(input_name);
  size_t counter;
  std::vector<map_data> records;
  records.reserve(lines); // use this if you prefer to push_back instead of accessing elements directly (which is less safe)

  auto dt = time_ms::execution(map_parse, input_name, records, counter, errors);
  std::cout << "TIMING: Parse took " << dt << " ms" << std::endl;

  dt = time_ms::execution(map_sort, records, counter);
  std::cout << "TIMING: Sort took " << dt << " ms" << std::endl;

  dt = time_ms::execution(map_write, input_name, records, counter, errors);
  std::cout << "TIMING: Write took" << dt << " ms" << std::endl;

  return 0;
}



