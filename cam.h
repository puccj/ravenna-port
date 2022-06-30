#ifndef CAM_H
#define CAM_H

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>  //for video capture
#include <opencv2/highgui.hpp>  //for destroyAllWindow and imshow

class Cam {
  static int _framesForBG;                      //number of frames used to create BG (the same for every Cam)
  static const int _maxConsecutiveFrames = 50;  //max number of consecutive frames used to calculate movement
  static int _thresholdValue;   //threshold applied during binary conversion
  static int _dilationValue;    //number of times dilation is applied
  static int _minAreaValue;     //areas less then this pixels will be ignored
  static int _consecutiveValue; //number of frames merged together to create borders
  static int _globalFrameCount; //needed to deal with change in slidebar

  static void consecutiveChanged(int value, void*);

  int _camIndex;
  cv::VideoCapture _cap;
  cv::Mat _background;
  cv::Mat _lastFrames[_maxConsecutiveFrames];   //store last 50 frames
  int _frameCount;

 public:
  static void setFramesForBG(int framesForBG) { _framesForBG = framesForBG; };
  Cam(int camIndex) : _camIndex{camIndex}, _frameCount{0} { _cap.open(_camIndex); };
  ~Cam() { close(); };
  void open(int camIndex);
  void close() { cv::destroyAllWindows(); };

  void showBackground(int delay = 500, std::string winName = "Background"); //show BG for 'delay' milliseconds (0 means forever)
  void calculateBackground();
  void setFrameCount(int frameCount) { _frameCount = frameCount; };
  
  //show current frame for 'delay' milliseconds (0 means forever). Return false if 'q' key is pressed
  bool show(std::string winName = "default", bool showRectangle = true, bool showBorders = true, int delay = 5);

  //show trackbars for this window
  void showTrackbars(std::string winName = "Trackbars");
};

#endif //CAM_H