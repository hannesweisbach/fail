#include <fstream>
#include <string>

#include "sal/SALConfig.hpp"
#include "util/Logger.hpp"

template <typename Filter>
void parseRangeFile(Filter &filter, const char *filename, fail::Logger& log) {

  std::ifstream filterFile(filename, std::ios::in);
  if (!filterFile.is_open()) {
    log << filename << " not found" << std::endl;
    return;
  }

  std::string buf;
  while (std::getline(filterFile, buf)) {
    if (buf.empty() || buf[0] == '#')
      continue;

    fail::address_t start, end;
    std::stringstream ss(buf, std::ios::in);
    ss >> std::hex >> start >> end;
    log << "new filter range: " << std::hex << start << " - " << end
        << std::endl;
    filter.addFilter(start, end);
  }
}
