#include <iostream>
#include <fstream>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/Logger.hpp"
#include "util/ProtoStream.hpp"
#include "sal/SALConfig.hpp"


using namespace std;
using namespace fail;
using namespace google::protobuf;

void VEZSCampaign::cb_send_pilot(DatabaseCampaignMessage pilot) {
	VEZSExperimentData *data = new VEZSExperimentData;
	data->msg.mutable_fsppilot()->CopyFrom(pilot);
	campaignmanager.addParam(data);
}
