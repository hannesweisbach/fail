// <iostream> needs to be included before *.pb.h, otherwise ac++/Puma chokes on the latter
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <err.h>

#include "comm/SocketComm.hpp"
#include "JobServer.hpp"
#include "Minion.hpp"

#ifndef __puma
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#endif

using namespace std;

namespace fail {

void JobServer::addParam(ExperimentData* exp)
{
#ifndef __puma
	m_inOutCounter.increment();
	m_undoneJobs.Enqueue(exp);
#endif
}

#ifdef SERVER_PERFORMANCE_MEASURE
volatile unsigned JobServer::m_DoneCount = 0;
#endif

ExperimentData *JobServer::getDone()
{
#ifndef __puma
	ExperimentData *exp = m_doneJobs.Dequeue();
	if (exp) {
		m_inOutCounter.decrement();
	}
	return exp;
#endif
}

void JobServer::setNoMoreExperiments()
{
#ifndef __puma
	boost::unique_lock<boost::mutex> lock(m_CommMutex);
#endif
	// currently not really necessary, as we only non-blockingly dequeue:
	m_undoneJobs.setIsFinished();

	m_noMoreExps = true;
	if (m_undoneJobs.Size() == 0 &&
	    noMoreExperiments() &&
	    m_runningJobs.Size() == 0) {
		m_doneJobs.setIsFinished();
	}
}

#ifdef SERVER_PERFORMANCE_MEASURE
void JobServer::measure()
{
	// TODO: Log-level?
	cout << "\n[Server] Logging throughput in \"" << SERVER_PERF_LOG_PATH << "\"..." << endl;
	ofstream m_file(SERVER_PERF_LOG_PATH, std::ios::trunc); // overwrite existing perf-logs
	if (!m_file.is_open()) {
		cerr << "[Server] Perf-logging has been enabled"
		     << "but I was not able to write the log-file \""
		     << SERVER_PERF_LOG_PATH << "\"." << endl;
		exit(1);
	}
	unsigned counter = 0;

	m_file << "time\tthroughput" << endl;
	unsigned diff = 0;
	while (!m_finish) {
		// Format: 1st column (seconds)[TAB]2nd column (throughput)
		m_file << counter << "\t" << (m_DoneCount - diff) << endl;
		counter += SERVER_PERF_STEPPING_SEC;
		diff = m_DoneCount;
		sleep(SERVER_PERF_STEPPING_SEC);
	}
	// NOTE: Summing up the values written in the 2nd column does not
	// necessarily yield the number of completed experiments/jobs
	// (due to thread-scheduling behaviour -> not sync'd!)
}
#endif // SERVER_PERFORMANCE_MEASURE

#ifndef __puma
/**
 * This is a predicate class for the remove_if operator on the thread
 * list. The operator waits for timeout milliseconds to join each
 * thread in the list. If the join was successful, the exited thread
 * is deallocated and removed from the list.
 */
struct timed_join_successful {
	int timeout_ms;
	timed_join_successful(int timeout)
		: timeout_ms(timeout) { }

	bool operator()(boost::thread* threadelement)
	{
		boost::posix_time::time_duration timeout = boost::posix_time::milliseconds(timeout_ms);
		if (threadelement->timed_join(timeout)) {
			delete threadelement;
			return true;
		} else {
			return false;
		}
	}
};
#endif

void JobServer::run()
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *ai;
	int err = getaddrinfo(NULL, m_port, &hints, &ai);
	if (err != 0) {
		errx(err, "getaddrinfo: %s\n", gai_strerror(err));
	}

	int s;
	struct addrinfo *i;
	for (i = ai; i; i = i->ai_next) {
		s = socket(i->ai_family, i->ai_socktype, i->ai_protocol);
		if (s == -1) {
			continue;
		}

		if (::bind(s, i->ai_addr, i->ai_addrlen) == 0) {
			int on = 1;
			if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on,
				       sizeof(on)) == -1) {
				perror("setsockopt");
			}
			break;
		}
	}

	if (i == NULL) {
		std::cerr << "Could not bind" << std::endl;
		return;
	}

	/* Listen with a backlog of maxThreads */
	if (listen(s, m_maxThreads) == -1) {
		perror("listen");
		close(s);
		// TODO: Log-level?
		return;
	}
	cout << "JobServer listening on port " << m_port << " ..." << endl;
	// TODO: Log-level?
#ifndef __puma
	boost::thread* th;
	while (!m_finish) {
		// Accept connection
		struct sockaddr_in clientaddr;
		socklen_t clen = sizeof(clientaddr);
		int cs = SocketComm::timedAccept(s, (struct sockaddr*)&clientaddr, &clen, 100);
		if (cs < 0) {
			if (errno != EWOULDBLOCK) {
				perror("poll/accept");
				close(s);
				// TODO: Log-level?
				return;
			} else {
				continue;
			}
		}

		bool creation_failed = false;
		do {
			// Spawn a thread for further communication,
			// and add this thread to a list threads
			// We can limit the generation of threads here.
			if (m_threadlist.size() >= m_maxThreads || creation_failed) {
				// Run over list with a timed_join,
				// removing finished threads.
				do {
					m_threadlist.remove_if(timed_join_successful(m_threadtimeout));
				} while (m_threadlist.size() >= m_maxThreads);
			}
			// Start new thread
			try {
				th = new boost::thread(CommThread(cs, *this));
				creation_failed = false;
			} catch (boost::thread_resource_error e) {
				cout << "failed to spawn thread, throttling ..." << endl;
				creation_failed = true;
				sleep(1);
			}
		} while (creation_failed);

		m_threadlist.push_back(th);
	}
	close(s);
	// when all undone Jobs are distributed  -> call a timed_join on all spawned
	// TODO: interrupt threads that do not want to join..
	while (m_threadlist.size() > 0)
		m_threadlist.remove_if( timed_join_successful(m_threadtimeout) );
#endif
}

void CommThread::operator()()
{
	// The communication thread implementation:

	Minion minion;
	FailControlMessage ctrlmsg;
	minion.setSocketDescriptor(m_sock);

	if (!SocketComm::rcvMsg(minion.getSocketDescriptor(), ctrlmsg)) {
		cout << "!![Server] failed to read complete message from client" << endl;
		close(m_sock);
		return;
	}

	switch (ctrlmsg.command()) {
	case FailControlMessage::NEED_WORK:
		// let old clients die (run_id == 0 -> possibly virgin client)
		if (!ctrlmsg.has_run_id() || (ctrlmsg.run_id() != 0 && ctrlmsg.run_id() != m_js.m_runid)) {
			cout << "!![Server] telling old client to die" << endl;
			ctrlmsg.Clear();
			ctrlmsg.set_command(FailControlMessage::DIE);
			ctrlmsg.set_build_id(42);
			SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg);
			break;
		}
		// give minion something to do..
		m_job_size = ctrlmsg.job_size();
		sendPendingExperimentData(minion);
		break;
	case FailControlMessage::RESULT_FOLLOWS:
		// ignore old client's results
		if (!ctrlmsg.has_run_id() || ctrlmsg.run_id() != m_js.m_runid) {
			cout << "!![Server] ignoring old client's results" << endl;
			break;
		}
		// get results and put to done queue.
		receiveExperimentResults(minion, ctrlmsg);
		break;
	default:
		// hm.. don't know what to do. please die.
		cout << "!![Server] no idea what to do with command #"
		     << ctrlmsg.command() << ", telling minion to die." << endl;
		ctrlmsg.Clear();
		ctrlmsg.set_command(FailControlMessage::DIE);
		ctrlmsg.set_build_id(42);
		SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg);
	}

	close(m_sock);
}

void CommThread::sendPendingExperimentData(Minion& minion)
{
	uint32_t i;
	uint32_t workloadID;
	std::deque<ExperimentData*> exp;
	ExperimentData* temp_exp = 0;
	FailControlMessage ctrlmsg;

	ctrlmsg.set_build_id(42);
	ctrlmsg.set_run_id(m_js.m_runid);
	ctrlmsg.set_command(FailControlMessage::WORK_FOLLOWS);

	for (i = 0; i < m_job_size; i++) {
		if (m_js.m_undoneJobs.Dequeue_nb(temp_exp) == true) {
			// Got an element from queue, assign ID to workload and send to minion
			workloadID = m_js.m_counter.increment(); // increment workload counter
			temp_exp->setWorkloadID(workloadID); // store ID for identification when receiving result
			ctrlmsg.add_workloadid(workloadID);
			exp.push_back(temp_exp);
		} else {
			break;
		}
	}
	if (exp.size() != 0) {
		ctrlmsg.set_job_size(exp.size());

		cout << " >>[" << ctrlmsg.workloadid(0) << "+"
			<< exp.size() << "]      \r" << flush;

		if (SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg)) {
			for (i = 0; i < ctrlmsg.job_size(); i++) {
				if (SocketComm::sendMsg(minion.getSocketDescriptor(), exp.front()->getMessage())) {

					// delay insertion into m_runningJobs until here, as
					// getMessage() won't work anymore if this job is re-sent,
					// received, and deleted in the meantime
					if (!m_js.m_runningJobs.insert(exp.front()->getWorkloadID(), exp.front())) {
						cout << "!![Server]could not insert workload id: [" << workloadID << "] double entry?" << endl;
					}

					exp.pop_front();
				} else {
					// add remaining jobs back to the queue
					cout << "!![Server] failed to send scheduled " << exp.size() << " jobs" << endl;
					while (exp.size()) {
						m_js.m_undoneJobs.Enqueue(exp.front());
						exp.pop_front();
					}
					break;
				}

			}
		}
		return;
	}

#ifndef __puma
	// Prevent receiveExperimentResults from modifying (or indirectly, via
	// getDone and the campaign, deleting) jobs in the m_runningJobs queue.
	// (See details in receiveExperimentResults)
	boost::unique_lock<boost::mutex> lock(m_js.m_CommMutex);
#endif
	if ((temp_exp = m_js.m_runningJobs.pickone()) != NULL) { // 2nd priority
		// (This picks one running job.)
		// TODO: Improve selection of parameter set to be resent:
		//  -  currently: Linear complexity!
		//  -  pick entry at random?
		//  -  retry counter for each job?

		// Implement resend of running-parameter sets to improve campaign speed
		// and to prevent result loss due to (unexpected) termination of experiment
		// clients.
		// (Note: Therefore we need to be aware of receiving multiple results for a
		//        single parameter-set, @see receiveExperimentResults.)
		uint32_t workloadID = temp_exp->getWorkloadID(); // (this ID has been set previously)
		// Resend the parameter-set.
		ctrlmsg.set_command(FailControlMessage::WORK_FOLLOWS);
		ctrlmsg.add_workloadid(workloadID); // set workload id
		ctrlmsg.set_job_size(1); // In 2nd priority the jobserver send only one job
		//cout << ">>[Server] Re-sending workload [" << workloadID << "]" << endl;
		cout << ">>R[" << workloadID << "]        \r" << flush;
		if (SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg)) {
			SocketComm::sendMsg(minion.getSocketDescriptor(), temp_exp->getMessage());
		}
	} else if (m_js.noMoreExperiments() == false) {
		// Currently we have no workload (even the running-job-queue is empty!), but
		// the campaign is not over yet. Minion can try again later.
		ctrlmsg.set_command(FailControlMessage::COME_AGAIN);
		SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg);
		cout << "--[Server] No workload, come again..."  <<  endl;
	} else {
		// No more elements, and campaign is over. Minion can die.
		ctrlmsg.set_command(FailControlMessage::DIE);
		cout << "--[Server] No workload, and no campaign, please die." << endl;
		SocketComm::sendMsg(minion.getSocketDescriptor(), ctrlmsg);
	}
}

void CommThread::receiveExperimentResults(Minion& minion, FailControlMessage& ctrlmsg)
{
	int i;
	ExperimentData* exp = NULL; // Get exp* from running jobs
	if (ctrlmsg.workloadid_size() > 0) {
		cout << " <<[" << ctrlmsg.workloadid(0) << "+"
		     << ctrlmsg.workloadid_size() << "]        \r" << flush;
	}
#ifndef __puma
	// Prevent re-sending jobs in sendPendingExperimentData:
	// a) sendPendingExperimentData needs an intact job to serialize and send it.
	// b) After moving the job to m_doneJobs, it may be retrieved and deleted
	//    by the campaign at any time.
	// Additionally, receiving a result overwrites the job's contents.  This
	// already may cause breakage in sendPendingExperimentData (a).
	boost::unique_lock<boost::mutex> lock(m_js.m_CommMutex);
#endif
	for (i = 0; i < ctrlmsg.workloadid_size(); i++) {
		if (m_js.m_runningJobs.remove(ctrlmsg.workloadid(i), exp)) { // ExperimentData* found
			// deserialize results, expect failures
			if (!SocketComm::rcvMsg(minion.getSocketDescriptor(), exp->getMessage())) {
				m_js.m_runningJobs.insert(ctrlmsg.workloadid(i), exp);
			} else {
				m_js.m_doneJobs.Enqueue(exp); // Put results in done queue
			}
		  #ifdef SERVER_PERFORMANCE_MEASURE
			++JobServer::m_DoneCount;
		  #endif
		} else {
			// We can receive several results for the same workload id because
			// we (may) distribute the (running) jobs to a *few* experiment-clients.
			cout << "[Server] Received another result for workload id ["
				 << ctrlmsg.workloadid(i) << "] -- ignored." << endl;

			SocketComm::dropMsg(minion.getSocketDescriptor());
		}
	}

	// all results complete?
	if (m_js.m_undoneJobs.Size() == 0 &&
	    m_js.noMoreExperiments() &&
	    m_js.m_runningJobs.Size() == 0) {
		m_js.m_doneJobs.setIsFinished();
	}
}

} // end-of-namespace: fail
