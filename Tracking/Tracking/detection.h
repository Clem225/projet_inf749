#pragma once

using namespace std;
using namespace cv;

void detection(HOGDescriptor &hog, Mat &frame, vector<Rect2d> &list_humans, MultiTracker &trackers);