#ifndef __L4SYS_MEMORYFILTER_HPP__
  #define __L4SYS_MEMORYFILTER_HPP__

#include "sal/SALConfig.hpp"
#include "cpu/instr.h"
#include <vector>

using namespace fail;

/**
 * \class MemoryFilter
 *
 * \brief Filters memory addresses
 *
 * This class is an interface that can be used to
 * implement memory filter classes that can
 * be used to decide if a memory address should be
 * excluded in the fault injection experiment.
 */
class MemoryFilter {
public:
	/**
	 * Decides if the given memory address at the given program
	 * location is invalid for fault injection
	 * @param addr the memory address in question
	 * @returns \c true if the address should be excluded,
	 *          \c false otherwise
	 */
	virtual bool isInvalid(address_t addr) const = 0;
	virtual ~MemoryFilter() {}
};

/**
 * \class RangeMemoryFilter
 *
 * Excludes a memory address if it lies within a certain range
 */
class RangeMemoryFilter : public MemoryFilter {
public:
	RangeMemoryFilter(address_t begin_addr, address_t end_addr)
	: beginAddress(begin_addr), endAddress(end_addr)
	{}
	~RangeMemoryFilter() {}
	/**
	 * check if the memory address lies within a certain range
	 * @param addr the memory address in question
	 * @returns \c true if the address lies within the predefined range,
	 *          \c false otherwise
	 */
	bool isInvalid(address_t addr) const
	{ return (beginAddress <= addr && addr < endAddress); }
private:
	address_t beginAddress; //<! the beginning of the address range
	address_t endAddress; //<! the end of the address range
};


/**
 * \class RangeSetMemoryFilter
 * 
 * Collects a list of filter ranges from an input file.
 */
class RangeSetMemoryFilter : public MemoryFilter {
private:
  std::vector<RangeMemoryFilter> _filters;

public:
  RangeSetMemoryFilter(char const *filename);
  ~RangeSetMemoryFilter() {}

  bool isInvalid(address_t addr) const 
  {
    for (const auto &filter : _filters) {
      if (filter.isInvalid(addr))
        return true;
    }

    return false;
	}
	
	void addFilter(address_t startAddr, address_t endAddr)
	{
		_filters.push_back(RangeMemoryFilter(startAddr, endAddr));
	}
};

#endif /* __L4SYS_MEMORYFILTER_HPP__ */
