#pragma once

using namespace std;
using namespace cv;

void detection(HOGDescriptor &hog, Mat &frame, vector<Rect2d> &list_humans);
//void detection2(HOGDescriptor &hog, Mat &frame, vector<Rect2d> &list_humans);
void detection2(CascadeClassifier &classif, Mat &frame, vector<Rect2d> &detect_list, double scaleFactor, cv::Size min = cv::Size(), cv::Size max = cv::Size());
