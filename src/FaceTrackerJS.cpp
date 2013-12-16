#include "FaceTrackerJS.h"

#define it at<int>
#define db at<double>


// (nearest point on a ray) to the given point
template <class T>
cv::Point3_<T> intersectPointRay(cv::Point3_<T> point, cv::Point3_<T> ray) {
  return ray * (point.dot(ray) / ray.dot(ray));
}


FaceTrackerJS::FaceTrackerJS()
    : rescale(1),
      iterations(10), // [1-25] 1 is fast and inaccurate, 25 is slow and accurate
      clamp(3),       // [0-4] 1 gives a very loose fit, 4 gives a very tight fit
      tolerance(.01), // [.01-1] match tolerance
      attempts(1),    // [1-4] 1 is fast and may not find faces, 4 is slow but will find faces
      failed(true),
      fcheck(true),   // check for whether the tracking failed
      frameSkip(-1),  // how often to skip frames
      useInvisible(true),
      age(-1) {
}


void FaceTrackerJS::setup() {
  wSize1.resize(1);
  wSize1[0] = 7;
  
  wSize2.resize(3);
  wSize2[0] = 11;
  wSize2[1] = 9;
  wSize2[2] = 7;

  tracker.Load("data/face2.tracker");
  // tri = FACETRACKER::IO::LoadTri("libs/FaceTracker/model/face.tri");
  // con = FACETRACKER::IO::LoadCon("libs/FaceTracker/model/face.con");

  cv::FileStorage fs("data/mbp-isight.yml", cv::FileStorage::READ);
  cv::Size imageSize, sensorSize;
  fs["cameraMatrix"] >> cameraMatrix;
  fs["imageSize_width"] >> imageSize.width;
  fs["imageSize_height"] >> imageSize.height;
  fs["sensorSize_width"] >> sensorSize.width;
  fs["sensorSize_height"] >> sensorSize.height;
  fs["distCoeffs"] >> distCoeffs;
  // std::cout << "cameraMatrix: " << cameraMatrix << std::endl;
  // std::cout << "distCoeffs: " << distCoeffs << std::endl;
}


bool FaceTrackerJS::update(cv::Mat gray) {
  bool tryAgain = true;
  for (int i = 0; tryAgain && i < attempts; i++) {
    if (tracker.Track(gray, failed ? wSize2 : wSize1, frameSkip, iterations, clamp, tolerance, fcheck) == 0) {
      currentView = tracker._clm.GetViewIdx();
      failed = false;
      tryAgain = false;
      updateObjectPoints();
    } else {
      tracker.FrameReset();
      failed = true;
    }
  }
  return !failed;
}


void FaceTrackerJS::updateObjectPoints() {
  const cv::Mat &mean = tracker._clm._pdm._M;
  const cv::Mat &variation = tracker._clm._pdm._V;
  const cv::Mat &weights = tracker._clm._plocal;
  objectPoints = mean + variation * weights;
}


cv::Point2f FaceTrackerJS::getImagePoint(int i) const {
  if (failed) {
    return cv::Point2f();
  }
  const cv::Mat &shape = tracker._shape;
  int n = shape.rows / 2;
  return cv::Point2f(shape.db(i, 0), shape.db(i + n, 0));
}


cv::Point3f FaceTrackerJS::getObjectPoint(int i) const {
  if (failed) {
    return cv::Point3f();
  } 
  int n = objectPoints.rows / 3;
  return cv::Point3f(objectPoints.db(i, 0), objectPoints.db(i + n, 0), objectPoints.db(i + n + n, 0));
}


float *FaceTrackerJS::getCalibratedObjectPoints() {
  // 1 load object and image points as Point2f/3f
  std::vector<cv::Point3f> objectPoints;
  std::vector<cv::Point2f> imagePoints;
  int size = tracker._shape.rows / 2;
  for(int i = 0; i < size; i++) {
    objectPoints.push_back(getObjectPoint(i));
    imagePoints.push_back(getImagePoint(i));
  }
  
  // 2 guess for the rotation and translation of the face
  // cv::Mat cameraMatrix = calibration.getDistortedIntrinsics().getCameraMatrix();
  // cv::Mat distCoeffs = calibration.getDistCoeffs();
  cv::Mat rvec, tvec;
  cv::solvePnP(cv::Mat(objectPoints), cv::Mat(imagePoints), cameraMatrix,  distCoeffs, rvec, tvec);
  if (tvec.at<double>(2, 0) < 0) {
    tvec.at<double>(2, 0) *= -1.;
  //   modelMatrix = makeMatrix(rvec, tvec);
  //   modelMatrix.scale(-1, -1, 1);
  // } else {
  //   modelMatrix = makeMatrix(rvec, tvec);
  }
  
  // 3 reproject using guess, and fit to the actual image location
  std::vector<cv::Point3f> fitWorldPoints;
  cv::Mat cameraMatrixInv = cameraMatrix.inv();
  cv::Mat rmat;
  cv::Rodrigues(rvec, rmat);
  for (int i = 0; i < objectPoints.size(); i++) {
    cv::Point2d imgImg = imagePoints[i];
    cv::Point3d objObj = objectPoints[i];
    cv::Point3d imgHom(imgImg.x, imgImg.y, 1.); // img->hom
    cv::Point3d imgWor = (cv::Point3f)cv::Mat(cameraMatrixInv * cv::Mat(imgHom)); // hom->wor
    cv::Point3d objWor = (cv::Point3d)cv::Mat(tvec + rmat * cv::Mat(objObj)); // obj->wor
    cv::Point3d fitWor = intersectPointRay(objWor, imgWor); // scoot it over
    fitWorldPoints.push_back(fitWor);
    // std::cout << i << ": " << fitWor << std::endl;
  }
  calibratedObjectPoints = cv::Mat(fitWorldPoints, true);
  // std::cout << "calibratedObjectPoints: " << calibratedObjectPoints.rows << ", " << calibratedObjectPoints.cols << ", " << calibratedObjectPoints.isContinuous() << std::endl;
  // float *p = (float*)calibratedObjectPoints.data;
  // for (int i = 0; i < 9; i++)
  //   std::cout << "i:: " << p[i] << std::endl;
  return (float*)calibratedObjectPoints.data;
}


