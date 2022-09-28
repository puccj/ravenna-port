#include "cam.h"

#include <iostream>
#include <random>
#include <opencv2/imgproc.hpp>  //for cvtColor (convertion to B&W)

int Cam::_framesForBG = 50;
int Cam::_thresholdValue = 50;  
int Cam::_dilationValue = 2;   
int Cam::_minAreaValue = 700;  
int Cam::_consecutiveValue = 2;
int Cam::_globalFrameCount = 0;
cv::Point Cam::_mousePosition = {-1,-1};

void Cam::consecutiveChanged(int value, void*) {
  _globalFrameCount = 0;
  
  if (value == 0) {
    _consecutiveValue = 1;
    return;
  }
}

void Cam::onMouseClicked(int event, int x, int y, int flag, void* param) {
  if (event == cv::EVENT_LBUTTONDOWN) {
    _mousePosition.x = x;
    _mousePosition.y = y;
  }
}

cv::Mat Cam::getBackground(cv::Mat* frames_ptr) {
  int rows = frames_ptr[0].rows;
  int cols = frames_ptr[0].cols;
  
  cv::Mat background(rows, cols, CV_8UC1);  //store the background image

  for (int x = 0; x < cols; x++) {
    for (int y = 0; y < rows; y++) {
      //create an array with the (x,y) pixel of all frames
      uchar currentPixel[_framesForBG];

      for (int i = 0; i < _framesForBG; i++) {
        //insert sort: pos is the position where the element will be inserted
        int pos = i-1;
        while(pos>=0 && frames_ptr[i].at<uchar>(y,x) < currentPixel[pos]) {
          currentPixel[pos+1] = currentPixel[pos];
          pos--;
        }
        currentPixel[pos+1] = frames_ptr[i].at<uchar>(y,x);
      }
      //now currentPixel is a sorted array with (x,y) pixel by all frames.
      //gets the median value and write it to the (x,y) pixel in background image
      background.at<uchar>(y,x) = currentPixel[_framesForBG/2];
    }
  }
  
  return background;
}

void Cam::reset() {
  if (_camIndex != -1)
    _cap.open(_camIndex);
  else
    _cap.open(_fileName);
    
  _frameCount = 0;
}

bool Cam::moved() {
  cv::Mat last = _lastFrames[(_frameCount-1) % _maxConsecutiveFrames];
  std::vector<cv::Point> blackPixels;
  cv::findNonZero(last, blackPixels);

  //std::cout << "DEBUG: The number of black pixels are: " << blackPixels.size() << '\n';

  /* int posOfLastFrame = (_frameCount % _maxConsecutiveFrames)-1;
  
  //sum last _consecutiveFrames-1 frames
  cv::Mat sum = _lastFrames[posOfLastFrame-1];
  for (int i = 2; i < _consecutiveValue; i++) {
    int posOfCurrent = posOfLastFrame - i;
    if (posOfCurrent < 0)
      posOfCurrent += _maxConsecutiveFrames;
    sum += _lastFrames[posOfCurrent];
  }

  cv::Mat result;

  //calculate difference with current frame and show it
  cv::absdiff(_lastFrames[posOfLastFrame], sum, result);
   */
  
  return false;
}

//    --Public--

void Cam::showTrackbars(std::string winName) {
  cv::namedWindow(winName, cv::WINDOW_AUTOSIZE); //create window
  cv::createTrackbar("Threshold", winName, &_thresholdValue, 100);
  cv::createTrackbar("Dilation interaction",winName, &_dilationValue, 20);
  cv::createTrackbar("Min Area", winName, &_minAreaValue, 1000);
  cv::createTrackbar("Consecutive frames", winName, &_consecutiveValue, _maxConsecutiveFrames, consecutiveChanged);
}

/*
void Cam::synchronize(Cam lhs, Cam rhs) {
  //show the first frame of both
  cv::Mat thisFrame;
  cv::Mat otherFrame;

  lhs.

  _cap.read(thisFrame);
  other.
}
*/

/*
double Cam::fps() {
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
  
  //reset cam/video at the beginning
  reset();

  return num_frames / seconds;
}
*/

/*
void Cam::open(int camIndex) {
  _camIndex = camIndex;
  _cap.open(camIndex);
  _frameCount = 0;
}

void Cam::open(std::string fileName) {
  _camIndex = -1;
  _fileName = fileName;
  _cap.open(fileName);
  _frameCount = 0;
}
*/

void Cam::calculateBackground(bool fast) {
  std::cout << "Getting Background... ";
  
  cv::Mat frames[_framesForBG];  //stored frames to calculate the bkgnd with  

  if (_camIndex >= 0 || fast) { //if a camera is opened, get consecutive BG frames..
    for (int i = 0; i < _framesForBG; i++) {
      _cap.read(frames[i]);
      cv::cvtColor(frames[i], frames[i], cv::COLOR_BGR2GRAY);
    }
  }
  else if (_camIndex == -1) { //..otherwise, if a video file has been opened, get random frames to bild the BG
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, _cap.get(cv::CAP_PROP_FRAME_COUNT)-1);
    for (int i = 0; i < _framesForBG; i++) {
      int rand = dist(rd);
      //std::cout << "Debug: rand = " << rand << '\n';
      _cap.set(cv::CAP_PROP_POS_FRAMES, rand); //set the frame id to read that particular frame
      _cap.read(frames[i]);  //read that frame
      cv::cvtColor(frames[i], frames[i], cv::COLOR_BGR2GRAY); //convert it in B&W
    }
  }

  _background = getBackground(frames);
  reset();
  std::cout << "     Done\n";
}

void Cam::showBackground(int delay, std::string winName) {
  cv::imshow(winName, _background);
  cv::waitKey(delay);
}

void Cam::setOrigin() {
  cv::namedWindow("Select origin");
  cv::setMouseCallback("Select origin", onMouseClicked);

  int key;

  if (_camIndex >= 0) {
    //for a camera, take the current frame
    std::cout << "Click on the system of reference's origin. Press 'space' to confirm.\n";

    do {
      key = -1;
      cv::Mat thisFrame;
      _cap.read(thisFrame);

      cv::Mat frameWithCross = thisFrame.clone();

      if (_mousePosition.x != -1 && _mousePosition.y != -1) {
        //draw a cross
        cv::line(frameWithCross, {_mousePosition.x-10, _mousePosition.y-10}, {_mousePosition.x+10, _mousePosition.y+10}, {0,0,255},2);
        cv::line(frameWithCross, {_mousePosition.x-10, _mousePosition.y+10}, {_mousePosition.x+10, _mousePosition.y-10}, {0,0,255},2);
      }

      cv::imshow("Select origin", frameWithCross);

      key = cv::waitKey(10);
    }
    while(key != ' ');
  }
  else {
    //for a video, take a random frame
    std::cout << "Click on the system of reference's origin. Press 'n' to show another (random) frame. Press 'space' to confirm.\n";
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, _cap.get(cv::CAP_PROP_FRAME_COUNT)-1);
    
    do {
      key = -1;

      int rand = dist(rd);
      //std::cout << "Debug: rand is: " << rand << '\n';
      cv::Mat randomFrame;
      _cap.set(cv::CAP_PROP_POS_FRAMES, rand);
      _cap.read(randomFrame);
      cv::imshow("Select origin", randomFrame);
      
      while(key != ' ' && key != 'n') {
        key = cv::waitKey(10);
        cv::Mat frameWithCross = randomFrame.clone();

        if (_mousePosition.x != -1 && _mousePosition.y != -1) {
          //draw a cross
          cv::line(frameWithCross, {_mousePosition.x-10, _mousePosition.y-10}, {_mousePosition.x+10, _mousePosition.y+10}, {0,0,255},2);
          cv::line(frameWithCross, {_mousePosition.x-10, _mousePosition.y+10}, {_mousePosition.x+10, _mousePosition.y-10}, {0,0,255},2);
        }
        cv::imshow("Select origin", frameWithCross);
      }
    }
    while(key != ' ');
  }

  _origin = _mousePosition;
  cv::destroyWindow("Select origin");
}

bool Cam::show(int fps, std::string winName, bool showRectangle, bool showHeight, bool showBorders) {
  if (winName == "default") {
    if (_camIndex > 0)
      winName = "Camera " + std::to_string(_camIndex);
    else
      winName = _fileName;
  }

  if (fps < 0) {
    std::cout << "WARNING: negative value set for fps. Using standard 30 fps instead.\n";
    fps = 30;
  }

  //read a frame
  cv::Mat thisFrame;
  _cap.read(thisFrame);
  
  if (thisFrame.empty()) {
    std::cout << "End of video\n";
    return false;
  }

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
    
    _lastFrames[_frameCount % _maxConsecutiveFrames] = thisFrame;
    _frameCount++;
    _globalFrameCount++;

    //skip if list is not already full
    if (_frameCount >= _consecutiveValue) {

      //sum the last "_consecutiveFrames" frames in list
      int posOfLastFrame = ((_frameCount-1) % _maxConsecutiveFrames);
      cv::Mat sum = _lastFrames[posOfLastFrame].clone();
    
      for (int i = 1; i < _consecutiveValue; i++) {
        int posOfCurrent = posOfLastFrame - i;
        if (posOfCurrent < 0)
          posOfCurrent += _maxConsecutiveFrames;
        sum += _lastFrames[posOfCurrent];
      }

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

        if (showHeight) {
          cv::Point center = {r.x + r.width/2, r.y + r.height/2};
          double distance = sqrt((center.x-_origin.x)*(center.x-_origin.x) + (center.y-_origin.y)* (center.y-_origin.y));
          cv::putText(origFrame,std::to_string((int) distance), {center.x, center.y}, cv::FONT_HERSHEY_SIMPLEX, 1, {0,255,255});
        }
      }
    }
    cv::imshow(winName, origFrame);

    //check if camera has moved
    if (moved())
      calculateBackground();
  }
  else {
    cv::imshow(winName, thisFrame);
  }

  int delay = 0;
  if (fps != 0)
    delay = 1000/fps;
  
  int key = cv::waitKey(delay);

  if (key == 'q')
    return false;
  if (key == 'b') {
    calculateBackground();
    showBackground();
  }
  if (key == 'm')
    moved();
  if (key == 't')
    showTrackbars();
  if (key == 'p')
    cv::waitKey(0);
  if (key == 'r')
    reset();

  return true;
}