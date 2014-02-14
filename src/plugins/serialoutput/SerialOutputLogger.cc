#include "SerialOutputLogger.hpp"
#include "sal/Listener.hpp"

using namespace std;
using namespace fail;

bool SerialOutputLogger::run()
{
	IOPortListener ev_ioport(m_port, m_out);
	while (true) {
		simulator.addListener(&ev_ioport);
		simulator.resume();
		m_output += ev_ioport.getData();
	}
	return true;
}

void SerialOutputLogger::resetOutput()
{
	m_output.clear();
}

string SerialOutputLogger::getOutput()
{
	return m_output;
}