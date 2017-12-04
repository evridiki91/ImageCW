#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <math.h>

using namespace std;
using namespace cv;

/***********************************************************************
Ground truth encoded as rectangles
  Rect(corner point x, corner point y, width,height )
  Uncomment and remake to run detector for a new image
************************************************************************/
// Rect box[] = {Rect(438,14,166,181)}; //dart 0
// Rect box[] = {Rect(185,127,211,205)}; //dart 1
// Rect box[] = {Rect(92,90,111,102)};	//dart 2
// Rect box[] = {Rect(325,154,63,58)};	//dart 3
// Rect box[] = {Rect(173,87,205,209)}; //dart 4
// Rect box[] = {Rect(429,136,108,115)};//dart 5
// Rect box[] = {Rect(205,112,74,71)};//dart 6
// Rect box[] = {Rect(250,162,148,159)};//dart 7
// Rect box[] = {Rect(62,249,72,94),Rect(835,214,131,131)};//dart 8
// Rect box[] = {Rect(197,39,246,251)};//dart 9
// Rect box[] = {Rect(85,101,108,115),Rect(580,125,65,90),Rect(912,147,43,68)};	//dart 10
// Rect box[] = {Rect(166,101,74,75)};//dart 11
// Rect box[] = {Rect(166,86,42,113)};//dart 12
// Rect box[] = {Rect(269,116,142,142)};//dart 13
// Rect box[] = {Rect(114,92,138,143),Rect(978,85,142,145)};//dart 14
Rect box[] = {Rect(145,53,143,145)};//dart 15

/***********************************************************************
thresh = value for tresholding function, anything above this
          will become maxValue, else it will become 0
Houghthresh, CircleHoughthresh = threshold for hough transform (lines and circles)

***********************************************************************/
float scaleFactorSobel = 1.0;
float thresh = 65;
float maxValue = 255;
float Houghthresh = 80;
float circleHoughthresh = 80;
String cascade_name = "dartcascade/cascade.xml";
CascadeClassifier cascade;
void detectAndDisplay( Mat frame );
