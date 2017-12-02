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
        double t = dir.at<double>(y,x) ;
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
  imwrite("hough_circle_norm.jpg",acc2d);
  double max,min;
  minMaxLoc(acc2d, &min,&max);
  convert(acc2d,acc2d,min,max);
  imwrite("hough_circle.jpg",acc2d);


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
      if (pointx == 0) dir.at<double>(y,x) = 0;
      else dir.at<double>(y,x) = atan(pointy/pointx) ;
    }
  }
  writeToCSV("dir.csv",dir);
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
  writeToCSV("mag_non_normalised.csv",mag_non_normalised);
  normalize(mag_non_normalised,mag,0, 255, NORM_MINMAX, CV_8UC1);
  printf("Finished Magnituding\n" );
}

void hough_line(Mat &thr, Mat &dir, vector<Point2f> &potentialLines, int distance_limit){
  const int rows=thr.size[0], cols=thr.size[1];
  printf("inside hough");
  int diagonal = floor(sqrt(rows*rows + cols*cols));
  int degree_range = 90;
  float resolution = 1;
  Mat acc2d = Mat::zeros(diagonal*2, degree_range*2/resolution,CV_64FC1);
  Mat acc2d_norm = Mat::zeros(diagonal*2, degree_range*2/resolution,CV_64FC1);
  for (int y = 0 ; y < rows; y++){
    for (int x = 0 ; x < cols; x++){
      int pixel = thr.at<uchar>(y,x);
      if ( pixel == 0) {
        continue;
      }
      else{
        for (float t = -90; t < 90; t+=resolution){
          int r = round(x*cosd(t) + y*sind(t));
          int t_index = (t+90)*(1/resolution) ;
          if(r >= -diagonal && t_index >=0 && t_index < degree_range*2/resolution && r <  diagonal) {
            acc2d.at<double>( r+diagonal,t_index) +=1;
          }
        }
      }
    }
  }
  writeToCSV("hough_line.csv",acc2d);
  printf("Finished hough");

  // log_mat(acc2d,acc2d_norm);
  // writeToCSV("houghline_log.csv",acc2d_norm);

  double max,min;
  minMaxLoc(acc2d, &min,&max);
  convert(acc2d,acc2d_norm,0,max);
  resize(acc2d_norm,acc2d_norm, Size(512, 512)) ;

  writeToCSV("hough_linenorm.csv",acc2d);
  printf("finding lcoal maxima\n");

  for (int r = -diagonal; r < diagonal; r++) {
    for (float t = -90 ; t <90; t+=resolution){
      int t_index = (t+90)*(1/resolution);
      int peak = acc2d.at<double>(r+diagonal,t_index);
      if (peak < Houghthresh) continue;
      else {
        int isMax = true;
        int isNew = true;
        Point2f point = Point(r+diagonal,t_index);
        int i;
        for ( i = 0; i < potentialLines.size(); i++){
          int t_potential = potentialLines[i].y;
          int r_potential = potentialLines[i].x;
          if ( peak > acc2d.at<double>(r_potential,t_potential)){
            isMax = true;
            if (euclidean(potentialLines[i],point) < distance_limit) isNew = false;
            else isNew = true;
            break;
          }
          else{
            isMax = false;
          }
        }
        if (isMax){
          if (potentialLines.size() == 0) potentialLines.push_back(point);
          else {
            if (isNew){
              potentialLines.erase(potentialLines.begin()+i);
              potentialLines.push_back(point);
            }
            else potentialLines.push_back(point);
          }
        }
      }
    }
  }
  printf("potential x %d y %d\n",potentialLines[0].x,potentialLines[0].y);

  for ( int index = 0; index < potentialLines.size(); index++){
    potentialLines[index].x -= diagonal;
    potentialLines[index].y = (potentialLines[index].y*resolution)-90;
  }
  imwrite("hough_line.jpg",acc2d_norm);
  printf(" Potential lines = %d\n",potentialLines.size());
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
