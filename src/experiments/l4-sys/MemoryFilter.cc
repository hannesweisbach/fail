#include "MemoryFilter.hpp"
#include "RangeFilter.hpp"

RangeSetMemoryFilter::RangeSetMemoryFilter(char const *filename)
	: _filters()
{
  fail::Logger log("MemFilter", false);
  parseRangeFile(*this, filename, log);
}
