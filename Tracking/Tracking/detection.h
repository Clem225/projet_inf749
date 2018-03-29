#pragma once

using namespace std;
using namespace cv;

void detection(HOGDescriptor &hog, Mat &frame, vector<Rect2d> &list_humans);
//void detection2(HOGDescriptor &hog, Mat &frame, vector<Rect2d> &list_humans);
void detection2(CascadeClassifier &classif, Mat &frame, vector<Rect2d> &detect_list, double scaleFactor, vector<double>& levelWeights, vector<int>& rejectLevels, cv::Size min = cv::Size(), cv::Size max = cv::Size());
bool est_superpose(const cv::Rect2d &r1, const cv::Rect2d &r2);
cv::Rect2d choix_rectangle(vector<cv::Rect2d> rec_superposes, vector<double> scores);