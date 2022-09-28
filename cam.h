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
  static int _movedThresholdValue;  //threshold applied to detect when camera moved (in % of pixel different from the BG)
  static cv::Point _mousePosition;  //coordinates of mouse

  //callback functions called if events accour
  static void onMouseClicked(int event, int x, int y, int flag, void* param);
  static void consecutiveChanged(int value, void*);

  int _camIndex;          // = -1 if a recording has been opened
  std::string _fileName;
  cv::VideoCapture _cap;
  cv::Mat _background;
  cv::Mat _lastFrames[_maxConsecutiveFrames];   //store last 50 frames
  int _frameCount;
  cv::Point _origin;

  cv::Mat getBackground(cv::Mat* images_ptr); //get the background from an array of frames
  void reset(); //re-open the camera/recording to reset the position at the beginning

  //detects if the camera has moved
  bool moved();

 public:
  static void setFramesForBG(int framesForBG) { _framesForBG = framesForBG; };
  
  //show trackbars for this window
  static void showTrackbars(std::string winName = "Trackbars");

  //ask the user to synchronize the 2 cameras manually
  // static void synchronize(Cam lhs, Cam rhs);

  Cam(int camIndex) : _camIndex{camIndex}, _frameCount{0} { _cap.open(_camIndex); };
  Cam(std::string fileName) : _camIndex{-1}, _fileName{fileName}, _frameCount{0} { _cap.open(_fileName); };
  ~Cam() { close(); };
  // double fps(); //get fps of the camera/recording
  // void open(int camIndex);
  // void open(std::string fileName);
  void close() { cv::destroyAllWindows(); };

  void showBackground(int delay = 500, std::string winName = "Background"); //show BG for 'delay' milliseconds (0 means forever)
  void calculateBackground(bool fast = false);
  void setFrameCount(int frameCount) { _frameCount = frameCount; };
  
  //show a random frame and ask the user to click the system of reference's origin
  void setOrigin();

  //show current frame for 1000/fps milliseconds (0 fps means forever). Return false if 'q' key is pressed
  bool show(int fps = 30, std::string winName = "default", bool showRectangle = true, bool showHeight = true, bool showBorders = true);
};

#endif //CAM_H