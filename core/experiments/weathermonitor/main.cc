#include <iostream>
#include <cstdlib>

#include "controller/CampaignManager.hpp"
#include "experiments/weathermonitor/campaign.hpp"

int main(int argc, char **argv)
{
	WeathermonitorCampaign c;
	return !fi::campaignmanager.runCampaign(&c);
}
