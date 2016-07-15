#include "InstructionFilter.hpp"
#include "RangeFilter.hpp"

// for more complex filters yet to come

RangeSetInstructionFilter::RangeSetInstructionFilter(char const *filename)
	: _filters()
{
  fail::Logger log("InstFilter", false);
  parseRangeFile(*this, filename, log);
}
