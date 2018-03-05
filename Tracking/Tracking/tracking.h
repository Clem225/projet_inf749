#pragma once

using namespace std;
using namespace cv;

Ptr<Tracker> createTracker(string name);
void tracking(MultiTracker &trackers, std::vector< Ptr<Tracker> > &list_trackers, string &trackerAlgo, vector<Rect2d> &list_humans, Mat &frame);