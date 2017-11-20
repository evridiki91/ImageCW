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
int ddepth = CV_32F;
double thresh = 125;
double maxValue = 255;



void writeToCSV(string filename, Mat m)
{
   ofstream file;
   file.open(filename.c_str());
   file<< cv::format(m, cv::Formatter::FMT_CSV) << std::endl;
   file.close();
}


void createRectFromDiagonal(Point a, Point b){
	printf("Rect(%d,%d,%d,%d);\n", a.x,a.y,b.x-a.x,b.y-a.y);
}

int overlap(Rect a, Rect b){
  if ((a.x+a.width) <= b.x ||a.x >= ( b.x+b.width )|| a.y >= ( b.y+b.height )|| (a.y + a.height) <= b.y){
    return 0;
  }
  return 1;
}

double f1score(double detected, double trueDetected, double correct){
	double precision = correct/detected;
	printf("precision %f\n",precision);
	double recall = correct / trueDetected;
	printf("recall %f\n",recall);
	return 2*(precision*recall)/(precision + recall);
}

void hough_circle(Mat &thr, int minr, int maxr,Mat &dir){
  printf("inside hough");
  printf("MAXR %d minr %d",maxr,minr);
  Mat acc2d = Mat::zeros(thr.size[0],thr.size[1],CV_8UC1);
  const int rows=thr.size[0], cols=thr.size[1], radii=maxr-minr;
  int dims[3] = {rows, cols, radii};
  cv::Mat acc3d = Mat::zeros(3, dims, CV_16UC1);

  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      int pixel = thr.at<uchar>(y,x);
      if ( pixel == 0) {
        continue;
      }
      else{
        int t = dir.at<uchar>(y,x);
        for (int r = minr; r < maxr; r++){
            int a = int (x -r * cos(t ));
            int b = int (y +r * sin(t ));
            if(a >= 0 && b >= 0 && a < rows && b <  cols) {
              acc3d.at<uchar>(a,b,r-minr) += 1;
            }
            a = int (x +r * cos(t ));
            b = int (y +r * sin(t ));
            if(a >= 0 && b >= 0 && a < rows && b <  cols) {
              acc3d.at<uchar>(a,b,r-minr) += 1;
            }
        }
      }
    }
  }
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      for (int r = minr; r < maxr; r++){
        acc2d.at<uchar>(y,x) += acc3d.at<uchar>(y,x,r);
      }
    }
  }

  // for (int y = 0 ; y < rows; y++){
  //   for (int x = 0 ; x < cols; x++){
  //       acc2d.at<uchar>(y,x) = log(acc2d.at<uchar>(y,x));
  //     }
  //   }

  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
        acc2d.at<uchar>(y,x) = log(acc2d.at<uchar>(y,x));
      }
    }
  writeToCSV("hough.csv",acc2d);
  imwrite("output.jpg",acc2d);
}


void hough_line(Mat &thr,Mat &dir){
  printf("inside hough");
  Mat acc2d = Mat::zeros(thr.size[0],thr.size[1],CV_8UC1);
  const int rows=thr.size[0], cols=thr.size[1];
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      if ( thr.at<uchar>(y,x) != 255) {
        continue;
      }
      else{
          int t = dir.at<uchar>(y,x);
          int r = int(x * cos(t) - y * sin(t));
          if(r >= 0 && r < rows) {
            acc2d.at<uchar>(r,t) += 1;
          }
        }
      }
    }
    
    writeToCSV("line_hough.csv",acc2d);
    imwrite("hough_line.jpg",acc2d);
}

//creating the overlapping rectangle
//NOTE: Rect a must be the true face
double overlapRectanglePerc(Rect a, Rect b){
	//finding the intersecting left, top, right, bottom coordinates
	int left = max(a.x,b.x);
	int top = max(a.y,b.y);
	int right =  min(a.x+a.width,b.x+b.width);
	int bottom = min(a.y+a.height,b.y+b.height);
	int width = right - left;
	int height = bottom - top;

	double overlappingArea = width*height;
	double originalArea = (a.width)*(a.height);
	double perc = overlappingArea/originalArea;
	return perc*100;
}
void detectAndDisplay( Mat frame );
String cascade_name = "dartcascade/cascade.xml";
CascadeClassifier cascade;
