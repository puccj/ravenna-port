#include "cam.h"

#include <iostream>
#include <opencv2/imgproc.hpp>  //for cvtColor (convertion to B&W)

int Cam::_framesForBG = 50;
int Cam::_thresholdValue = 50;  
int Cam::_dilationValue = 2;   
int Cam::_minAreaValue = 500;  
int Cam::_consecutiveValue = 2;
int Cam::_globalFrameCount = 0;

void Cam::consecutiveChanged(int value, void*) {
  _globalFrameCount = 0;
  
  if (value == 0) {
    _consecutiveValue = 1;
    return;
  }
}

//    --Public--

void Cam::open(int camIndex) {
  _camIndex = camIndex;
  _cap.open(camIndex);
}

void Cam::calculateBackground() {
  std::cout << "Getting Background... ";
  cv::Mat frames[_framesForBG];  //stored frames to calculate the bkgnd with  

  for (int i = 0; i < _framesForBG; i++) {
    _cap.read(frames[i]);
    cv::cvtColor(frames[i], frames[i], cv::COLOR_BGR2GRAY);
  }

  int rows = frames[0].rows;
  int cols = frames[0].cols;
  
  cv::Mat background(rows, cols, CV_8UC1);  //store the background image

  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < rows; y++) {
      //create an array with the (x,y) pixel of all frames
      uchar currentPixel[_framesForBG];

      for (int i = 0; i < _framesForBG; i++) {
        //insert sort: pos is the position where the element will be inserted
        int pos = i-1;
        while(pos>=0 && frames[i].at<uchar>(y,x) < currentPixel[pos]) {
          currentPixel[pos+1] = currentPixel[pos];
          pos--;
        }
        currentPixel[pos+1] = frames[i].at<uchar>(y,x);
      }
      //now currentPixel is a sorted array with (x,y) pixel by all frames.
      //gets the median value and write it to the (x,y) pixel in background image
      background.at<uchar>(y,x) = currentPixel[_framesForBG/2];
    }
  }  
  _background = background;
  std::cout << "     Done\n";
}

void Cam::showBackground(int delay, std::string winName) {
  cv::imshow(winName, _background);
  cv::waitKey(delay);
}

bool Cam::show(std::string winName, bool showRectangle, bool showBorders, int delay) {
  if (winName == "default") {
    winName = "Camera " + std::to_string(_camIndex);
  }

  //read a frame
  cv::Mat thisFrame;
  _cap.read(thisFrame);
  
  //calculate everything only if it's needed
  if (showRectangle || showBorders) {
    //save the original frame
    cv::Mat origFrame;
    thisFrame.copyTo(origFrame);

    cv::cvtColor(thisFrame, thisFrame, cv::COLOR_BGR2GRAY);                       //convert to grayscale
    cv::absdiff(thisFrame, _background, thisFrame);                               //subtract background from image
    cv::threshold(thisFrame, thisFrame, _thresholdValue, 255, cv::THRESH_BINARY); //thresholding to convert to binary
    cv::dilate(thisFrame,thisFrame, cv::Mat(), cv::Point(-1,-1), _dilationValue); //dilate the image (no inside dark regions)

    if (_globalFrameCount < _frameCount)
      _frameCount = 0;
    
    _lastFrames[_frameCount % _consecutiveValue] = thisFrame;
    _frameCount++;
    _globalFrameCount++;

    //skip if list is not already full
    if (_frameCount >= _consecutiveValue) {

      //add all frames in list
      cv::Mat sum = _lastFrames[0];
      for (int i = 1; i < _consecutiveValue; i++)
        sum += _lastFrames[i];

      //find the contours and draw them
      std::vector<cv::Mat> contours;
      cv::findContours(sum, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
      cv::drawContours(origFrame, contours, -1, {255,0,0}, 1);

      for (auto contour : contours) {
        //skip if the area is too little
        if (cv::contourArea(contour) < _minAreaValue)
          continue;

        //otherwise, draw a rectangle
        cv::Rect r = cv::boundingRect(contour);
        cv::rectangle(origFrame, r, {0,255,0}, 2);
      }
    }
    cv::imshow(winName, origFrame);
  }
  else {
    cv::imshow(winName, thisFrame);
  }

  int key = cv::waitKey(delay);
  if (key == 'q')
    return false;
  if (key == 'b') {
    calculateBackground();
    showBackground();
  }

  return true;
}

void Cam::showTrackbars(std::string winName) {
  cv::namedWindow(winName, cv::WINDOW_AUTOSIZE); //create window
  cv::createTrackbar("Threshold", winName, &_thresholdValue, 100);
  cv::createTrackbar("Dilation interaction",winName, &_dilationValue, 20);
  cv::createTrackbar("Min Area", winName, &_minAreaValue, 1000);
  cv::createTrackbar("Consecutive frames", winName, &_consecutiveValue, _maxConsecutiveFrames, consecutiveChanged);
}