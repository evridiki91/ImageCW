#include "constants.hpp"

/***********************************************************************
Converting a matrix of float/signed/any type other than CV_8U, to CV_8U.
  Scales every value so that it ranges from 0 - 255.
***********************************************************************/
void convert(Mat &src,Mat &dst, float min, float max){
  src.convertTo(dst,CV_8U,255.0/(max-min),-255.0*min/(max-min));
}

/***********************************************************************
Calculates the Euclidean distance between two Points
***********************************************************************/

#define sind(x) (sin((x) * M_PI /180))
#define cosd(x) (cos((x) * M_PI /180))

float euclidean(Point a, Point b){
  return sqrt((b.y - a.y)*(b.y - a.y) + (b.x - a.x)*(b.x - a.x));
}

void drawCircles(vector<Vec3f> circles, Mat &frame, int color){
  for( int i = 0; i < circles.size(); i++ )
  {
    // finding circle center and circle radius
     Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
     int radius = cvRound(circles[i][2]);
     //draw circle
     if (color == 0) circle( frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
     if (color == 1) circle( frame, center, radius, Scalar(0,255,0), 3, 8, 0 );
     if (color == 2) circle( frame, center, radius, Scalar(255,0,0), 3, 8, 0 );
   }
}

/***********************************************************************
Helper function to check the output of each matrix in order to be sure
that everything works as expected. It was ussualy used for checking if the
conversion of matrices of float/double type were correctly rescaled to uchar
***********************************************************************/

void writeToCSV(string filename, Mat m)
{
   ofstream file;
   file.open(filename.c_str());
   file<< cv::format(m, cv::Formatter::FMT_CSV) << std::endl;
   file.close();
}

/***********************************************************************
Function that returns:
 0 if two rectangle do not overlap and
 1 if they overlap
It firstly checks overlapping in the x direction for both rectangles
and then in the y direction.
***********************************************************************/

int overlap(Rect a, Rect b){
  if ((a.x+a.width) <= b.x ||a.x >= ( b.x+b.width )
  || a.y >= ( b.y+b.height )|| (a.y + a.height) <= b.y){
    return 0;
  }
  return 1;
}

/***********************************************************************
Function that calculates the f1 score
Parameters:
  detected = number of elements that were detected, (TP+FP)
  trueDetected = no of elements that should have been correctly detected
  correct = no of elements that have been correctly detected
***********************************************************************/

double f1score(double detected, double trueDetected, double correct){
  double tp = correct;
  double fp = detected - tp;
  double fn = trueDetected - tp;
  double precision = tp/(tp + fp);
  double recall = tp/(tp + fn);

	printf("true positives %f\n",tp);
  printf("false positives %f\n",fp);
  printf("false negatives %f\n",fn);
  printf("Precision:  %f\n",precision);
  printf("Recall %f\n",recall);
	return 2*tp/(2*tp+fp+fn);
}

int minIndex(vector<int> &vec){
  int min = 99999;
  int index = -1;
  for (size_t i = 0; i < vec.size(); i++ ){
    if (vec[i] < min ){
      min = vec[i];
      index = i;
    }
  }
  return index;
}

void log_mat(Mat &src, Mat &dst){
  for (int i = 0; i < src.rows; i++){
    for (int j = 0; j < src.cols; j++){
      if (src.at<double>(i,j) == 0)   dst.at<double>(i,j) = 0;
      else dst.at<double>(i,j) = log(src.at<double>(i,j));
    }
  }
}

int sizeBetween(float perc, Rect a, Rect b){
  if (a.area() <= perc* b.area() || b.area() <= perc* a.area()) return true;
  else return false;
}
