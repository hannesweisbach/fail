#ifndef __RANDOM_JUMP_IMPORTER_H__
#define __RANDOM_JUMP_IMPORTER_H__

#include <vector>
#include "util/CommandLine.hpp"
#include "Importer.hpp"

#include "util/llvmdisassembler/LLVMDisassembler.hpp"

class RandomJumpImporter : public Importer {
	llvm::OwningPtr<llvm::object::Binary> binary;
	llvm::OwningPtr<fail::LLVMDisassembler> disas;

	fail::CommandLine::option_handle FROM, TO;

	fail::MemoryMap *m_mm_from, *m_mm_to;
	std::vector<fail::guest_address_t> m_jump_to_addresses;
public:
	RandomJumpImporter();

protected:
	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
								 Trace_Event &ev);
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
								  Trace_Event &ev) {
		/* ignore on purpose */
		return true;
	}

	virtual void open_unused_ec_intervals() {
		/* empty, Memory Map has a different meaning in this importer */
	}

	void getAliases(std::deque<std::string> *aliases) {
		aliases->push_back("RandomJumpImporter");
	}
};

#endif
