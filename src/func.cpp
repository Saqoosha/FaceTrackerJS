#include <iostream>
#include "FaceTrackerJS.h"

// #define CONVERT_TO_GRAY

extern "C" {

  int hello() {
    std::cout << "Hello\n";
    return 123;
  }


  FaceTrackerJS *createTracker() {
    FaceTrackerJS *tracker = new FaceTrackerJS();
    tracker->setup();
    return tracker;
  }


  bool update(FaceTrackerJS *tracker, unsigned char *data, int width, int height) {
    cv::Mat original(height, width, CV_8UC4, data);
    cv::Mat gray(height, width, CV_8UC1);
#ifdef CONVERT_TO_GRAY
    cv::cvtColor(original, gray, CV_RGBA2GRAY);
#else
    cv::Mat out[] = { gray };
    int from_to[] = { 1, 0 };
    cv::mixChannels(&original, 1, out, 1, from_to, 1);
#endif
    bool result = tracker->update(gray);
    return result;
  }


  double *getImagePoints(FaceTrackerJS *tracker) {
    return tracker->getImagePoints();
  }


  float *getCalibratedObjectPoints(FaceTrackerJS *tracker) {
    return tracker->getCalibratedObjectPoints();
  }


  double *getEulerAngles(FaceTrackerJS *tracker) {
    return tracker->getEulerAngles();
  }

}
