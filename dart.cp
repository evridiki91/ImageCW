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
	//normalising gradients from 0 to 255 (1020 = sobel limit)
	convert(gradx,gradx_norm, -1020, 1020);
	convert(grady,grady_norm, -1020, 1020);
  imwrite("gradx.jpg",gradx_norm);
  imwrite("grady.jpg",grady_norm);

  //calculating the gradient magnitude
  gradient_magnitude(gradx,grady,mag);
	//normalising for visualisation
	int max = round(sqrt(1020*1020 + 1020*1020));
	convert(mag,mag_norm,0,max );
	imwrite("magnitude.jpg",mag_norm);

  //threshold the magnitude image using the thresh variable
  threshold(mag,thr, thresh, maxValue, THRESH_BINARY);
  imwrite("threshold.jpg",thr);

  //calculating the gradient direction
  gradient_direction(gradx,grady,dir);
	//normalising the direction for visualisation
	convert(dir,dir_norm,-CV_PI, CV_PI);
	imwrite("dir.jpg",dir_norm);
  writeToCSV("dir.csv",dir);

  /*Parameters for Hough transform*/
  // int maxr = round(hypot(mag.size[0], mag.size[1])/2); //hypotenous = diagonal of image
  int maxr = 110; //works better than above apparently
  int minr = 30;  //found through eyeballing.
	// int maxr = 100; //good for coin images
  // int minr = 15;  //good for coin images

  vector<Vec3f> circles,circles_concentric;
	vector<Point2f> potentialLines, potentialIntersectionLines;

	//Performing HT for circles
	// (threshold, direction, min radius, max radius, destination, neighbourhood size, threshold Hough)
	hough_circle(thr,dir, minr, maxr,circles,thr.size[0]/4,circleHoughthresh, 0 );

	printf("Hough circle found %d circles\n\n",circles.size());
	//stores if the circle has a concentric circle/lines intersecting
	int concentric_bool[circles.size()];
	int intersection_bool[circles.size()];
	//stores circles as rectangles for fair comparison
  std::vector<Rect> circle_rect;
  //Diagonals for creating rectangles
  Point diagonal_a,diagonal_b,diagonal_concentric_a,diagonal_concentric_b;
	//rectangle for the square inside circle
	Rect concentric_square;


  for( size_t i = 0; i < circles.size(); i++ ){
		intersection_bool[i] = 0;
  	// creating rectangles to represent the circles found and push them into vector
     Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
     int radius = cvRound(circles[i][2]);
     diagonal_a = Point(center.x - radius, center.y - radius);
     diagonal_b = Point(center.x + radius, center.y + radius);
     Rect rectangle_c = Rect(diagonal_a,diagonal_b);
     circle_rect.push_back(rectangle_c);

		 //creating a square that represents the inside of the circle
     // width of "concentric" square found using pythagorean circles
		 int width_2 = round( sqrt(2)*radius/2);
		 diagonal_concentric_a = Point(center.x - width_2, center.y - width_2);
		 diagonal_concentric_b = Point(center.x + width_2, center.y + width_2);
		 concentric_square = Rect(diagonal_concentric_a,diagonal_concentric_b);

		 //finding the new hough circle parameters
		 int concentric_minr = ceil(radius/4);
		 int concentric_maxr = ceil(radius*3.0/4.0);

		 //Call hough circle transform for concentric circles
		 printf("Hough for concentric \n" );

		 //call hough transform on portion defined by concentric_square with altered parameters
		 // to find circles with smaller radii
		 hough_circle( thr(concentric_square), dir(concentric_square),concentric_minr,concentric_maxr ,
		 circles_concentric, concentric_square.width/4, circleHoughthresh/2, 1 );


		 Mat thr_inters_lines;
		 threshold(mag,thr_inters_lines, thresh/1.25, maxValue, THRESH_BINARY);
		 imwrite("thr_inters_lines.jpg",thr_inters_lines);
		 //performing Hough transform for lines
		 // (threshold, destination, neighbourhood size, threshold for Hough)
		 hough_line(thr_inters_lines, potentialIntersectionLines,concentric_square.width/8,frame.size[0]/10);
		//drawing detected lines
		printf("Found %d lines inside concentric \n",potentialIntersectionLines.size());

		// for( size_t p = 0; p < potentialIntersectionLines.size(); p++ ){
		// 	 float rho = potentialIntersectionLines[p].x, theta = potentialIntersectionLines[p].y;
		// 	 Point pt1, pt2;
		// 	 double a = cosd(theta), b = sind(theta);
		// 	 pt1.x = 0;
		// 	 pt1.y = (rho - (pt1.x)*cosd(theta))/sind(theta);
		// 	 pt2.x = frame.size[1]; /*concentric_square.x + concentric_square.width;*/
		// 	 pt2.y = (rho - (pt2.x)*cosd(theta))/sind(theta);
		// 	 line( frame, pt1, pt2, Scalar(0,0,255), 1, CV_AA);
		// }

		Mat intersection_accumulator = Mat::zeros(thr.size[0],thr.size[1],CV_32FC1);
		for (int x0 = concentric_square.x; x0 < concentric_square.x+concentric_square.width; x0++){
		 for( size_t p = 0; p < potentialIntersectionLines.size(); p++ ){
			 float rho = potentialIntersectionLines[p].x, theta = potentialIntersectionLines[p].y;
			 float m = - (cosd(theta)/sind(theta));
			 float c = rho*(1/sind(theta));
			 int y0 =cvRound(c + m * x0);
			 for (int k = -1; k < 1; k++){
				 for (int l = -1; l < 1; l++){
						if (y0+l >= concentric_square.y  &&
								y0+l < concentric_square.y+concentric_square.height && x0+k >= concentric_square.x
								&& x0+k < concentric_square.x + concentric_square.width){
					 		intersection_accumulator.at<float>(x0+k,y0+l) += 1;
					}
				 	}
				}
		 	}
		}
		writeToCSV("intersection_accumulator.csv",intersection_accumulator);
		for (int x0 = concentric_square.x; x0 < concentric_square.x+concentric_square.width; x0++){
			for (int y0 = concentric_square.y; y0 < concentric_square.y+concentric_square.height; y0++){
				if (intersection_accumulator.at<float>(x0,y0)>=6) {
					int distance = cvRound(euclidean(Point(x0,y0),Point(center.x,center.y)) );
					printf("distance %d\n",distance);
					if (distance <= 15){
						intersection_bool[i] = 1;
					}
				}
			}
		}

		printf("Hough for concentric found %d concentric circles\n",circles_concentric.size() );
		if (circles_concentric.size() > 0){

			 //marking whether a circle has a concentric circle inside
			 for( size_t j = 0; j < circles_concentric.size(); j++ ) {
		      Point center_conc(cvRound(circles_concentric[j][0]+concentric_square.x), cvRound(circles_concentric[j][1])+concentric_square.y);
		      int radius_conc = cvRound(circles_concentric[j][2] );
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

  vector<Rect> final_rect;
	vector<Rect> filtered_darts;
	vector<int> area_vector;

	//if no VJ darts or circles were detected then return empty
  if ((darts.size() == 0) && (circles.size() == 0)) {

		hough_line(thr, potentialIntersectionLines,Houghthresh/0.7,maxr);
	 //drawing detected lines
	 printf("Found %d lines \n",potentialIntersectionLines.size());

	 Mat intersection_accumulator = Mat::zeros(thr.size[0],thr.size[1],CV_32FC1);
	 for (int x0 = 0; x0 < thr.size[1]; x0++){
		for( size_t p = 0; p < potentialIntersectionLines.size(); p++ ){
			float rho = potentialIntersectionLines[p].x, theta = potentialIntersectionLines[p].y;
			float m = - (cosd(theta)/sind(theta));
			float c = rho*(1/sind(theta));
			int y0 =cvRound(c + m * x0);
			for (int k = -1; k < 1; k++){
				for (int l = -1; l < 1; l++){
					 if (y0+l >= 0  &&
							 y0+l < thr.size[0] &&
							 x0+k >= 0&&
							 x0+k < thr.size[1]){
						 intersection_accumulator.at<float>(x0+k,y0+l) += 1;
					}
				 }
			 }
		 }
	 }

	 for (int x0 = 0; x0 < thr.size[1]; x0++){
		 for (int y0 = 0; y0 < thr.size[0]; y0++){
			 if (intersection_accumulator.at<float>(x0,y0)>=6) {
				 Rect correct = Rect(Point(x0-minr,y0-minr),Point(x0+minr,y0+minr));
						 final_rect.push_back(correct);
				}
			}
		}
  }

	//if no circles were detected return VJ darts
  else if (circles.size() == 0) {
    final_rect = darts;
  }

	//if no VJ darts were detected return circles
  else if (darts.size() == 0) {
		 for (int num = 0; num < circles.size(); num++){
			 if (concentric_bool[num] || intersection_bool[num]){
				 final_rect.push_back(circle_rect[num]);
			 }
		 }
  }

  else{
		int distance_array[darts.size()][circle_rect.size()];
		int overlap_array[darts.size()][circle_rect.size()];
		//arrays to store distance/overlap % between each dart and circle
		for( int c_i = 0; c_i < circle_rect.size(); c_i++ ){
			for (int d_i = 0; d_i < darts.size(); d_i++){
				Point center_circle(cvRound(circles[c_i][0]),cvRound(circles[c_i][1]));
				Point center_dart(cvRound(darts[d_i].x+darts[d_i].width/2), cvRound(darts[d_i].y+darts[d_i].height/2));

				distance_array[d_i][c_i] = cvRound(euclidean(center_dart,center_circle));
				int overlap_perc = 0;
				if (overlap(darts[d_i],circle_rect[c_i])){
					overlap_perc = overlapRectanglePerc(darts[d_i],circle_rect[c_i]);
				}
				overlap_array[d_i][c_i] = overlap_perc;
				printf("VJ %d: x %d y %d \n",d_i,center_dart.x,center_dart.y);
				printf("Circle %d: x %d y %d \n",c_i,center_circle.x,center_circle.y);

				printf("distance of VJ %d and HT %d is %d\n",d_i,c_i, distance_array[d_i][c_i] );
				printf("overlap of VJ %d and HT %d is %d\n\n",d_i,c_i,overlap_array[d_i][c_i] );
			}
		}

		for( int c_i = 0; c_i < circle_rect.size(); c_i++ ){
			Rect correct;
			for (int d_i = 0; d_i < darts.size(); d_i++){

				//&& sizeBetween(2.5,darts[j],circle_rect[i])
				if (concentric_bool[c_i] && intersection_bool[c_i] ){
					if (distance_array[d_i][c_i] < 40 && overlap_array[d_i][c_i] > 0){
						printf("\ngood dart and concentric circle found\n");
						printf("distance of VJ %d and HT %d is %d\n",d_i,c_i, distance_array[d_i][c_i] );
						printf("overlap of VJ %d and HT %d is %d\n\n",d_i,c_i,overlap_array[d_i][c_i] );
						filtered_darts.push_back(darts[d_i]);
					}
				}
				else{
        if ((overlap_array[d_i][c_i] > 50 || (distance_array[d_i][c_i] < 40)) && intersection_bool[c_i]  ){
					//its a good circle and a good dart
					printf("\ngood dart and circle found\n");
					printf("distance of VJ %d and HT %d is %d\n",d_i,c_i, distance_array[d_i][c_i] );
					printf("overlap of VJ %d and HT %d is %d\n\n",d_i,c_i,overlap_array[d_i][c_i] );
					filtered_darts.push_back(darts[d_i]);
				}
			}
		}
		if (filtered_darts.size() > 0) {
			for (size_t f_i = 0; f_i < filtered_darts.size(); f_i++){
					area_vector.push_back(abs(filtered_darts[f_i].area() - circle_rect[c_i].area()) );
				}
			int index = minIndex(area_vector);
			if (concentric_bool[c_i]){

				correct = Rect(
					Point((0.25*filtered_darts[index].x+ 0.75*circle_rect[c_i].x),(0.25*filtered_darts[index].y+0.75*circle_rect[c_i].y)),
					Point(((filtered_darts[index].x + filtered_darts[index].width)*0.25+ (circle_rect[c_i].x + circle_rect[c_i].width )*0.75),
					((filtered_darts[index].y + filtered_darts[index].height)*0.25 + (circle_rect[c_i].y + circle_rect[c_i].height ) *0.75)));
			}
			else{
       correct = Rect(
        Point((filtered_darts[index].x+circle_rect[c_i].x)/2,(filtered_darts[index].y+circle_rect[c_i].y)/2),
        Point(((filtered_darts[index].x + filtered_darts[index].width)+ (circle_rect[c_i].x + circle_rect[c_i].width ))/2,
        ((filtered_darts[index].y + filtered_darts[index].height)+ (circle_rect[c_i].y + circle_rect[c_i].height ))/2));
			}
			final_rect.push_back(correct);
			printf("pushing correct\n");
			filtered_darts.clear();
			area_vector.clear();
	 }
	}

	if (final_rect.size() == 0){
		if (darts.size() == 0){
			for (int i = 0; i < circles.size(); i++){
				if (intersection_bool[i] || concentric_bool[i]){
						final_rect.push_back(circle_rect[i]);
				}
			}
		}
	}
	if (final_rect.size() == 0){

	}

	}
  printf("final rect size  %d\n",final_rect.size() );
  printf("circle rect size  %d\n",circle_rect.size() );
  printf("VJ rect size  %d\n",darts.size() );

	//counter for correctly recognised darts
	int counter = 0;

	for( size_t i = 0; i < circle_rect.size(); i++ ){
		rectangle(frame, Point(circle_rect[i].x, circle_rect[i].y), Point(circle_rect[i].x + circle_rect[i].width, circle_rect[i].y + circle_rect[i].height), Scalar( 255, 255, 255 ), 2);
	}
  for( size_t i = 0; i < darts.size(); i++ ){
    rectangle(frame, Point(darts[i].x, darts[i].y), Point(darts[i].x + darts[i].width, darts[i].y + darts[i].height), Scalar( 255, 0, 255 ), 2);
  }
	// draw rectangle around detected darts
	for( size_t i = 0; i < final_rect.size(); i++ ){
		rectangle(frame, Point(final_rect[i].x, final_rect[i].y), Point(final_rect[i].x + final_rect[i].width, final_rect[i].y + final_rect[i].height), Scalar( 0, 255, 255 ), 2);
	}

	//variable for storing the true face coordinates
	vector<Rect> truedarts(box, box + sizeof(box)/sizeof(box[0]));
	for (size_t td = 0; td<truedarts.size(); td++ ){
		rectangle(frame, Point(truedarts[td].x, truedarts[td].y), Point(truedarts[td].x + truedarts[td].width, truedarts[td].y + truedarts[td].height), Scalar( 255, 0, 0 ), 2);
		for( size_t fr = 0; fr < final_rect.size(); fr++ ){
			//if they do not overlap then go to next iteration
			if (!overlap(truedarts[td],final_rect[fr])){
				continue; }
			else {
				//getting the % of overlap
				float percentage = overlapRectanglePerc(truedarts[td],final_rect[fr]);
				printf("percentage of overlap  %f%%\n",percentage );
				if (percentage >= 60){
          if (final_rect[fr].area() <= 2*truedarts[td].area() ){
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
