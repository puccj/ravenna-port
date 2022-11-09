/*///////////////////////////////////////////////////////////////////////
 * 
 * This file was just used to generate demonstrative videos of the code
 * that I used both in presentation and in the thesis
 * 
 * ////////////////////////////////////////////////////////////////////*/

#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

int main() {
  cv::VideoCapture cap("sample.mp4");
  cv::Mat bgrBG = cv::imread("background.png");
  cv::Mat hsvBG;
  cv::cvtColor(bgrBG,hsvBG,cv::COLOR_BGR2HSV);
  cv::Mat bg;
  cv::cvtColor(bgrBG,bg,cv::COLOR_BGR2GRAY);
  
  int frameCount = 0;
  int fitCount = 0;
  int maxFitLenght = 100;
  cv::Point3i headPositions[100];   //store last positions of the head to perform fit
  int fitLenght = 3;
  int fps = 20;

  cv::VideoWriter out("output.mp4",cv::VideoWriter::fourcc('M','P','4','V'),fps, {cap.get(cv::CAP_PROP_FRAME_WIDTH),cap.get(cv::CAP_PROP_FRAME_HEIGHT)});

  while(true) {
    cv::Mat bgrFrame;
    cap.read(bgrFrame);

    if (bgrFrame.empty()) 
      break;

    cv::Mat lastFrame = bgrFrame.clone();

    cv::Mat hsvFrame;
    cv::cvtColor(bgrFrame,hsvFrame,cv::COLOR_BGR2HSV);
  
    cv::Mat frame;
    cv::cvtColor(bgrFrame,frame,cv::COLOR_BGR2GRAY);
    
    cv::absdiff(frame,bg,frame);
    cv::threshold(frame, frame, 50, 255, cv::THRESH_BINARY);  //thresholding to convert to binary
    cv::dilate(frame, frame, cv::Mat(), cv::Point(-1,-1), 1); //dilate the image (no inside dark regions)

    std::vector<cv::Point> whitePoints; //store white points in sum
    cv::findNonZero(frame, whitePoints);

    cv::Mat shadow(hsvFrame.size(), CV_8UC1, cv::Scalar(0)); //store pixels considered to be shadow
    for (auto point : whitePoints) {
      //color is stored in a Vec3b. val[0]=Hue, val[1]=Saturation, val[2]=Value
      //a pixel is a shadow if the color has approxiamtetly the same hue and sat, but lower value than the bg
      cv::Vec3b frameColor = hsvFrame.at<cv::Vec3b>(point);
      cv::Vec3b bgColor = hsvBG.at<cv::Vec3b>(point);
      if (std::abs(frameColor.val[0] - bgColor.val[0]) < 48
                && frameColor.val[1] - bgColor.val[1] < 64 
                && frameColor.val[2] * 1.0 / bgColor.val[2] > 48/100.0 
                && frameColor.val[2] * 1.0 / bgColor.val[2] < 150/100.0) {
        shadow.at<uchar>(point) = 255;
      }
    }
    cv::dilate(shadow, shadow, cv::Mat(), cv::Point(-1,-1), 1);

    //remove shadows from detected image
    frame -= shadow;

    //find contours [(and draw them)]
    std::vector<cv::Mat> contours;
    cv::findContours(frame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  


    //HEAD
    int thresholdValue = 125;
    int minAreaValue = 150;
    int maxAreaValue = 200;

    int minDistValue = 30;    //20
    int minRadiusValue = 8;   //5
    int maxRadiusValue = 29;  //29

    cv::Mat threshFrame;
    cv::cvtColor(bgrFrame, threshFrame, cv::COLOR_BGR2GRAY);
    cv::medianBlur(threshFrame,threshFrame, 3);
    
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(threshFrame, circles, cv::HOUGH_GRADIENT, 1, minDistValue, 100, 30, minRadiusValue, maxRadiusValue);


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
        cv::rectangle(bgrFrame, r, {0,255,0}, 2);

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
      headPositions[fitCount % 100] = {center.x, center.y, frameCount};
      fitCount++;


      //draw the circle
        cv::circle(bgrFrame, center, 1, {255,0,255}, 3, cv::LINE_AA);
    }
    
    frameCount++;

    /* Linear fit of: 
   * - trajectory (assumed linear)  (y = a + bx)      y(x)
   * - vx (assumed constant)        (x = x0 + vx *t)  x(t)
   * - vy (assumed constant)        (y = y0 + vy *t)  y(t)
   */
  
  int size = maxFitLenght;
  if (fitCount < maxFitLenght)
    size = fitCount;

  //convert seconds in number of frames
  int framesForFit = fitLenght * fps;

  double sumX = 0;
  double sumY = 0;
  double sumT = 0;
  int skipped = 0;
  for (auto i = 0; i < size; ++i) {
    //skip if frame is too old
    if (frameCount - headPositions[i].z > framesForFit) {
      skipped++;
      continue;
    }

    sumX += headPositions[i].x;
    sumY += headPositions[i].y;
    sumT += headPositions[i].z;
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
    if (frameCount - headPositions[i].z > framesForFit)
      continue;

    double xMinusAvarage = headPositions[i].x - averageX;
    double tMinusAvarage = headPositions[i].z - averageT;
    numYX += xMinusAvarage*(headPositions[i].y - averageT);
    numXT += tMinusAvarage*(headPositions[i].x - averageX);
    numYT += tMinusAvarage*(headPositions[i].y - averageY);
    denX += xMinusAvarage*xMinusAvarage;
    denT += tMinusAvarage*tMinusAvarage;
  }

  double b = numYX/denX;
  double a = averageY - b*averageX;
  double vx = numXT/denT;
  double x0 = averageX - vx*averageT;
  double vy = numYT/denT;
  double y0 = averageY - vy*averageT;

  int cols = lastFrame.cols;
  // int a = result.a;
  // int b = result.b;
  cv::line(lastFrame, {0,a}, {cols, a + b*cols}, {0,0,255}, 2);

  double time = frameCount+1; //+2 to take extreme pos of head

  //calculate absolut position
  double x = x0 + vx*time;
  double y = y0 + vy*time;

  cv::circle(lastFrame,{x,y},1,{0,255,0},3,cv::LINE_AA);


  cv::imshow("output",lastFrame);
  lastFrame.convertTo(lastFrame,CV_8U);
  out.write(lastFrame);

  int k = cv::waitKey(1);
  if (k == 'q')
    break;

  }
  out.release();

  return 0;
}