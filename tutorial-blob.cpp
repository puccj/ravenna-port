#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/core/cvstd.hpp>

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
   
  int minThreshold = 0;
  int maxThreshold = 700;
  int minArea = 100;
  int maxArea = 500;
  int minCircularity = 10;  // :100
  int minConvexity = 38 ;    // :100
  int minInertiaRatio = 1;  // :100
  params.filterByArea = true;         //Filter by area
  params.filterByCircularity = true;  //Filter by Circularity
  params.filterByConvexity = true;    //Filter by Convexity
  params.filterByInertia = true;      //Filter by Inertia

  cv::namedWindow("Trackbars");
  cv::createTrackbar("Min threshold", "Trackbars", &minThreshold, 300);
  cv::createTrackbar("Max threshold", "Trackbars", &maxThreshold, 10000);
  cv::createTrackbar("Min Area", "Trackbars", &minArea, 1000, onAreaChanged);
  cv::createTrackbar("Max Area", "Trackbars", &maxArea, 1000);
  cv::createTrackbar("Min Circularity (x100)", "Trackbars", &minCircularity, 100, onCircularityChanged);
  cv::createTrackbar("Min Convexity (x100)", "Trackbars", &minConvexity, 100, onConvexityChanged);
  cv::createTrackbar("Min Inertia (x100)", "Trackbars", &minInertiaRatio, 100, onInertiaChanged);

  while (true) {
    params.minThreshold = minThreshold;
    params.maxThreshold = maxThreshold;
    params.minArea = minArea;
    params.minCircularity = minCircularity/100.0;
    params.minConvexity = minConvexity/100.0;
    params.minInertiaRatio = minInertiaRatio/100.0;
    params.maxArea = maxArea;
  
    cap.read(frame);

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