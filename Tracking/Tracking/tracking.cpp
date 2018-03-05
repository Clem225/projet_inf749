#include "stdafx.h"

using namespace std;
using namespace cv;

Ptr<Tracker> createTracker(string name)
{
	Ptr<Tracker> tracker;//lol

	if (name == "KCF")
		tracker = cv::TrackerKCF::create();
	else if (name == "TLD")
		tracker = cv::TrackerTLD::create();
	else if (name == "BOOSTING")
		tracker = cv::TrackerBoosting::create();
	else if (name == "MEDIAN_FLOW")
		tracker = cv::TrackerMedianFlow::create();
	else if (name == "MIL")
		tracker = cv::TrackerMIL::create();
	else if (name == "GOTURN")
		tracker = cv::TrackerGOTURN::create();
	else if (name == "MOSSE")
		tracker = cv::TrackerMOSSE::create();
	else if (name == "CSRT")
		tracker = cv::TrackerCSRT::create();
	else
		CV_Error(Error::StsBadArg, "Nom de l'algorithme inconnu\n");

	return tracker;
}

void tracking(MultiTracker &trackers, std::vector< Ptr<Tracker> > &list_trackers, string &trackerAlgo, vector<Rect2d> &list_humans, Mat &frame)
{
	list_trackers.clear();
	for (size_t i = 0; i < list_humans.size(); i++)
	{
		list_trackers.push_back(createTracker(trackerAlgo));
	}

	trackers.add(list_trackers, frame, list_humans);
}