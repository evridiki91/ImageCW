/////////////////////////////////////////////////////////////////////////////
//
// COMS30121 - face.cpp
//
/////////////////////////////////////////////////////////////////////////////

// header inclusion
// header inclusion
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

Point(456,28),Point(584,178) //Dart 0
Point(201,144), Point(381,314) //Dart 1
Point(111,108), Point(181,178)	//Dart 2
Point(325,154),Point(388,212)//Dart 3
Point(195,104),Point(355,271)//Dart 4
Point(442,149),Point(512,232)//Dart 5
Point(214,121),Point(270,178)//Dart 6
Point(262,175),Point(346,303)//Dart 7
Point(74,261),Point(122,333)//Dart 8
Point(852,226),Point(947,328)//Dart 8
Point(226,65),Point(418,259)//Dart 9
Point(104,112),Point(176,200)//Dart 10
Point(590,136),Point(636,199)//Dart 10
Point(924,160),Point(947,207)//Dart 10
Point(179,112),Point(228,149)	//Dart 11
Point(166,86),Point(208,199)//Dart 12
Point(285,128),Point(391,235)//Dart 13
Point(134,114),Point(232,213)//Dart 14
Point(1002,109),Point(1100,204)//Dart 14
Point(166,70),Point(273,176)//Dart 15





void detectAndDisplay( Mat frame );

String cascade_name = "dartcascade/cascade.xml";
CascadeClassifier cascade;

int overlap(Rect a, Rect b){
  if ((a.x+a.width) <= b.x ||a.x >= ( b.x+b.width )|| a.y >= ( b.y+b.height )|| (a.y + a.height) <= b.y){
    return 0;
  }
  return 1;
}

Rect createRectFromDiagonal(Point a, Point b){
  return Rect(a.x,a.y,b.x-a.x,b.y-a.y);
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

/** @function main */
int main( int argc, const char** argv )
{
       // 1. Read Input Image
	Mat frame = imread(argv[1], CV_LOAD_IMAGE_COLOR);

	// 2. Load the Strong Classifier in a structure called `Cascade'
	if( !cascade.load( cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

	// 3. Detect darts and Display Result
	detectAndDisplay( frame );
	// 4. Save Result Image
	imwrite( "detected.jpg", frame );

	return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
	std::vector<Rect> darts;
	Mat frame_gray;

	// 1. Prepare Image by turning it into Grayscale and normalising lighting
	cvtColor( frame, frame_gray, CV_BGR2GRAY );
	equalizeHist( frame_gray, frame_gray );

	// 2. Perform Viola-Jones Object Detection
	cascade.detectMultiScale( frame_gray, darts, 1.1, 1, 0|CV_HAAR_SCALE_IMAGE, Size(50, 50), Size(500,500) );

	//counter for correctly recognised darts
	int counter = 0;
	//draw rectangle around detected darts
	for( int i = 0; i < darts.size(); i++ ){
		rectangle(frame, Point(darts[i].x, darts[i].y), Point(darts[i].x + darts[i].width, darts[i].y + darts[i].height), Scalar( 0, 255, 0 ), 2);
	}
	/*
	//variable for storing the true face coordinates
	vector<Rect> truedarts(box, box + sizeof(box)/sizeof(box[0]));

	for (int j = 0; j<truedarts.size(); j++ ){
		for( int i = 0; i < darts.size(); i++ ){
			//if they do not overlap then go to next iteration
			if (!overlap(truedarts[j],darts[i])){
				continue; }
			else {
				//getting the % of overlap
				double percentage = overlapRectanglePerc(truedarts[j],darts[i]);
				printf("percentage of overlap  %f%%\n",percentage );
				if (percentage > 70){
					//increment the counter for correctly recognised darts and go to next face
					counter++;
					break; }
			}
		}
	}
	printf("Found %d darts out of %d\n",counter,truedarts.size());
	*/
}
