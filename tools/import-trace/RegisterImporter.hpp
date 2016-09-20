#ifndef __REGISTER_IMPORTER_H__
#define __REGISTER_IMPORTER_H__

#include <set>
#include "util/CommandLine.hpp"
#include "Importer.hpp"

#include "util/llvmdisassembler/LLVMDisassembler.hpp"


class RegisterImporter : public Importer {
	llvm::OwningPtr<llvm::object::Binary> binary;
	llvm::OwningPtr<fail::LLVMDisassembler> disas;

	bool addRegisterTrace(fail::simtime_t curtime, instruction_count_t instr,
						  Trace_Event &ev,
						  const fail::LLVMtoFailTranslator::reginfo_t &info,
						  char access_type);

	fail::CommandLine::option_handle NO_GP, FLAGS, IP, NO_SPLIT;
	bool do_gp, do_flags, do_ip, do_split_registers;

	std::set<unsigned> m_register_ids;
	unsigned m_ip_register_id;

public:
	RegisterImporter();

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
		aliases->push_back("RegisterImporter");
		aliases->push_back("regs");
	}

};

#endif
