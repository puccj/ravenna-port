#ifndef CAM_H
#define CAM_H

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#define Tracks Cam::Trackbars
#define Param Cam::Parameter

struct Fit3d {
  double a,b,x0,vx,y0,vy;
};

class Cam {
  //Parameters for foreground detection
  static int thresholdValue;
  static int dilationValue;
  static int consecutiveValue;
  static int minAreaValue;
  static const int maxConsecutiveFrames = 20;

  //if the consecutiveFrame slider moves, _frameCount of each Cam need to be reset to prevent crash
  //to do that, the static global counter goes to 0 when the slider moves and each Cam object check on that
  static unsigned int globalFrameCount;

  //Parameters for shadow detection
  static int hueThreshold;
  static int saturationThreshold;
  static int lowValueThreshold;
  static int upValueThreshold;
  static int dilationShadow;

  //Parameters for head detection
  static int minDistValue;
  static int minRadiusValue;
  static int maxRadiusValue;

  //Parameters for fit of head positions
  static const int maxFitLenght = 100;  //max number of past frame stored to calculate the fit with
  static double fitLenght;              //number of past seconds to calcualate the fit with

  static cv::Point mousePosition;
  static void onMouseClicked(int event, int x, int y, int flag, void* param);
  static void consecutiveChanged(int value, void*);

  bool _fileOpened;     //true if a video is opened, false if a camera is opened
  std::string _camName; //needed to differentiate windows of different cameras
  cv::VideoCapture _cap;
  cv::Mat _background;  //store the background image
  cv::Point _origin;    //frame of reference's origin
  double _scaleFactor;  //[cm/pixel] number of cm corresponding to 1 pixel
  double _fps;
  int _frameCount;
  cv::Mat _lastBinary[maxConsecutiveFrames];  //store last binary images to perform detection
  cv::Mat _lastFrame;                         //store the last frame acquired
  int _fitCount;
  cv::Point3i _headPositions[maxFitLenght];   //store last positions of the head to perform fit

  void calculateFps();

 public:
  enum class Trackbars{All, Detection, Shadow, Circle};
  enum class Parameter;
  static void showTrackbars(Trackbars type = Trackbars::All);
  static void setParameter(Parameter param, int value);
  Cam(int camIndex, std::string camName = "Default"); 
  Cam(std::string filePath, std::string camName = "Default");

  //show the background for 'delay' milliseconds (0 = untill a key is pressed)
  void showBackground();
  //calculate the starting background using given number of seconds
  //(0(default) = entire video lenght for a file, 5 seconds for a camera)
  void calculateBackground(double seconds = 0, double weight = 0.005);
  void setOrigin();
  void setScale();

  //calculate head positions for the current frame (needs to be called repeatedly)
  //return false if the loop that calls this function should break
  bool process(bool drawContours = false, bool drawRects = false, bool drawCircles = false);

  //calculate the linear fit of trajectory and velocity using the head positions
  //if draw == true, draw the trajectory fit over the original frame
  Fit3d fit(bool draw = false);

  //show the last frame taken for 'delay' milliseconds time (0 = untill a key is pressd)
  //return false if the loop that calls this function should break
  bool show(int delay = 0, std::string winName = "Default");
};

#endif