#include "helper_functions.hpp"

/***********************************************************************
Function that calculates the hough transform for circles and displays
its Hough Space.
Inputs: Source = Magnitude matrix
        Dir    = Direction matrix
        minr,maxr = minimum and maximum radius
Process: Thresholds magnitude image according to the thresh variable defined
in constants.hpp. Iterates through all pixels where an edge has been detected
i.e. has value 255. Sets theta = direction matrix.(+-0.02). Calculates a,b
and increments the accumulator Acc[b,a,r].For visualisation purposes
all the r's are summed up in each dimension to reduce the 3d accumulator
to 2d
***********************************************************************/

void hough_circle(Mat thr, Mat dir, int minr, int maxr ){
  printf("inside hough\n");

  Mat acc2d = Mat::zeros(thr.size[0],thr.size[1],CV_64FC1);
  const int rows=thr.size[0], cols=thr.size[1], radii=maxr-minr;
  int dims[3] = {rows, cols, radii};
  cv::Mat acc3d = Mat::zeros(3, dims, CV_64FC1);
  //iterate through all pixels

  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      int pixel = thr.at<uchar>(y,x);
      // printf("pixel : %d",pixel);
      //if an edge has not been detected go to next pixel
      if ( pixel == 255) {
        //Use gradient direction information as theta (+-0.02)
        double t = dir.at<double>(y,x);
          //iterate through all posible radii
        for (int r = 0; r < radii; r++){
            int x0 = (x -(r+minr) * cos(t));
            int y0 = (y -(r+minr) * sin(t));
            // printf("i %d j %d k %d i0%d j0%d",y,x,r,y0,x0);
            if(x0 >= 0 && y0 >= 0 && x0 < cols && y0 <  rows) {
              // increment accumulator
              acc3d.at<double>( y0,x0,r) += 1;
            }
            x0 = (x + (r+minr) * cos(t));
            y0 = (y + (r+minr) * sin(t));
            // printf("i1%d j1%d\n", y0,x0);
            if(x0 >= 0 && y0 >= 0 && x0 < cols && y0 <  rows) {
              // increment accumulator
              acc3d.at<double>( y0,x0,r) += 1;
            }
          }
      }
    }
  }
  //accumulator 3d to 2d for visualisation purposes
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      for (int r = 0; r < maxr - minr ; r++){
        acc2d.at<double>(y,x) += acc3d.at<double>(y,x,r);
      }
    }
  }
  writeToCSV("hough_circle.csv",acc2d);
  double max,min;
  minMaxLoc(acc2d, &min,&max);
  convert(acc2d,acc2d,min,max);
  imwrite("hough_circle.jpg",acc2d);
}

void hough_circle2(Mat thr, int minr, int maxr ){
  printf("inside hough\n");

  Mat acc2d = Mat::zeros(thr.size[0],thr.size[1],CV_64FC1);
  const int rows=thr.size[0], cols=thr.size[1], radii=maxr-minr;
  int dims[3] = {rows, cols, radii};
  cv::Mat acc3d = Mat::zeros(3, dims, CV_64FC1);
  //iterate through all pixels

  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      int pixel = thr.at<uchar>(y,x);
      // printf("pixel : %d",pixel);
      //if an edge has not been detected go to next pixel
      if ( pixel == 255) {
        //Use gradient direction information as theta (+-0.02)
        for (double t = - M_PI; t < M_PI; t+= 0.1){
          //iterate through all posible radii
          for (int r = 0; r < radii; r++){
              int x0 = (x -(r+minr) * cos(t));
              int y0 = (y -(r+minr) * sin(t));
              // printf("i %d j %d k %d i0%d j0%d",y,x,r,y0,x0);
              if(x0 >= 0 && y0 >= 0 && x0 < cols && y0 <  rows) {
                // increment accumulator
                acc3d.at<double>( y0,x0,r) += 1;
              }
              x0 = (x + (r+minr) * cos(t));
              y0 = (y + (r+minr) * sin(t));
              // printf("i1%d j1%d\n", y0,x0);
              if(x0 >= 0 && y0 >= 0 && x0 < cols && y0 <  rows) {
                // increment accumulator
                acc3d.at<double>( y0,x0,r) += 1;
              }
            }
          }
      }
    }
  }
  //accumulator 3d to 2d for visualisation purposes
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      for (int r = 0; r < maxr - minr ; r++){
        acc2d.at<double>(y,x) += acc3d.at<double>(y,x,r);
      }
    }
  }
  writeToCSV("hough_circle2.csv",acc2d);
  log_mat(acc2d,acc2d);
  double max,min;
  minMaxLoc(acc2d, &min,&max);
  convert(acc2d,acc2d,min,max);
  imwrite("hough_circle2.jpg",acc2d);
}


/***********************************************************************
Function that calculates the gradient direction of an image using the
gradients in the x and y direction given by sobel. The result is stored in
dir.
***********************************************************************/

void gradient_direction(Mat &dx,Mat &dy, Mat &dir){
  printf("Directioning\n" );
  for (int y = 0; y < dx.size[0]; y++){
    for (int x = 0; x < dx.size[1]; x++){
      double pointx = dx.at<double>(y,x);
      double pointy = dy.at<double>(y,x);
      dir.at<double>(y,x) = atan2(pointy,pointx) ;
    }
  }
  printf("Finished Directioning\n" );
}

/***********************************************************************
Function that calculates the gradient magnitude of an image using the
gradients in the x and y direction given by sobel. The result is stored in
mag.
***********************************************************************/

void gradient_magnitude(Mat &dx,Mat &dy, Mat &mag){
  printf("Magnituding\n" );
  Mat mag_non_normalised = Mat::zeros(dx.rows,dx.cols,CV_64FC1);
  for (int y = 0; y < dx.size[0]; y++){
    for (int x = 0; x < dx.size[1]; x++){
      double pointx = dx.at<double>(y,x);
      double pointy = dy.at<double>(y,x);
      mag_non_normalised.at<double>(y,x) = sqrt(pointx*pointx +pointy*pointy) ;
    }
  }
  normalize(mag_non_normalised,mag,0, 255, NORM_MINMAX, CV_8UC1);
  printf("Finished Magnituding\n" );
}




void hough_line(Mat &thr, Mat &dir ){
  printf("inside hough");
  Mat acc2d = Mat::zeros(thr.size[0],thr.size[1],CV_64FC1);
  const int rows=thr.size[0], cols=thr.size[1];
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      int pixel = thr.at<uchar>(y,x);
      if ( pixel == 0) {
        continue;
      }
      else{
        float t = dir.at<float>(y,x);
        int r = round(x*cos(t) + y*sin(t));
        // if(r >= 0 && t >= 0 && t < cols && y0 <  rows) acc2d.at<float>(r,t) +=1;
        }
      }
    }
    double min,max;
    minMaxLoc(acc2d, &min,&max);
    convert(acc2d,acc2d,min,max);
    imwrite("hough_line.jpg",acc2d);
    // writeToCSV("hough_circlenorm.csv",acc2d);

  }

/***********************************************************************
Calculates the overlapping percentage of two rectangles.
It finds the rightmost and bottom coordinates of the overlap rectangle
by finding the maximum of the two rectangles respective coordinates.
We do the same for the y direction.
NOTE: Rect a must be the reference. The overlapping percentage will
be compared to this.
***********************************************************************/

float overlapRectanglePerc(Rect a, Rect b){
	//finding the intersecting left, top, right, bottom coordinates
	int left = max(a.x,b.x);
	int top = max(a.y,b.y);
	int right =  min(a.x+a.width,b.x+b.width);
	int bottom = min(a.y+a.height,b.y+b.height);
	int width = right - left;
	int height = bottom - top;
  printf(" l %d r %d t %d b %d w %d h %d ", left,right,top,bottom,width, height);
	float overlappingArea = width*height;
	float originalArea = (a.width)*(a.height);
	float perc = overlappingArea/originalArea;
	return perc*100;
}
