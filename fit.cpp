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

void showShadowTrackbars(std::string windowName = "Shadow trackbars") {
  cv::namedWindow(windowName);
  cv::createTrackbar("Hue threshold", windowName, &hueThreshold, 200);
  cv::createTrackbar("Saturation threshold", windowName, &saturationThreshold, 200);
  cv::createTrackbar("Lower Value Threshold", windowName, &lowValueThreshold, 200);
  cv::createTrackbar("Upper Value Threshold", windowName, &upValueThreshold, 200);
  cv::createTrackbar("Dilation", windowName, &dilationShadow, 5);  
}

int thresholdValue = 125;
int minAreaValue = 150;
int maxAreaValue = 200;
int minDistValue = 30;
int minRadiusValue = 8;
int maxRadiusValue = 29; //35

void showCircleTrackbars(std::string windowName = "Circle trackbars") {
  cv::namedWindow(windowName);
  cv::createTrackbar("Threshold", windowName, &thresholdValue, 200);
  cv::createTrackbar("Min area", windowName, &minAreaValue, 200);
  cv::createTrackbar("Max area", windowName, &maxAreaValue, 500);
  cv::createTrackbar("Min distance", windowName, &minDistValue, 100);
  cv::setTrackbarMin("Min distance", windowName, 1);
  cv::createTrackbar("Min radius", windowName, &minRadiusValue, 50);
  cv::createTrackbar("Max radius", windowName, &maxRadiusValue, 100);
}

/*bool operator<(cv::Point lhs, cv::Point rhs) {
  if (lhs.x < rhs.x && lhs.y < )
}*/

/*//a 4d point in the x,y,t,v space
struct Point4 {
  double x,y,t,v;
};*/

bool isInVector(int element, std::vector<int> vector) {
  for (auto i : vector) {
    if (element == i)
      return true;
  }
  return false;
}

int main() {
  int thresholdValue = 50;
  int dilationValue = 1;
  int consecutiveValue = 2;
  int minAreaValue = 700;

  const int maxFitLenght = 100;
  int fitLenght = 90;           //number of past position to calcualate the fit with (30 fps -> 3 seconds)

  cv::Mat background = getBackground("sample.mp4");
  //std::cout << "BG: " << background.size << ", " << background.channels() << '\n';
  cv::Mat grayBackground;
  cv::Mat hsvBackground;
  cv::cvtColor(background,grayBackground, cv::COLOR_BGR2GRAY);
  grayBackground.convertTo(grayBackground, CV_8U);
  cv::cvtColor(background, hsvBackground, cv::COLOR_BGR2HSV);
  //std::cout << "Gray BG: " << grayBackground.size << ", " << grayBackground.channels() << " (" << grayBackground.type() << ")\n";

  cv::VideoCapture cap("sample.mp4");

  showShadowTrackbars();
  showCircleTrackbars();

  cv::Mat lastFrames[consecutiveValue];
  int frameCount = 0;
  //cv::Point positions[fitNumber]; //store the position of the head to fit it
  
  int fitCount = 0;
  cv::Point3d positions[maxFitLenght];

  while (true) {
    cv::Mat frame;
    cap.read(frame);

    if (frame.empty()) {
      std::cout << "End of video\n";
      break;
    }

    cv::Mat origFrame = frame.clone();
    cv::Mat grayFrame;

    cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);                   //convert to grayscale
    cv::absdiff(grayFrame, grayBackground, frame);                        //subtract background from image
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

    std::vector<cv::Rect> rects;
    for (auto contour : contours) {
      //skip if the area is too little
      if (cv::contourArea(contour) < minAreaValue)
        continue;

      //create and draw a rectangle
      cv::Rect r = cv::boundingRect(contour);
      cv::rectangle(origFrame, r, {0,255,0}, 2);
      rects.push_back(r);
    }

    //find cirlces and draw them
    cv::medianBlur(grayFrame, grayFrame, 3);
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(grayFrame, circles, cv::HOUGH_GRADIENT, 1, minDistValue, 100, 30, minRadiusValue, maxRadiusValue);

    for (auto circle : circles) {
      cv::Point center(circle[0], circle[1]);
      bool inside = false;
      for (auto r : rects) {
        if (center.inside(r)) {
          inside = true;
          break;
        }
      }
      //skip if point is not inside a rect
      if (!inside)
        continue;
      
      //otherwise draw it and add to fit
      cv::circle(origFrame, center, 1, {255,0,255}, 3, cv::LINE_AA);
      cv::circle(origFrame, center, circle[2], {100,100,100}, 1, cv::LINE_AA);
    
      //push back position
      positions[fitCount % maxFitLenght] = {center.x, center.y, frameCount};
      fitCount++;
    }

    cv::Mat plotYX(origFrame.size(), origFrame.type(), cv::Scalar(0));
    cv::Mat plotXT(frame.cols, 1000, CV_8UC3);
    cv::Mat plotYT(frame.rows, 1000, CV_8UC3);
    
    /* Linear fit of: 
     * - trajectory (assumed linear)  (y = a + bx)      y(x)
     * - vx (assumed constant)        (x = x0 + vx *t)  x(t)
     * - vy (assumed constant)        (y = y0 + vy *t)  y(t)
     */
    for (auto point3 : positions) {
      cv::Scalar color;
      if (frameCount - point3.z > fitLenght)
        color = {100,100,100};
      else {
        int time = (int)(point3.z*4) % 765;
        if (time < 255)
          color = {255-time, time, 0};
        else if (time < 510)
          color = {0, 510-time, time-255};
        else
          color = {time-510, 0, 765-time};
      }
      cv::circle(plotYX, {point3.x, point3.y}, 1, color, 3);
      cv::circle(plotXT, {point3.z, point3.x}, 1, color, 3);
      cv::circle(plotYT, {point3.z, point3.y}, 1, color, 3);
    }
    
    int size = maxFitLenght;
    if (fitCount < maxFitLenght)
      size = fitCount;

    double sumX = 0;
    double sumY = 0;
    double sumT = 0;
    //double sumWeight = 0;
    int skipped = 0;
    for (auto i = 0; i < size; ++i) {
      // double weight = std::pow(2,point.z);
      // sumWeight += weight;

      //skip if frame is too old
      if (frameCount - positions[i].z > fitLenght) {
        skipped++;
        continue;
      }

      sumX += positions[i].x; //* weight;
      sumY += positions[i].y; //* weight;
      sumT += positions[i].z; //* weight;
    }
    double averageX = sumX / (size-skipped); //sumWeight;
    double averageY = sumY / (size-skipped);
    double averageT = sumT / (size-skipped);

    double numYX = 0;
    double numXT = 0;
    double numYT = 0;
    double denX = 0;
    double denT = 0;
    
    for (auto i = 0; i < size; ++i) {
      //skip if frame is too old
      if (frameCount - positions[i].z > fitLenght)
        continue;

      double xMinusAvarage = positions[i].x - averageX;
      double tMinusAvarage = positions[i].z - averageT;
      numYX += xMinusAvarage*(positions[i].y - averageT);
      numXT += tMinusAvarage*(positions[i].x - averageX);
      numYT += tMinusAvarage*(positions[i].y - averageY);
      denX += xMinusAvarage*xMinusAvarage;
      denT += tMinusAvarage*tMinusAvarage;
    }

    double b = numYX/denX;
    double a = averageY - b*averageX;
    double vx = numXT/denT;
    double x0 = averageX - vx*averageT;
    double vy = numYT/denT;
    double y0 = averageY - vy*averageT;

    /* Remove some data and calculate the fit again. We remove:
     * - the farthest point between 2 (or more) that have the same z (time) coordinate
     * - points too far from fit (not done yet)
     */
    
    std::vector<int> removed;
    for (auto i = 0; i < size; ++i) {
      if (frameCount - positions[i].z > fitLenght)
        continue;
      for (auto j = i+1; j < size; ++j) {
        if (frameCount - positions[j].z > fitLenght)
          continue;
        if (positions[i].z == positions[j].z) {
          double den = sqrt(1 + b*b);
          double distanceI = abs(positions[i].y - (b* positions[i].x + a)) / den;
          double distanceJ = abs(positions[j].y - (b* positions[j].x + a)) / den;
          
          //remove one point
          if (distanceI > distanceJ) {
            removed.push_back(i);
            break;
          }
          else
            removed.push_back(j);
        }
      }
    }

    //Redo the fit
    for (auto i = 0; i < size; ++i) {
      cv::Scalar color;
      if (isInVector(i,removed))
        color = {255,255,255};
      else if (frameCount - positions[i].z > fitLenght)
        color = {100,100,100};
      else {
        int time = (int)(positions[i].z*4) % 765;
        if (time < 255)
          color = {255-time, time, 0};
        else if (time < 510)
          color = {0, 510-time, time-255};
        else
          color = {time-510, 0, 765-time};
      }
      cv::circle(plotYX, {positions[i].x, positions[i].y}, 1, color, 3);
      cv::circle(plotXT, {positions[i].z, positions[i].x}, 1, color, 3);
      cv::circle(plotYT, {positions[i].z, positions[i].y}, 1, color, 3);
    }
    
    sumX = 0;
    sumY = 0;
    sumT = 0;
    skipped = 0;
    for (auto i = 0; i < size; ++i) {
      //skip if frame is too old
      if (frameCount - positions[i].z > fitLenght || positions[i].z == -1) {
        skipped++;
        continue;
      }

      sumX += positions[i].x;
      sumY += positions[i].y;
      sumT += positions[i].z;
    }
    averageX = sumX / (size-skipped);
    averageY = sumY / (size-skipped);
    averageT = sumT / (size-skipped);

    numYX = 0;
    numXT = 0;
    numYT = 0;
    denX = 0;
    denT = 0;
    
    for (auto i = 0; i < size; ++i) {
      //skip if frame is too old
      if (frameCount - positions[i].z > fitLenght || positions[i].z == -1)
        continue;

      double xMinusAvarage = positions[i].x - averageX;
      double tMinusAvarage = positions[i].z - averageT;
      numYX += xMinusAvarage*(positions[i].y - averageT);
      numXT += tMinusAvarage*(positions[i].x - averageX);
      numYT += tMinusAvarage*(positions[i].y - averageY);
      denX += xMinusAvarage*xMinusAvarage;
      denT += tMinusAvarage*tMinusAvarage;
    }

    b = numYX/denX;
    a = averageY - b*averageX;
    vx = numXT/denT;
    x0 = averageX - vx*averageT;
    vy = numYT/denT;
    y0 = averageY - vy*averageT;

    //std::cout << "vx = " << vx << ", x0 = " << x0 << "   \t   vy = " << vy << ", y0 = " << y0 << '\t' << "v = " << sqrt(abs(vx*vy)) << '\n';

    //draw fit lines
    cv::line(origFrame, {0,a}, {frame.cols, a+ b*frame.cols}, {0,0,255}, 2);
    cv::line(plotYX, {0,a}, {frame.cols, a+ b*frame.cols}, {0,0,255}, 2);
    cv::line(plotXT, {0,x0}, {1000, x0 + vx*1000}, {0,0,255}, 2);
    cv::line(plotYT, {0,y0}, {1000, y0 + vy*1000}, {0,0,255}, 2);

    cv::imshow("Result", origFrame);
    cv::imshow("Plot y(x)", plotYX);
    cv::imshow("Plot x(t)", plotXT);
    cv::imshow("Plot y(t)", plotYT);
    int k = cv::waitKey(100);
    if (k == 'q')
      break;
    if (k == 't') {
      showShadowTrackbars();
      showCircleTrackbars();
    }
  }

  return 0;
}