#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

int main() {
  cv::VideoCapture cap("AXIS P1435-LE 2022-04-28_15_19_54_981.asf");
  cv::Mat firstFrame;
  cap.read(firstFrame);
  
  cv::Mat background1(firstFrame.size(), CV_32FC3);
  cv::Mat background2(firstFrame.size(), CV_32FC3);
  cv::Mat background3(firstFrame.size(), CV_32FC3);

  while (true) {
    cv::Mat thisFrame;
    cap.read(thisFrame);

    cv::accumulateWeighted(thisFrame, background1, 0.01);
    cv::accumulateWeighted(thisFrame, background2, 0.005);
    cv::accumulateWeighted(thisFrame, background3, 0.001);

    cv::Mat bg1 = background1 /255;
    cv::Mat bg2 = background2 /255;
    cv::Mat bg3 = background3 /255;

    cv::imshow("frame", thisFrame);
    cv::imshow("bg1", bg1);
    cv::imshow("bg2", bg2);
    cv::imshow("bg3", bg3);

    int k = cv::waitKey(0);
    if (k == 'q')
      break;
  }
}