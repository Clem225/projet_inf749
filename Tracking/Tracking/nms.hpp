#pragma once

#include <vector>
#include <numeric>

enum PointInRectangle { XMIN, YMIN, XMAX, YMAX };

std::vector<cv::Rect2d> nms(const std::vector<std::vector<double>> & boxes, const float & threshold);

std::vector<double> GetPointFromRect(const std::vector<std::vector<double>> &,
                                    const PointInRectangle &);

std::vector<double> ComputeArea(const std::vector<double> &,
                               const std::vector<double> &,
                               const std::vector<double> &,
                               const std::vector<double> &);

template <typename T>
std::vector<int> argsort(const std::vector<T> & v);

std::vector<double> Maximum(const double &,
                           const std::vector<double> &);

std::vector<double> Minimum(const double &,
                           const std::vector<double> &);

std::vector<double> CopyByIndexes(const std::vector<double> &,
                                 const std::vector<int> &);

std::vector<int> RemoveLast(const std::vector<int> &);

std::vector<double> Subtract(const std::vector<double> &,
                            const std::vector<double> &);

std::vector<double> Multiply(const std::vector<double> &,
                            const std::vector<double> &);

std::vector<double> Divide(const std::vector<double> &,
                          const std::vector<double> &);

std::vector<int> WhereLarger(const std::vector<double> &,
                             const double &);

std::vector<int> RemoveByIndexes(const std::vector<int> &,
                                 const std::vector<int> &);

std::vector<cv::Rect2d> BoxesToRectangles(const std::vector<std::vector<double>> &);

std::vector<std::vector<double>> RectanglesToBoxes(const std::vector<cv::Rect2d> & boxes);

template <typename T>
std::vector<T> FilterVector(const std::vector<T> &,
                            const std::vector<int> &);
