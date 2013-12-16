#ifndef FACETRACKERJS_H_
#define FACETRACKERJS_H_

#include <vector>
#include "opencv2/opencv.hpp"
#include "FaceTracker/Tracker.h"

class FaceTrackerJS {
 public:
  FaceTrackerJS();
  void setup();
  bool update(cv::Mat image);
  void updateObjectPoints();

  inline double *getImagePoints() const {
    return (double *)tracker._shape.data;
  }
  cv::Point2f getImagePoint(int i) const;
  cv::Point3f getObjectPoint(int i) const;
  float *getCalibratedObjectPoints();
  inline double *getEulerAngles() const {
    return (double *)tracker._clm._pglobl.data;
  }

 private:
  bool failed;
  int age;
  int currentView;
  
  bool fcheck;
  double rescale;
  int frameSkip;
  
  std::vector<int> wSize1, wSize2;
  int iterations;
  int attempts;
  double clamp, tolerance;
  bool useInvisible;
  
  FACETRACKER::Tracker tracker;
  cv::Mat tri, con;
  
  cv::Mat im, gray;
  cv::Mat objectPoints;
  cv::Mat calibratedObjectPoints;

  cv::Mat cameraMatrix;
  cv::Mat distCoeffs;
};

#endif  // FACETRACKERJS_H_
