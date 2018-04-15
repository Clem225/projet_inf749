#pragma once

using namespace std;
using namespace cv;

void detection(HOGDescriptor &hog, Mat &frame, vector<Rect2d> &list_humans);
void detection2(CascadeClassifier &classif, Mat &frame, vector<Rect2d> &detect_list, double scaleFactor, vector<double>& levelWeights, vector<int>& rejectLevels, cv::Size min = cv::Size(), cv::Size max = cv::Size());
void nms(const std::vector<cv::Rect2d>& srcDetect, const std::vector<cv::Rect2d>& srcTrack, std::vector<cv::Rect2d>& resRects, float thresh, int neighbors = 0);
