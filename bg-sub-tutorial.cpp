#include <iostream>
#include <sstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

int main(int argc, char* argv[])
{
  /*int thresholdValue = 400;
  cv::namedWindow("Trackbars", cv::WINDOW_AUTOSIZE);
  cv::createTrackbar("Threshold", "Trackbars", &thresholdValue,1000);
  */

  //create Background Subtractor objects
  //cv::Ptr<cv::BackgroundSubtractor> pBackSub = cv::createBackgroundSubtractorMOG2(500,2000);
  cv::Ptr<cv::BackgroundSubtractor> pBackSub = cv::createBackgroundSubtractorKNN(500, 10010, true);

  cv::VideoCapture capture("sample.mp4");
  
  cv::Mat frame, fgMask, thresh;
  while (true) {
    capture >> frame;
    if (frame.empty())
      break;
    
    pBackSub->apply(frame, fgMask);
    
    cv::inRange(fgMask,cv::Scalar(254,254,254),cv::Scalar(255,255,255),thresh); 
    //cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1,-1));
    //find the contours and draw them
    std::vector<cv::Mat> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::drawContours(frame, contours, -1, {255,0,0}, 1);
    for (auto contour : contours) {
      //skip if the area is too little
      if (cv::contourArea(contour) < 700)
        continue;

      cv::Rect r = cv::boundingRect(contour);     //create a rectangle
      cv::rectangle(frame, r, {0,255,0}, 2);  //draw the rectangle
    }

    //show the current frame and the fg masks
    cv::imshow("Frame", frame);
    cv::imshow("FG Mask", fgMask);
    cv::imshow("Final", thresh);

    //get the input from the keyboard
    int keyboard = cv::waitKey(30);
    if (keyboard == 'q')
      break;
  }
  return 0;
}