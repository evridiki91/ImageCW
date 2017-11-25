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
Rect box[] = {Rect(456,28,128,150)}; //dart 0
// Rect box[] = {Rect(201,144,180,170)}; //dart 1
// Rect box[] = {Rect(111,108,70,70)};	//dart 2
// Rect box[] = {Rect(325,154,63,58)};	//dart 3
// Rect box[] = {Rect(195,104,137,167)}; //dart 4
// Rect box[] = {Rect(442,149,70,83)};//dart 5
// Rect box[] = {Rect(214,121,56,57)};//dart 6
// Rect box[] = {Rect(262,175,84,128)};//dart 7
// Rect box[] = {Rect(74,261,48,72),Rect(852,226,95,102)};//dart 8
// Rect box[] = {Rect(226,65,192,194)};//dart 9
// Rect box[] = {Rect(104,112,72,88),Rect(590,136,46,63),Rect(924,160,23,47)};	//dart 10
// Rect box[] = {Rect(185,114,39,37)};//dart 11
// Rect box[] = {Rect(166,86,42,113)};//dart 12
// Rect box[] = {Rect(285,128,106,107)};//dart 13
// Rect box[] = {Rect(134,114,98,99),Rect(1002,109,98,95)};//dart 14
// Rect box[] = {Rect(166,70,107,106)};//dart 15

/***********************************************************************
thresh = value for tresholding function, anything above this
  will become 255, else it will become 0
***********************************************************************/
float scaleFactorSobel = 1.0;
float thresh = 40;
float maxValue = 255;
String cascade_name = "dartcascade/cascade.xml";
CascadeClassifier cascade;
void detectAndDisplay( Mat frame );
