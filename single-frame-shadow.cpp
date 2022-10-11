#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

int main() {
  cv::Mat background = cv::imread("background.png");
  cv::Mat frame = cv::imread("frame.png");
  cv::Mat fg = cv::imread("fg.png");

  cv::Mat hsvBackground;
  cv::Mat hsvFrame;
  cv::cvtColor(background, hsvBackground, cv::COLOR_BGR2HSV);
  cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);

  int hueThreshold = 500;
  int saturationThreshold = 500;
  int lowValueThreshold = 5;
  int upValueThreshold = 500;
  cv::namedWindow("Trackbars", cv::WINDOW_AUTOSIZE); //create window
  cv::createTrackbar("Hue threshold", "Trackbars", &hueThreshold, 500);
  cv::createTrackbar("Saturation threshold","Trackbars", &saturationThreshold, 500);
  cv::createTrackbar("Lower Value Threshold", "Trackbars", &lowValueThreshold, 500);
  cv::createTrackbar("Upper Value Threshold", "Trackbars", &upValueThreshold, 500);

  std::vector<cv::Point> whitePoints;
  for (int i = 0; i < fg.cols; i++) {
    for (int j = 0; j < fg.rows; j++) {
      if (fg.at<cv::Vec3b>(cv::Point(i,j)) == cv::Vec3b{255,255,255}) {
        //std::cout << i << ", " << j << '\n';
        whitePoints.push_back({i,j});
      }
    }
  }

  while (true) {
    for (auto point : whitePoints) {
      cv::Vec3b frameColor = hsvFrame.at<cv::Vec3b>(point);
      cv::Vec3b backgroundColor = hsvBackground.at<cv::Vec3b>(point);
      if (std::abs(frameColor.val[0] - backgroundColor.val[0]) < hueThreshold
                && frameColor.val[1] - backgroundColor.val[1] < saturationThreshold 
                && frameColor.val[2] < backgroundColor.val[2])
      frame.at<cv::Vec3b>(point) = {100,100,100};
    }

    cv::imshow("fg", fg);
    cv::imshow("Frame", frame);
    cv::imshow("Background", background);
    int k = cv::waitKey(30);
    if (k == 'q')
      break;
  }
}