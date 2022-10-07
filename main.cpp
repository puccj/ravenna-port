#include "cam.h"
//#include "video.h"

#include <iostream>

int main() {
  
  Cam right("sample.mp4");
  Cam left("sample2.mp4");

  right.setOrigin();
  right.setScale();
  left.setOrigin();
  left.setScale();

  right.calculateBackground();
  left.calculateBackground();

  //cv::Point shift = {right.origin().x - left.origin().x, right.origin().y - left.origin().y};
  //Cam::showTrackbars();

  while(right.show(60) && left.show(60)) {
    //check for sovrapposition between squares:
    std::vector<cv::Rect> rightBoxes = right.boxes();
    std::vector<cv::Rect> leftBoxes = left.boxes();
    int rightSize = rightBoxes.size();
    int leftSize = rightBoxes.size();

    cv::Size drawDim = {1024,576};
    cv::Point center = {drawDim.width/2, drawDim.height/2};

    cv::Mat draw = cv::Mat::zeros(drawDim, CV_8UC3);
    cv::circle(draw, center ,5,{0,255,0},2);

    for (int i = 0; i < rightSize; i++) {
      cv::rectangle(draw, rightBoxes[i] + center, {255,0,0}, 2);
      for (int j = 0; j < leftSize; j++) {
        cv::rectangle(draw, leftBoxes[j] + center, {0,0,255}, 2);
        if ((rightBoxes[i] & leftBoxes[j]).area() > 0) {
          std::cout << "Debug: r" << i << " - l" << j << " overlapping.\n"; // Difference in position:\n";
          /*std::cout << "x: " << rightBoxes[i].x - leftBoxes[j].x << '\t';
          std::cout << "y: " << rightBoxes[i].y - leftBoxes[j].y << '\t';
          std::cout << "width: " << rightBoxes[i].width - leftBoxes[j].width << '\t';
          std::cout << "height: " << rightBoxes[i].height - leftBoxes[j].height << '\n';*/
          cv::rectangle(draw, (rightBoxes[i] & leftBoxes[j]) + center, {0,255,0}, 2);
        }
      }
    }
    cv::imshow("Draw",draw);
    std::cout << '\n';
    cv::waitKey(0);
  };
  
  /*
  Cam webcam(0);
  webcam.setOrigin();
  webcam.calculateBackground();
  webcam.showBackground();
  while(webcam.show());
  */

  return 0;
}