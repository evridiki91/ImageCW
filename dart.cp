#include "dart.hpp"
/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
  //initialisations
	std::vector<Rect> darts;
	Mat frame_gray, blurred;
  Mat gradx = Mat::zeros(frame.rows, frame.cols,CV_64FC1);
  Mat grady = Mat::zeros(frame.rows, frame.cols,CV_64FC1);
  Mat mag = Mat::zeros(frame.rows, frame.cols,CV_64FC1);
  Mat thr = Mat::zeros(frame.rows, frame.cols,CV_8UC1);
  Mat dir = Mat::zeros(frame.rows, frame.cols,CV_64FC1);

	// 1. Prepare Image by turning it into Grayscale and normalising lighting
	cvtColor( frame, frame_gray, CV_BGR2GRAY );
	equalizeHist( frame_gray, frame_gray );
  //applying gaussian blur to get rid of noise
  GaussianBlur( frame_gray, blurred , Size(3,3), 0, 0, BORDER_DEFAULT );
  imwrite("blurred.jpg",blurred);

  // calculating gradient for x and y
  //sobel(src,dest,depth,x-order,y-order,kernel size, scale,delta)
  Sobel( blurred, gradx, CV_64FC1, 1, 0, 3, scaleFactorSobel, 0, BORDER_DEFAULT);
  Sobel( blurred, grady, CV_64FC1, 0, 1, 3, scaleFactorSobel, 0, BORDER_DEFAULT);

  writeToCSV("gradx.csv",gradx);
  writeToCSV("grady.csv",grady);
  imwrite("gradx.jpg",gradx);
  imwrite("grady.jpg",grady);

  //calculating the gradient magnitude
  gradient_magnitude(gradx,grady,mag);
  imwrite("magnitude.jpg",mag);
  //threshold the magnitude image using the thresh variable
  threshold(mag,thr, thresh, maxValue, THRESH_BINARY);
  imwrite("threshold.jpg",thr);
  //calculating the gradient direction
  gradient_direction(gradx,grady,dir);
  writeToCSV("dir.csv",dir);

  /*Parameters for Hough transform*/
  // int maxr = round(hypot(mag.size[0], mag.size[1])/5); //hypotenous = diagonal of image
  int maxr = 170; //works better than above apparently
  int minr = 40;  //found through eyeballing.

  vector<Vec3f> circles,circles_concentric;
  //display hough transform
  hough_circle(thr,dir, minr, maxr );
  printf("hough transform done\n");
  writeToCSV("dir.csv",dir);

  // Apply the Hough Transform to find the circles
  //houghcircles(src,dest, method, accumulator resolution, min distance btw detected circles,
  // threshold for edge detection, accumulator threshold(votes), minr,maxr  )
  HoughCircles( blurred, circles, CV_HOUGH_GRADIENT, 1, blurred.rows/8, cannyThresh, Houghthresh, minr,maxr);
  drawCircles(circles, frame,0);

  printf("found %d\n",circles.size());

	int concentric_bool[circles.size()];
  std::vector<Rect> circle_rect;
  Point diagonal_a,diagonal_b,diagonal_concentric_a,diagonal_concentric_b;
  Rect concentric;
  for( size_t i = 0; i < circles.size(); i++ ){
    // creating rectangles to represent the circles found
     Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
     int radius = cvRound(circles[i][2]);
     diagonal_a = Point(center.x - radius, center.y - radius);
     diagonal_b = Point(center.x + radius, center.y + radius);
     Rect rectangle_c = Rect(diagonal_a,diagonal_b);
     circle_rect.push_back(rectangle_c);

     // finding whether a circle has a
		 int width_2 = round( sqrt(2)*radius/2);
		 diagonal_concentric_a = Point(center.x - width_2, center.y - width_2);
		 diagonal_concentric_b = Point(center.x + width_2, center.y + width_2);
		 concentric = Rect(diagonal_concentric_a,diagonal_concentric_b);
		 HoughCircles( blurred(concentric), circles_concentric, CV_HOUGH_GRADIENT, 1, concentric.width/8, cannyThresh/2, Houghthresh/2, radius/8,radius*3/4);
		 if (circles_concentric.size() > 0){
			 for( int j = 0; j < circles_concentric.size(); j++ ) {
		      Point center_conc(cvRound(circles_concentric[i][0]+concentric.x), cvRound(circles_concentric[i][1])+concentric.y);
		      int radius_conc = cvRound(circles_concentric[i][2]);
		      if (radius <= 0) {
						concentric_bool[i] = 0;
					}
					else{
						if (euclidean(center,center_conc) < 15){
							concentric_bool[i] = 1;
							printf("found concentric\n");
							circle( frame, center_conc, radius_conc, Scalar(255,0,0), 2, 8, 0 );
							break;
						}
						else concentric_bool[i] = 0;
					}
		    }
			}
			else {
				concentric_bool[i] = 0;
			}
   }


  // 2. Perform Viola-Jones Object Detection
	cascade.detectMultiScale( frame_gray, darts, 1.1, 1, 0|CV_HAAR_SCALE_IMAGE, Size(50, 50), Size(500,500) );
  // vector<Vec4i> lines;
  vector<Rect> final_rect, filtered_circles, filtered_darts;
  // 
  // if ((darts.size() == 0) && (circles.size() == 0)) {
  //   final_rect.clear();
  // }
  // else if (circles.size() == 0) {
  //   final_rect = darts;
  // }
  // else if (darts.size() == 0) {
  //   final_rect = circle_rect;
  // }
  // else{
  //
  //   for (int j = 0; j < darts.size(); j++){
  //     Point center_dart(darts[j].x+darts[j].width/2, darts[j].y+darts[j].height/2);
  //
  //     for( int i = 0; i < circle_rect.size(); i++ ){
	// 			float overlap_perc;
  //       Point center_circle(round(circle_rect[i].x+circle_rect[i].width/2),round(circle_rect[i].y+circle_rect[i].height/2));
	// 			float distance = euclidean(center_dart,center_circle);
  //       printf("distance of VJ %d and HT %d is %f\n\n",j,i,distance);
  //
	// 			if (!overlap(darts[j],circle_rect[i])) overlap_perc = 0;
	// 			else overlap_perc = overlapRectanglePerc(darts[j],circle_rect[i]);
  //       printf("overlap of VJ %d and HT %d is %f\n",j,i,overlap_perc);
  //
  //       if (overlap_perc > 20 && distance < 50 && sizeBetween(2.5,darts[j],circle_rect[i])){
	// 				//its a good circle and a good dart
	// 				filtered_circles.push_back(circle_rect[i]);
	// 			}
	// 		}
	// 		if (filtered_circles.size() > 0) filtered_darts.push_back(darts[j]);
	// 	}
	// 	Mat score_matrix = Mat::zeros(filtered_darts.size(), filtered_circles.size(),CV_32FC1);
	// 	for (int i = 0; i < filtered_darts.size(); i++){
	// 		for (int j = 0; j < filtered_circles.size(); j++){
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //
  //           Rect correct = Rect(
  //             Point((darts[j].x+circle_rect[i].x)/2,(darts[j].y+circle_rect[i].y)/2),
  //             Point(((darts[j].x + darts[j].width)+ (circle_rect[i].x + circle_rect[i].width ))/2,
  //             ((darts[j].y + darts[j].height)+ (circle_rect[i].y + circle_rect[i].height ))/2));
  //           final_rect.push_back(correct);
  //       }
  //     }
  //   }
	// HoughLinesP(thr, lines, 1, CV_PI/180, 70, minr, 0 );
	//
	// for( size_t i = 0; i < lines.size(); i++ )
	// {
	//     line( frame, Point(lines[i][0], lines[i][1]),
	//         Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 2, 8 );
	// }

  printf("final rect size  %d\n",final_rect.size() );
  printf("circle rect size  %d\n",circle_rect.size() );
  printf("VJ rect size  %d\n",darts.size() );

	//counter for correctly recognised darts
	int counter = 0;

  // for( size_t i = 0; i < circle_rect.size(); i++ ){
	// 	rectangle(frame, Point(circle_rect[i].x, circle_rect[i].y), Point(circle_rect[i].x + circle_rect[i].width, circle_rect[i].y + circle_rect[i].height), Scalar( 255, 255, 0 ), 2);
	// }

  for( size_t i = 0; i < darts.size(); i++ ){
    rectangle(frame, Point(darts[i].x, darts[i].y), Point(darts[i].x + darts[i].width, darts[i].y + darts[i].height), Scalar( 0, 255, 0 ), 2);
  }

	//draw rectangle around detected darts
	// for( size_t i = 0; i < final_rect.size(); i++ ){
	// 	rectangle(frame, Point(final_rect[i].x, final_rect[i].y), Point(final_rect[i].x + final_rect[i].width, final_rect[i].y + final_rect[i].height), Scalar( 0, 0, 255 ), 2);
	// }

	//variable for storing the true face coordinates
	vector<Rect> truedarts(box, box + sizeof(box)/sizeof(box[0]));
	for (size_t j = 0; j<truedarts.size(); j++ ){
		rectangle(frame, Point(truedarts[j].x, truedarts[j].y), Point(truedarts[j].x + truedarts[j].width, truedarts[j].y + truedarts[j].height), Scalar( 255, 0, 0 ), 2);
		for( size_t i = 0; i < final_rect.size(); i++ ){
			//if they do not overlap then go to next iteration
			if (!overlap(truedarts[j],final_rect[i])){
				continue; }
			else {
				//getting the % of overlap
				float percentage = overlapRectanglePerc(truedarts[j],final_rect[i]);
				printf("percentage of overlap  %f%%\n",percentage );
				if (percentage > 65){
          if (final_rect[i].area() <= 1.5*truedarts[j].area() ){
    				//increment the counter for correctly recognised darts and go to next face
    				counter++;
    				break; }
          }
        }
      }
    }
    printf("Found %d darts out of %d\n",counter,truedarts.size());
    float f1 = f1score(final_rect.size(),truedarts.size(),counter);
    printf("f1score is %lf\n",f1);
    printf("size of final detections is  %d\n",final_rect.size());

}


int main( int argc, const char** argv ) {
	Mat frame = imread(argv[1], CV_LOAD_IMAGE_COLOR);

	if( !cascade.load( cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

	detectAndDisplay( frame );
	imwrite( "detected.jpg", frame );

	return 0;
}
