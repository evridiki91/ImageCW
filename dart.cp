#include "dart.hpp"
/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
  //initialisations
	std::vector<Rect> darts;
	Mat frame_gray, blurred;
  Mat gradx = Mat::zeros(frame.rows, frame.cols,CV_64FC1);
  Mat grady = Mat::zeros(frame.rows, frame.cols,CV_64FC1);
	Mat gradx_norm = Mat::zeros(frame.rows, frame.cols,CV_8UC1);
	Mat grady_norm = Mat::zeros(frame.rows, frame.cols,CV_8UC1);
	Mat mag = Mat::zeros(frame.rows, frame.cols,CV_64FC1);
	Mat mag_norm = Mat::zeros(frame.rows, frame.cols,CV_8UC1);
  Mat thr = Mat::zeros(frame.rows, frame.cols,CV_8UC1);
	Mat dir_norm = Mat::zeros(frame.rows, frame.cols,CV_8UC1);
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
	convert(gradx,gradx_norm, -1020, 1020);
	convert(grady,grady_norm, -1020, 1020);

  writeToCSV("gradx.csv",gradx);
  writeToCSV("grady.csv",grady);
  imwrite("gradx.jpg",gradx_norm);
  imwrite("grady.jpg",grady_norm);

  //calculating the gradient magnitude
  gradient_magnitude(gradx,grady,mag);
	int max = round(sqrt(1020*1020 + 1020*1020));
	convert(mag,mag_norm,0,max );
	imwrite("magnitude.jpg",mag_norm);
  //threshold the magnitude image using the thresh variable
  threshold(mag,thr, thresh, maxValue, THRESH_BINARY);
  imwrite("threshold.jpg",thr);
  //calculating the gradient direction
  gradient_direction(gradx,grady,dir);
	convert(dir,dir_norm,-CV_PI, CV_PI);
	imwrite("dir.jpg",dir_norm);
  writeToCSV("dir.csv",dir);

  /*Parameters for Hough transform*/
  // int maxr = round(hypot(mag.size[0], mag.size[1])/5); //hypotenous = diagonal of image
  int maxr = 170; //works better than above apparently
  int minr = 40;  //found through eyeballing.
	// int maxr = 100; //works better than above apparently
  // int minr = 15;  //found through eyeballing.

  vector<Vec3f> circles,circles_concentric,circles_test;
	vector<Point2f> potentialLines;

	hough_line(thr, dir, potentialLines,thr.size[0]/8);

	printf("Hough line found %d lines\n\n",potentialLines.size());

	hough_circle(thr,dir, minr, maxr,circles,thr.size[0]/4,circleHoughthresh );

	for (size_t i = 0; i < circles.size(); i++)
		circle(frame, Point(circles[i][0],circles[i][1]), circles[i][2], Scalar(255,255,255),3);

	printf("Hough circle found %d circles\n\n",circles.size());

	//stores if the circle has a concentric circle inside
	int concentric_bool[circles.size()];

	//stores circles as rectangles for fair comparison
  std::vector<Rect> circle_rect;
  Point diagonal_a,diagonal_b,diagonal_concentric_a,diagonal_concentric_b;
  Rect concentric;

  for( size_t i = 0; i < circles.size(); i++ ){
  //   // creating rectangles to represent the circles found
     Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
     int radius = cvRound(circles[i][2]);
     diagonal_a = Point(center.x - radius, center.y - radius);
     diagonal_b = Point(center.x + radius, center.y + radius);
     Rect rectangle_c = Rect(diagonal_a,diagonal_b);
     circle_rect.push_back(rectangle_c);

		 int width_2 = round( sqrt(2)*radius/2);
		 diagonal_concentric_a = Point(center.x - width_2, center.y - width_2);
		 diagonal_concentric_b = Point(center.x + width_2, center.y + width_2);
		 concentric = Rect(diagonal_concentric_a,diagonal_concentric_b);
		 int concentric_minr = ceil(radius/4);
		 int concentric_maxr = ceil(radius*3.0/4.0);

		 printf("Hough for concentric \n" );
		 hough_circle( thr(concentric), dir(concentric),concentric_minr,concentric_maxr ,
		 circles_concentric, concentric.width/6, circleHoughthresh/2  );

		 printf("Hough for concentric found %d concentric circles\n",circles_concentric.size() );
		 if (circles_concentric.size() > 0){
			 for( size_t j = 0; j < circles_concentric.size(); j++ ) {
		      Point center_conc(cvRound(circles_concentric[i][0]+concentric.x), cvRound(circles_concentric[i][1])+concentric.y);
		      int radius_conc = cvRound(circles_concentric[i][2] );
					printf("drawing circle with x %d y %d r %d\n",center_conc.x,center_conc.y,radius_conc);
					circle(frame, center_conc, radius_conc, Scalar(255,0,0),2);
		      if (radius <= 0) {
						concentric_bool[i] = 0;
					}
					else{
						//make sure that the circle found is concentric by comparing the 2 centers(+/-)
						if (euclidean(center,center_conc) < 15){
							concentric_bool[i] = 1;
							printf("found concentric\n");
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

  // vector<Rect> final_rect;
	// vector<Rect> filtered_darts;
	// vector<int> area_vector;
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
	// 	//storing whether a circle was pushed into the final detections due to concentri circle detection
	// 	int removed[circle_rect.size()];
	// 	int distance_array[darts.size()][circle_rect.size()];
	// 	int overlap_array[darts.size()][circle_rect.size()];
  //
	// 	for( size_t i = 0; i < circle_rect.size(); i++ ){
	// 		removed[i] = 0;
	// 			if (concentric_bool[i]) {
	// 				final_rect.push_back(circle_rect[i]);
	// 				removed[i] = 1;
	// 			}
	// 	}
  //
	// 	//arrays to store distance/overlap % between each dart and circle
	// 	for( size_t i = 0; i < circle_rect.size(); i++ ){
	// 		Point center_circle(round(circle_rect[i].x+circle_rect[i].width/2),round(circle_rect[i].y+circle_rect[i].height/2));
	// 		for (size_t j = 0; j < darts.size(); j++){
	// 			Point center_dart(round(darts[j].x+darts[j].width/2), round(darts[j].y+darts[j].height/2));
	// 			distance_array[i][j] = round(euclidean(center_dart,center_circle));
  //       printf("distance of VJ %d and HT %d is %d\n",j,i, distance_array[i][j] );
  //
	// 			float overlap_perc;
	// 			if (!overlap(darts[j],circle_rect[i])) overlap_perc = 0;
	// 			else overlap_perc = overlapRectanglePerc(darts[j],circle_rect[i]);
	// 			overlap_array[i][j] = round(overlap_perc);
  //       printf("overlap of VJ %d and HT %d is %d\n\n",j,i,overlap_array[i][j] );
	// 		}
	// 	}
  //
	// 	for( size_t i = 0; i < circle_rect.size(); i++ ){
	// 		for (size_t j = 0; j < darts.size(); j++){
	// 			printf(" OV %d %d %d\n",j,i,overlap_array[i][j]);
	// 			printf(" D %d %d %d\n",j,i,distance_array[i][j]);
	// 		}
	// 	}
  //
	// 	for( size_t i = 0; i < circle_rect.size(); i++ ){
	// 		int found = 0;
	// 		if (removed[i]) {
	// 			printf("skipping i %d\n",i);
	// 			continue;
	// 		}
	// 		for (size_t j = 0; j < darts.size(); j++){
	// 			printf("HT %d VJ %d\n",i,j);
 	// 			printf("%d %d \n",overlap_array[i][j]);
	// 			//&& sizeBetween(2.5,darts[j],circle_rect[i])
  //       if (overlap_array[i][j] > 50 || (distance_array[i][j] < 40 )  ){
	// 				//its a good circle and a good dart
	// 				printf("good dart and circle found\n");
	// 				found = 1;
	// 				filtered_darts.push_back(darts[j]);
	// 			}
	// 		}
  //
	// 		if (found == 1) {
	// 			for (size_t it = 0; it < filtered_darts.size(); it++){
	// 				area_vector.push_back(filtered_darts[it].area());
	// 			}
  //
	// 			int index = minIndex(area_vector);
  //       Rect correct = Rect(
  //         Point((filtered_darts[index].x+circle_rect[i].x)/2,(filtered_darts[index].y+circle_rect[i].y)/2),
  //         Point(((filtered_darts[index].x + filtered_darts[index].width)+ (circle_rect[i].x + circle_rect[i].width ))/2,
  //         ((filtered_darts[index].y + filtered_darts[index].height)+ (circle_rect[i].y + circle_rect[i].height ))/2));
  //       final_rect.push_back(correct);
	// 		}
	// 	}
  // }
	// // HoughLinesP(thr, lines, 1, CV_PI/180, 70, minr, 0 );
	// //
	// // for( size_t i = 0; i < lines.size(); i++ )
	// // {
	// //     line( frame, Point(lines[i][0], lines[i][1]),
	// //         Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 2, 8 );
	// // }
  //
  // printf("final rect size  %d\n",final_rect.size() );
  // printf("circle rect size  %d\n",circle_rect.size() );
  // printf("VJ rect size  %d\n",darts.size() );
  //
	// //counter for correctly recognised darts
	// int counter = 0;
  //
  // for( size_t i = 0; i < filtered_darts.size(); i++ ){
	// 	rectangle(frame, Point(filtered_darts[i].x, filtered_darts[i].y), Point(filtered_darts[i].x + filtered_darts[i].width, filtered_darts[i].y + filtered_darts[i].height), Scalar( 0, 255, 0 ), 2);
	// }
  //
	for( size_t i = 0; i < circle_rect.size(); i++ ){
		rectangle(frame, Point(circle_rect[i].x, circle_rect[i].y), Point(circle_rect[i].x + circle_rect[i].width, circle_rect[i].y + circle_rect[i].height), Scalar( 255, 255, 255 ), 2);
	}
  //
  //
  // for( size_t i = 0; i < darts.size(); i++ ){
  //   rectangle(frame, Point(darts[i].x, darts[i].y), Point(darts[i].x + darts[i].width, darts[i].y + darts[i].height), Scalar( 255, 0, 255 ), 2);
  // }
	// printf("filtered_darts %d\n",filtered_darts.size());
	// // draw rectangle around detected darts
	// for( size_t i = 0; i < final_rect.size(); i++ ){
	// 	rectangle(frame, Point(final_rect[i].x, final_rect[i].y), Point(final_rect[i].x + final_rect[i].width, final_rect[i].y + final_rect[i].height), Scalar( 0, 255, 255 ), 2);
	// }
  //
	// //variable for storing the true face coordinates
	// vector<Rect> truedarts(box, box + sizeof(box)/sizeof(box[0]));
	// for (size_t j = 0; j<truedarts.size(); j++ ){
	// 	rectangle(frame, Point(truedarts[j].x, truedarts[j].y), Point(truedarts[j].x + truedarts[j].width, truedarts[j].y + truedarts[j].height), Scalar( 255, 0, 0 ), 2);
	// 	for( size_t i = 0; i < final_rect.size(); i++ ){
	// 		//if they do not overlap then go to next iteration
	// 		if (!overlap(truedarts[j],final_rect[i])){
	// 			continue; }
	// 		else {
	// 			//getting the % of overlap
	// 			float percentage = overlapRectanglePerc(truedarts[j],final_rect[i]);
	// 			printf("percentage of overlap  %f%%\n",percentage );
	// 			if (percentage > 65){
  //         if (final_rect[i].area() <= 1.5*truedarts[j].area() ){
  //   				//increment the counter for correctly recognised darts and go to next face
  //   				counter++;
  //   				break; }
  //         }
  //       }
  //     }
  //   }
  //   printf("Found %d darts out of %d\n",counter,truedarts.size());
  //   float f1 = f1score(final_rect.size(),truedarts.size(),counter);
  //   printf("f1score is %lf\n",f1);
  //   printf("size of final detections is  %d\n",final_rect.size());
}


int main( int argc, const char** argv ) {
	Mat frame = imread(argv[1], CV_LOAD_IMAGE_COLOR);

	if( !cascade.load( cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

	detectAndDisplay( frame );
	imwrite( "detected.jpg", frame );

	return 0;
}
