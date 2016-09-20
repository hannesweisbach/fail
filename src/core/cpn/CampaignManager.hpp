/**
 * \file CampaignManager.hpp
 * \brief The manager for an entire campaign
 */

#ifndef __CAMPAIGN_MANAGER_HPP__
#define __CAMPAIGN_MANAGER_HPP__

#include "sal/SALInst.hpp"
#include "comm/ExperimentData.hpp"
#include "JobServer.hpp"
#include "Campaign.hpp"
#include "util/CommandLine.hpp"
#include "util/Logger.hpp"

namespace fail {

static Logger log_send("CampaignManager");

/**
 * \class CampaignManager
 *
 * The CampaignManager allows a user-campaign access to all constant
 * simulator information and forwards single experiments to the JobServer.
 */
class CampaignManager {
private:
	JobServer *m_jobserver;
	Campaign* m_currentCampaign;
	CommandLine::option_handle port;
public:
	CampaignManager() : m_jobserver(0), m_currentCampaign(0)
	{
		fail::CommandLine &cmd = fail::CommandLine::Inst();
		port = cmd.addOption("p", "port", Arg::Required,
				     "-p,--port \tListening port of server; no "
				     "value chooses port automatically");
	}
	~CampaignManager() { delete m_jobserver; }
	/**
	 * Executes a user campaign
	 */
	bool runCampaign(Campaign* c)
	{
		fail::CommandLine &cmd = fail::CommandLine::Inst();
		if (!cmd.parse()) {
			log_send << "Error parsing arguments." << std::endl;
			exit(-1);
		}

		if (!m_jobserver) {
			m_jobserver =
			    (cmd[port].count() > 0)
				? new JobServer(cmd[port].first()->arg)
				: new JobServer;
		}
		m_currentCampaign = c;
		bool ret = c->run();
		m_jobserver->done();
		return ret;
	}
	/**
	 * Returns a const reference for acquiring constant simulator specific information.
	 * e.g., Registernames, to ease experiment data construction.
	 * The campaign description is not allowed to change the simulator
	 * state, as the actual simulation runs within another process (Minion)
	 * @return constant reference to the current simulator backend.
	 */
	SimulatorController const& getSimulator() const { return simulator; }
	/**
	 * Add a experiment parameter set.
	 * The user campaign has to allocate the Parameter object,
	 * and deallocate it after result reception.
	 * A Parameter set includes space for results.
	 * @param exp A pointer to a ExperimentData set.
	 */
	void addParam(ExperimentData* exp) { m_jobserver->addParam(exp); }
	/**
	 * A user campaign can request a single result (blocking) from the queue.
	 * @return Pointer to a parameter object with filled result data
	 * @see addParam()
	 */
	ExperimentData* getDone() { return m_jobserver->getDone(); }
	/**
	 * Signal, that there will not come any further parameter sets.
	 */
	void noMoreParameters() { m_jobserver->setNoMoreExperiments(); }
	 /**
	 * User campaign has finished.
	 */
	void done() { m_jobserver->done(); }
	/**
	 * Wait actively, until all experiments expired.
	 */
//	void waitForCompletion();
};

extern CampaignManager campaignmanager;

} // end-of-namespace: fail

#endif // __CAMPAIGN_MANAGER_HPP__
