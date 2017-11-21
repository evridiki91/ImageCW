#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <math.h>

using namespace std;
using namespace cv;

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

float thresh = 80;
float maxValue = 255;

void convert(Mat &src,Mat &dst){
  double minVal, maxVal;
  minMaxLoc(src, &minVal, &maxVal); //find minimum and maximum intensities
  src.convertTo(dst, CV_8U, 255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));
  printf("Converting done\n");
}

void gradient_direction(Mat &dx,Mat &dy, Mat &dir){
  printf("Directioning\n" );
  dir.create(dx.size(), dx.type() );
  for (int y = 0; y < dx.size[0]; y++){
    for (int x = 0; x < dx.size[1]; x++){
      float pointx = dx.at<float>(y,x);
      float pointy = dy.at<float>(y,x);
      dir.at<float>(y,x) = atan2(pointy,pointx) ; // - (CV_PI/2)
    }
  }
  printf("Finished Directioning\n" );
}

void gradient_magnitude(Mat &dx,Mat &dy, Mat &mag){
  printf("Magnituding\n" );
  mag.create(dx.size(), dx.type() );
  for (int y = 0; y < dx.size[0]; y++){
    for (int x = 0; x < dx.size[1]; x++){
      float pointx = dx.at<float>(y,x);
      float pointy = dy.at<float>(y,x);
      mag.at<float>(y,x) = sqrt(pointx*pointx +pointy*pointy) ;
    }
  }
  printf("Finished Magnituding\n" );
}


float euclidean(Point a, Point b){
  return sqrt((b.y - a.y)*(b.y - a.y) + (b.x - a.x)*(b.x - a.x));
}

void writeToCSV(string filename, Mat m)
{
   ofstream file;
   file.open(filename.c_str());
   file<< cv::format(m, cv::Formatter::FMT_CSV) << std::endl;
   file.close();
}


Rect createRectFromDiagonal(Point a, Point b){
	// printf("Rect(%d,%d,%d,%d);\n", a.x,a.y,b.x-a.x,b.y-a.y);
  return Rect(a.x,a.y,b.x-a.x,b.y-a.y);
}

int overlap(Rect a, Rect b){
  if ((a.x+a.width) <= b.x ||a.x >= ( b.x+b.width )|| a.y >= ( b.y+b.height )|| (a.y + a.height) <= b.y){
    return 0;
  }
  return 1;
}

//detected = number of elements that were detected, (TP+FP)
//trueDetected = no of elements that should have been correctly detected
//correct = no of elements that have been correctly detected
double f1score(double detected, double trueDetected, double correct){
  double tp = correct;
  double fp = detected - tp;
  double fn = trueDetected - tp;

	printf("true positives %f\n",tp);
  printf("false positives %f\n",fp);
  printf("false negatives %f\n",fn);
	return 2*tp/(2*tp+fp+fn);
}



void hough_line(Mat &thr, Mat &dir, int hough_threshold ){
  printf("inside hough");
  Mat acc2d = Mat::zeros(thr.size[0],thr.size[1],CV_32F);
  const int rows=thr.size[0], cols=thr.size[1];
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      int pixel = thr.at<uchar>(y,x);
      if ( pixel == 0) {
        continue;
      }
      else{
        float t = dir.at<float>(y,x);
        for (float i = -0.1; i <= 1.1; i += 0.1){
          int r = round(x*cos(t) + y*sin(t));
          acc2d.at<float>(r,t) +=1;
        }
      }
    }
  }
  writeToCSV("hough_circle.csv",acc2d);
  convert(acc2d,acc2d);
  writeToCSV("hough_circlenorm.csv",acc2d);
  imwrite("hough_circle.jpg",acc2d);
}

//creating the overlapping rectangle
//NOTE: Rect a must be the reference
float overlapRectanglePerc(Rect a, Rect b){
	//finding the intersecting left, top, right, bottom coordinates
	int left = max(a.x,b.x);
	int top = max(a.y,b.y);
	int right =  min(a.x+a.width,b.x+b.width);
	int bottom = min(a.y+a.height,b.y+b.height);
	int width = right - left;
	int height = bottom - top;

	float overlappingArea = width*height;
	float originalArea = (a.width)*(a.height);
	float perc = overlappingArea/originalArea;
	return perc*100;
}
void detectAndDisplay( Mat frame );
String cascade_name = "dartcascade/cascade.xml";
CascadeClassifier cascade;
