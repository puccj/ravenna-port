#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

int main() {
  int thresholdValue = 125;
  int minAreaValue = 150;
  int maxAreaValue = 200;
  int minDistValue = 30;
  int minRadiusValue = 8;
  int maxRadiusValue = 29; //35


  cv::namedWindow("Trackbars");
  cv::createTrackbar("Threshold", "Trackbars", &thresholdValue, 200);
  cv::createTrackbar("Min area", "Trackbars", &minAreaValue, 200);
  cv::createTrackbar("Max area", "Trackbars", &maxAreaValue, 500);
  cv::createTrackbar("Min distance", "Trackbars", &minDistValue, 100);
  cv::setTrackbarMin("Min distance", "Trackbars", 1);
  cv::createTrackbar("Min radius", "Trackbars", &minRadiusValue, 50);
  cv::createTrackbar("Max radius", "Trackbars", &maxRadiusValue, 100);

  cv::Mat frame;
  cv::VideoCapture cap("sample.mp4");
  while (true) {
    cap.read(frame);
    cv::Mat threshFrame;
    cv::cvtColor(frame, threshFrame, cv::COLOR_BGR2GRAY);
    cv::medianBlur(threshFrame,threshFrame, 3);
    
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(threshFrame, circles, cv::HOUGH_GRADIENT, 1, minDistValue, 100, 30, minRadiusValue, maxRadiusValue);

    for (auto circle : circles) {
      cv::Point center(circle[0], circle[1]);
      cv::circle(frame, center, 1, {255,0,255}, 3, cv::LINE_AA);
      cv::circle(frame, center, circle[2], {100,100,100}, 1, cv::LINE_AA);
    }
    
    
    /*
    cv::threshold(threshFrame, threshFrame, thresholdValue, 255, cv::THRESH_BINARY);
    std::vector<cv::Mat> contours;
    cv::findContours(threshFrame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    //cv::drawContours(threshFrame, contours, -1, 1);

    for (auto contour : contours) {
      //skip if area too small or big
      int area = cv::contourArea(contour);
      if (area < minAreaValue || area > maxAreaValue)
        continue;
      
      //draw a circle
      cv::Rect r = cv::boundingRect(contour);
      //cv::Rect r(cv::Point{10,10},cv::Point{200,200});  
      cv::rectangle(frame, r, {0,255,0},2);
    }
    */
    
    cv::imshow("Thresh", threshFrame);
    cv::imshow("Detected", frame);

    int k = cv::waitKey(60);
    if (k == 'q')
      break;
  }

  return 0;
}