#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

cv::Mat getBackground(std::string path, double alpha = 0.005) {
  std::cout << "Getting color background... ";
  cv::VideoCapture cap(path);
  cv::Mat firstFrame;
  cap.read(firstFrame);
  
  cv::Mat acc(firstFrame.size(), CV_32FC3);

  while (true) {
    cv::Mat thisFrame;
    cap.read(thisFrame);
    if (thisFrame.empty())
      break;

    cv::accumulateWeighted(thisFrame, acc, alpha);
  }

  std::cout << "     Done\n";
  return acc;
}


int hueThreshold = 48;
int saturationThreshold = 64;
int lowValueThreshold = 48;
int upValueThreshold = 150;
int dilationShadow = 1;

void showTrackbars(std::string windowName = "Trackbars") {
  cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE); //create window
  cv::createTrackbar("Hue threshold", windowName, &hueThreshold, 200);
  cv::createTrackbar("Saturation threshold", windowName, &saturationThreshold, 200);
  cv::createTrackbar("Lower Value Threshold", windowName, &lowValueThreshold, 200);
  cv::createTrackbar("Upper Value Threshold", windowName, &upValueThreshold, 200);
  cv::createTrackbar("Dilation", windowName, &dilationShadow, 5);  
}

int main() {
  int thresholdValue = 50;
  int dilationValue = 1;
  int consecutiveValue = 2;
  int minAreaValue = 700;

  cv::Mat background = getBackground("sample.mp4");
  //std::cout << "BG: " << background.size << ", " << background.channels() << '\n';
  cv::Mat grayBackground;
  cv::Mat hsvBackground;
  cv::cvtColor(background,grayBackground, cv::COLOR_BGR2GRAY);
  grayBackground.convertTo(grayBackground, CV_8U);
  cv::cvtColor(background, hsvBackground, cv::COLOR_BGR2HSV);
  //std::cout << "Gray BG: " << grayBackground.size << ", " << grayBackground.channels() << " (" << grayBackground.type() << ")\n";

  cv::VideoCapture cap("sample.mp4");

  cv::Mat lastFrames[consecutiveValue];
  int frameCount = 0;

  showTrackbars();

  while (true) {
    cv::Mat frame;
    cap.read(frame);

    cv::Mat origFrame = frame.clone();

    cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);                       //convert to grayscale
    //std::cout << "Frame: " << frame.size << ", " << frame.channels() << " (" << frame.type() << ")\n";
    cv::absdiff(frame, grayBackground, frame);                            //subtract background from image
    cv::threshold(frame, frame, thresholdValue, 255, cv::THRESH_BINARY);  //thresholding to convert to binary
    cv::dilate(frame, frame, cv::Mat(), cv::Point(-1,-1), dilationValue); //dilate the image (no inside dark regions)
      
    lastFrames[frameCount % consecutiveValue] = frame;
    frameCount++;

    //skip if list is not already full
    if (frameCount < consecutiveValue) 
      continue;
    
    //add all frames in list
    cv::Mat sum = lastFrames[0];
    for (int i = 1; i < consecutiveValue; i++)
      sum += lastFrames[i];

    //find shadows and draw them
    cv::Mat hsvFrame;
    cv::cvtColor(origFrame, hsvFrame, cv::COLOR_BGR2HSV);

    std::vector<cv::Point> whitePoints;
    cv::Mat shadow(origFrame.size(), CV_8UC1, cv::Scalar(0));

    cv::findNonZero(sum, whitePoints);
    for (auto point : whitePoints) {
      cv::Vec3b frameColor = hsvFrame.at<cv::Vec3b>(point);
      cv::Vec3b backgroundColor = hsvBackground.at<cv::Vec3b>(point);
      if (std::abs(frameColor.val[0] - backgroundColor.val[0]) < hueThreshold
                && frameColor.val[1] - backgroundColor.val[1] < saturationThreshold 
                && frameColor.val[2] * 1.0 / backgroundColor.val[2] > lowValueThreshold/100.0 
                && frameColor.val[2] * 1.0 / backgroundColor.val[2] < upValueThreshold/100.0) {
        //origFrame.at<cv::Vec3b>(point) = {255,0,255};
        shadow.at<uchar>(point) = 255;
      }
    }

    //whitePoints.clear();
    cv::dilate(shadow, shadow, cv::Mat(), cv::Point(-1,-1), dilationShadow);
    cv::imshow("Shadow", shadow);

    //remove shadows from detected image
    sum -= shadow;
    cv::imshow("Detection", sum);

    //find the contours and draw them
    std::vector<cv::Mat> contours;
    cv::findContours(sum, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::drawContours(origFrame, contours, -1, {255,0,0}, 1);

    for (auto contour : contours) {
      //skip if the area is too little
      if (cv::contourArea(contour) < minAreaValue)
        continue;

      //create and draw a rectangle
      cv::Rect r = cv::boundingRect(contour);
      cv::rectangle(origFrame, r, {0,255,0}, 2);
    }

    cv::imshow("Result", origFrame);
    int k = cv::waitKey(100);
    if (k == 'q')
      break;
  }

  return 0;
}