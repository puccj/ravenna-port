#include "cam.h"
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <random>

// -- Static private --

int Cam::thresholdValue = 50;
int Cam::dilationValue = 1;
int Cam::consecutiveValue = 2;
int Cam::minAreaValue = 700;
unsigned int Cam::globalFrameCount = 0;

int Cam::hueThreshold = 48;
int Cam::saturationThreshold = 64;
int Cam::lowValueThreshold = 48;
int Cam::upValueThreshold = 150;
int Cam::dilationShadow = 1;

int Cam::minDistValue = 30;
int Cam::minRadiusValue = 8;
int Cam::maxRadiusValue = 29;

double Cam::fitLenght = 3;

cv::Point Cam::mousePosition = {-1,-1};

void Cam::onMouseClicked(int event, int x, int y, int flag, void* param) {
  if (event == cv::EVENT_LBUTTONDOWN) {
    mousePosition.x = x;
    mousePosition.y = y;
  }
}

void Cam::consecutiveChanged(int value, void*) {
  globalFrameCount = 0;
}

// -- Private --

void Cam::calculateFps() {
  _fps = _cap.get(cv::CAP_PROP_FPS);

  //if the property is zero, try to calculate it in a different way
  if (_fps != 0)
    return;

  std::cout << "Calculating fps...";
  int num_frames = 60; //number of frames to capture
  time_t start, end;
  cv::Mat frame;
  
  time(&start);
  for(int i = 0; i < num_frames; i++)
    _cap >> frame;
  time(&end);

  double seconds = difftime (end, start);
  std::cout << "     Done (" << num_frames/seconds << " fps)\n";

  //Maybe I need to reset video/camera

  _fps = num_frames / seconds;
}

// -- Static public --

void Cam::showTrackbars(Trackbars type) {
  if (type == Trackbars::All) {
    std::string winName = "All Trackbars";
    cv::namedWindow(winName, cv::WINDOW_NORMAL);
    cv::createTrackbar("Threshold", winName, &thresholdValue, 100);
    cv::createTrackbar("Dilation (detection)",winName, &dilationValue, 20);
    cv::createTrackbar("Min Area", winName, &minAreaValue, 1000);
    cv::createTrackbar("Consecutive frames", winName, &consecutiveValue, maxConsecutiveFrames, consecutiveChanged);
    cv::setTrackbarMin("Consecutive frames", winName, 1);
    cv::createTrackbar("Hue threshold", winName, &hueThreshold, 150);
    cv::createTrackbar("Saturation threshold", winName, &saturationThreshold, 150);
    cv::createTrackbar("Lower Value Threshold", winName, &lowValueThreshold, 200);
    cv::createTrackbar("Upper Value Threshold", winName, &upValueThreshold, 200);
    cv::createTrackbar("Dilation (shadow)", winName, &dilationShadow, 5);  
    cv::createTrackbar("Min distance", winName, &minDistValue, 100);
    cv::setTrackbarMin("Min distance", winName, 1);
    cv::createTrackbar("Min radius", winName, &minRadiusValue, 50);
    cv::createTrackbar("Max radius", winName, &maxRadiusValue, 100);
    return;
  }
  if (type == Trackbars::Detection) {
    std::string winName = "Detection Trackbars";
    cv::namedWindow(winName);
    cv::createTrackbar("Threshold", winName, &thresholdValue, 100);
    cv::createTrackbar("Dilation (detection)",winName, &dilationValue, 20);
    cv::createTrackbar("Min Area", winName, &minAreaValue, 1000);
    cv::createTrackbar("Consecutive frames", winName, &consecutiveValue, maxConsecutiveFrames, consecutiveChanged);
    cv::setTrackbarMin("Consecutive frames", winName, 1);
    return;
  }
  if (type == Trackbars::Shadow) {
    std::string winName = "Shadow Trackbars";
    cv::namedWindow(winName);
    cv::createTrackbar("Hue threshold", winName, &hueThreshold, 150);
    cv::createTrackbar("Saturation threshold", winName, &saturationThreshold, 150);
    cv::createTrackbar("Lower Value Threshold", winName, &lowValueThreshold, 200);
    cv::createTrackbar("Upper Value Threshold", winName, &upValueThreshold, 200);
    cv::createTrackbar("Dilation (shadow)", winName, &dilationShadow, 5);  
    return;
  }
  if (type == Trackbars::Circle) {
    std::string winName = "Circle Trackbars";
    cv::namedWindow(winName);
    cv::createTrackbar("Min distance", winName, &minDistValue, 100);
    cv::setTrackbarMin("Min distance", winName, 1);
    cv::createTrackbar("Min radius", winName, &minRadiusValue, 50);
    cv::createTrackbar("Max radius", winName, &maxRadiusValue, 100);
    return;
  }
}

enum class Cam::Parameter{
  thresholdValue, dilationValue, consecutiveValue, minAreaValue,
  hueThreshold, saturationThreshold, lowValueThreshold, upValueThreshold, dilationShadow,
  minDistValue, minRadiusValue, maxRadiusValue,
  fitLenght
};

void Cam::setParameter(Parameter param, int value) {
  //TO DO
}

// -- Public --

Cam::Cam(int camIndex, std::string camName) 
  : _frameCount{0}, _fitCount{0}, _camName{camName}, _origin{cv::Point{-1,-1}}, 
  _fileOpened{false}, _cap{cv::VideoCapture(camIndex)} { 
  
  if (camName == "Default")
    _camName = std::to_string(camIndex);

  calculateFps();
}

Cam::Cam(std::string filePath, std::string camName)
  : _frameCount{0}, _fitCount{0}, _camName{camName}, _origin{cv::Point{-1,-1}}, 
  _fileOpened{true}, _cap{cv::VideoCapture(filePath)} {

  if (camName == "Default")
    _camName = filePath;

  calculateFps();
}

void Cam::showBackground() {
  cv::imshow(_camName + ": Background", _background/255);
}

void Cam::calculateBackground(double seconds, double weight) {
  std::cout << "Getting color background... ";
  
  int frames = seconds * _fps;

  if (frames == 0 && !_fileOpened)
    frames = 60;
  cv::Mat firstFrame;
  _cap.read(firstFrame);
  
  cv::Mat acc(firstFrame.size(), CV_32FC3);

  int counter = 1;
  while (true) {
    cv::Mat thisFrame;
    _cap.read(thisFrame);
    counter++;
    if (thisFrame.empty())
      break;
    if (counter > frames && frames != 0)
      break;

    cv::accumulateWeighted(thisFrame, acc, weight);
  }

  //if cap is a file, set to the beggining of the video
  if (_fileOpened)
    _cap.set(cv::CAP_PROP_POS_FRAMES, 0);

  std::cout << "     Done\n";
  _background = acc;
}

void Cam::setOrigin() {
  cv::namedWindow(_camName + ": Select origin");
  cv::setMouseCallback(_camName + ": Select origin", onMouseClicked);

  if (_fileOpened)
    std::cout << "Click on the system of reference's origin. Press 'n' to show another (random) frame. Press 'space' to confirm.\n";
  else
    std::cout << "Click on the system of reference's origin. Press 'space' to confirm.\n";

  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, _cap.get(cv::CAP_PROP_FRAME_COUNT)-1);
  int key;
  do {
    key = 1;

    cv::Mat frame;
    if (_fileOpened) {
      int rand = dist(rd);
      _cap.set(cv::CAP_PROP_POS_FRAMES, rand);
    }
    _cap.read(frame);

    do {
      cv::Mat frameWithCross = frame.clone();
      //draw a cross
      if (mousePosition.x != -1 && mousePosition.y != -1) {
        cv::drawMarker(frameWithCross, mousePosition,{0,0,255},1);
      }
      cv::imshow(_camName + ": Select origin", frameWithCross);
      key = cv::waitKeyEx(10);

      //move the cross
      if (key == 2424832)       //left
        mousePosition.x--;
      else if (key == 2555904)  //right
        mousePosition.x++;
      else if (key == 2490368)  //up
        mousePosition.y--;
      else if (key == 2621440)  //down
        mousePosition.y++;
    }
    while(_fileOpened && key != 'n' && key != ' ');
  }
  while(key != ' ');

  _origin = mousePosition;
  cv::destroyWindow(_camName + ": Select origin");

  if (_fileOpened)
    _cap.set(cv::CAP_PROP_POS_FRAMES, 0);
}

void Cam::setScale() {
  if (_origin == cv::Point{-1,-1})
    setOrigin();

  mousePosition = {-1,-1};  //reset mouse position
  cv::namedWindow(_camName + ": Select second point");
  cv::setMouseCallback(_camName + ": Select second point", onMouseClicked);

  if (_fileOpened)
    std::cout << "Click on the system of reference's origin. Press 'n' to show another (random) frame. Press 'space' to confirm.\n";
  else
    std::cout << "Click on a second point. Press 'space' to confirm.\n";

  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, _cap.get(cv::CAP_PROP_FRAME_COUNT)-1);  
  int key;
  do {
    key = -1;
    cv::Mat frame;
    if (_fileOpened) {
      int rand = dist(rd);
      _cap.set(cv::CAP_PROP_POS_FRAMES, rand);
    }
    _cap.read(frame);

    do {
      cv::Mat frameWithCross = frame.clone();
      //draw origin point
      cv::drawMarker(frameWithCross, _origin, {0,0,255}, 3, 5, 1);

      //draw a cross
      if (mousePosition.x != -1 && mousePosition.y != -1) {
        cv::drawMarker(frameWithCross, mousePosition,{0,0,255},1);
      }
      cv::imshow(_camName + ": Select second point", frameWithCross);
      key = cv::waitKeyEx(10);
      
      //move the cross
      if (key == 2424832)       //left
        mousePosition.x--;
      else if (key == 2555904)  //right
        mousePosition.x++;
      else if (key == 2490368)  //up
        mousePosition.y--;
      else if (key == 2621440)  //down
        mousePosition.y++;
    }
    while (_fileOpened && key != ' ' && key != 'n');
  }
  while (key != ' ');
  
  cv::destroyWindow(_camName + ": Select second point");
  
  //calculate scale factor
  double realDistance;
  std::cout << "Insert the distance (cm) between the points: ";
  std::cin >> realDistance;
  
  _scaleFactor = realDistance / cv::norm(_origin - mousePosition);
  std::cout << "Scale factor is " << _scaleFactor << " cm/pixel\n";

  if (_fileOpened)
    _cap.set(cv::CAP_PROP_POS_FRAMES, 0);
}

bool Cam::process(bool drawContours, bool drawRects, bool drawCircles) {
  cv::Mat grayBackground;
  cv::Mat hsvBackground;
  cv::cvtColor(_background,grayBackground, cv::COLOR_BGR2GRAY);
  grayBackground.convertTo(grayBackground, CV_8U);
  cv::cvtColor(_background, hsvBackground, cv::COLOR_BGR2HSV);

  cv::Mat frame;
  _cap.read(frame);
  
  if (frame.empty()) {
    std::cout << "End of video\n";
    return false;
  }

  _lastFrame = frame.clone();
  cv::Mat grayFrame;
  cv::Mat hsvFrame;
  cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);
  cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

  // -- Foreground detection --
  cv::absdiff(grayFrame, grayBackground, frame);                        //subtract background from image
  cv::threshold(frame, frame, thresholdValue, 255, cv::THRESH_BINARY);  //thresholding to convert to binary
  cv::dilate(frame, frame, cv::Mat(), cv::Point(-1,-1), dilationValue); //dilate the image (no inside dark regions)
  
  //check if globalFrameCount has been reset (consecutiveValue changed)
  if (globalFrameCount < _frameCount)
    _frameCount = 0;
    
  _lastBinary[_frameCount % consecutiveValue] = frame;
  _frameCount++;

  //skip if list is not already full
  if (_frameCount < consecutiveValue)
    return true;
  
  //add all frames in list
  cv::Mat sum = _lastBinary[0];
  for (auto i = 1; i < consecutiveValue; ++i)
    sum += _lastBinary[i];

  // -- Shadow detection --

  std::vector<cv::Point> whitePoints; //store white points in sum
  cv::findNonZero(sum, whitePoints);

  cv::Mat shadow(hsvFrame.size(), CV_8UC1, cv::Scalar(0)); //store pixels considered to be shadow
  for (auto point : whitePoints) {
    //color is stored in a Vec3b. val[0]=Hue, val[1]=Saturation, val[2]=Value
    //a pixel is a shadow if the color has approxiamtetly the same hue and sat, but lower value than the bg
    cv::Vec3b frameColor = hsvFrame.at<cv::Vec3b>(point);
    cv::Vec3b bgColor = hsvBackground.at<cv::Vec3b>(point);
    if (std::abs(frameColor.val[0] - bgColor.val[0]) < hueThreshold
              && frameColor.val[1] - bgColor.val[1] < saturationThreshold 
              && frameColor.val[2] * 1.0 / bgColor.val[2] > lowValueThreshold/100.0 
              && frameColor.val[2] * 1.0 / bgColor.val[2] < upValueThreshold/100.0) {
      shadow.at<uchar>(point) = 255;
    }
  }
  cv::dilate(shadow, shadow, cv::Mat(), cv::Point(-1,-1), dilationShadow);

  //remove shadows from detected image
  sum -= shadow;
  
  //find contours and draw them
  std::vector<cv::Mat> contours;
  cv::findContours(sum, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  if (drawContours)
    cv::drawContours(_lastFrame, contours, -1, {255,0,0}, 1);

  // -- Head detection --

  //find cirlces
  cv::medianBlur(grayFrame, grayFrame, 3);
  std::vector<cv::Vec3f> circles;
  cv::HoughCircles(grayFrame, circles, cv::HOUGH_GRADIENT, 1, minDistValue, 100, 30, minRadiusValue, maxRadiusValue);

  //for each circle, check if it's inside a contour
  for (auto circle : circles) {
    cv::Point center(circle[0], circle[1]);
    bool inside = false;
    for (auto contour : contours) {
      //skip if area of contour is too little (it's probably noise)
      if (cv::contourArea(contour) < minAreaValue)
        continue;
      
      //otherwise draw a rectangle
      cv::Rect r = cv::boundingRect(contour);
      if (drawRects)
        cv::rectangle(_lastFrame, r, {0,255,0}, 2);

      //check if head is inside rectangle
      if (center.inside(r)) {
      //if (cv::pointPolygonTest(contour, center, false) >= 0) {
        inside = true;
        break;
      }
    }

    //skip if point is not inside a contour
    if (!inside)
      continue;

    //otherwise add to fit
    _headPositions[_fitCount % maxFitLenght] = {center.x, center.y, _frameCount};
    _fitCount++;
    globalFrameCount++;

    //draw the circle
    if (drawCircles)
      cv::circle(_lastFrame, center, 1, {255,0,255}, 3, cv::LINE_AA);
  }

  return true;
}

Fit3d Cam::fit(bool draw) {
  /* Linear fit of: 
   * - trajectory (assumed linear)  (y = a + bx)      y(x)
   * - vx (assumed constant)        (x = x0 + vx *t)  x(t)
   * - vy (assumed constant)        (y = y0 + vy *t)  y(t)
   */
  
  int size = maxFitLenght;
  if (_fitCount < maxFitLenght)
    size = _fitCount;

  //convert seconds in number of frames
  int framesForFit = fitLenght * _fps;

  double sumX = 0;
  double sumY = 0;
  double sumT = 0;
  int skipped = 0;
  for (auto i = 0; i < size; ++i) {
    //skip if frame is too old
    if (_frameCount - _headPositions[i].z > framesForFit) {
      skipped++;
      continue;
    }

    sumX += _headPositions[i].x;
    sumY += _headPositions[i].y;
    sumT += _headPositions[i].z;
  }
  double averageX = sumX / (size-skipped);
  double averageY = sumY / (size-skipped);
  double averageT = sumT / (size-skipped);

  double numYX = 0;
  double numXT = 0;
  double numYT = 0;
  double denX = 0;
  double denT = 0;
  for (auto i = 0; i < size; ++i) {
    //skip if frame is too old
    if (_frameCount - _headPositions[i].z > framesForFit)
      continue;

    double xMinusAvarage = _headPositions[i].x - averageX;
    double tMinusAvarage = _headPositions[i].z - averageT;
    numYX += xMinusAvarage*(_headPositions[i].y - averageT);
    numXT += tMinusAvarage*(_headPositions[i].x - averageX);
    numYT += tMinusAvarage*(_headPositions[i].y - averageY);
    denX += xMinusAvarage*xMinusAvarage;
    denT += tMinusAvarage*tMinusAvarage;
  }

  double b = numYX/denX;
  double a = averageY - b*averageX;
  
  Fit3d result;
  result.b = numYX/denX;
  result.a = averageY - result.b*averageX;
  result.vx = numXT/denT;
  result.x0 = averageX - result.vx*averageT;
  result.vy = numYT/denT;
  result.y0 = averageY - result.vy*averageT;

  if (draw) {
    int cols = _lastFrame.cols;
    // int a = result.a;
    // int b = result.b;
    cv::line(_lastFrame, {0,a}, {cols, a + b*cols}, {0,0,255}, 2);
  }

  //If I want: Remove some points and redo the fit

  return result;
}

bool Cam::show(int delay, std::string winName) {
  if (winName == "Default")
    winName = _camName + ": Result";

  cv::imshow(winName, _lastFrame);
  int k = cv::waitKey(delay);
  if (k == 'q')
    return false;
  return true;
}