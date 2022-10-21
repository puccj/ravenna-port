#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/imgproc.hpp>

#include <random>
#include <iostream>

cv::SimpleBlobDetector::Params params;

static void onAreaChanged(int value, void*) {
  if (value == 0)
    params.filterByArea = false;
  else
    params.filterByArea = true;
}
static void onCircularityChanged(int value, void*) {
  if (value == 0)
    params.filterByCircularity = false;
  else
    params.filterByCircularity = true;
}
static void onConvexityChanged(int value, void*) {
  if (value == 0)
    params.filterByConvexity = false;
  else
    params.filterByConvexity = true;
}
static void onInertiaChanged(int value, void*) {
  if (value == 0)
    params.filterByInertia = false;
  else
    params.filterByInertia = true;
}

int main() {
  cv::Mat frame;
  cv::VideoCapture cap("sample.mp4", cv::IMREAD_GRAYSCALE);

  /* Default values 
  thresholdStep = 10;
  minThreshold = 50;
  maxThreshold = 220;
  minRepeatability = 2;
  minDistBetweenBlobs = 10;*/

  int minThreshold = 0;
  int maxThreshold = 1000;
  int minArea = 175;
  int maxArea = 202;
  int minCircularity = 0;  // :100
  int minConvexity = 0 ;    // :100
  int minInertiaRatio = 0;  // :100
  int minDistBetweenBlobs = 0;
  int minRepeatability = 1;
  params.filterByArea = false;         //Filter by area
  params.filterByCircularity = false;  //Filter by Circularity
  params.filterByConvexity = false;    //Filter by Convexity
  params.filterByInertia = false;      //Filter by Inertia

  cv::namedWindow("Trackbars", cv::WINDOW_AUTOSIZE);
  cv::createTrackbar("Min threshold", "Trackbars", &minThreshold, 300);
  cv::createTrackbar("Max threshold", "Trackbars", &maxThreshold, 10000);
  cv::createTrackbar("Min Area", "Trackbars", &minArea, 250, onAreaChanged);
  cv::createTrackbar("Max Area", "Trackbars", &maxArea, 250);
  cv::createTrackbar("Min Circularity (x100)", "Trackbars", &minCircularity, 100, onCircularityChanged);
  cv::createTrackbar("Min Convexity (x100)", "Trackbars", &minConvexity, 100, onConvexityChanged);
  cv::createTrackbar("Min Inertia (x100)", "Trackbars", &minInertiaRatio, 100, onInertiaChanged);
  cv::createTrackbar("Min Distance", "Trackbars", &minDistBetweenBlobs, 100);
  cv::createTrackbar("Min Repeatability", "Trackbars", &minRepeatability, 100);
  cv::setTrackbarMin("Min Repeatability", "Trackbars", 1);

  while (true) {
    params.minThreshold = minThreshold;
    params.maxThreshold = maxThreshold;
    params.minArea = minArea;
    params.minCircularity = minCircularity/100.0;
    params.minConvexity = minConvexity/100.0;
    params.minInertiaRatio = minInertiaRatio/100.0;
    params.maxArea = maxArea;
    params.minDistBetweenBlobs = minDistBetweenBlobs;
    params.minRepeatability = minRepeatability;
  
    cap.read(frame);
    cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    cv::medianBlur(frame, frame, 5);

    cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);  //Set up the detector with default parameters
    std::vector<cv::KeyPoint> keypoints;  //Detect blobs

    detector->detect(frame, keypoints);
    
    //Draw detected blobs as red circles.
    //DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
    cv::Mat im_with_keypoints;
    cv::drawKeypoints(frame, keypoints, im_with_keypoints, {0,0,255}, cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

    //Show blobs
    cv::imshow("keypoints", im_with_keypoints );
    int k = cv::waitKey(60);
    if (k == 'q')
      break;
  }

  return 0;
}