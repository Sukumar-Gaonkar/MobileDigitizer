#include <jni.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/nonfree/nonfree.hpp>

#include <time.h>
#include <typeinfo>
#include <android/log.h>
#include <vector>

using namespace std;
using namespace cv;

extern "C" {
JNIEXPORT void JNICALL Java_com_mobiledigitizer_ace_MainActivity_imageProcess(JNIEnv* env,jobject thisObj,jlong addrRgba,jlong addrGray,jlong addrTemplateMat,jlong addrTemplateKeypoints,jlong addrTemplateDescriptors,jlong addrOutput);
void Mat_to_vector_KeyPoint(Mat& mat, vector<KeyPoint>& v_kp);
bool comapreDMatch(DMatch match1,DMatch match2);

JNIEXPORT void JNICALL Java_com_mobiledigitizer_ace_MainActivity_imageProcess(JNIEnv* env,jobject thisObj,jlong addrRgba,jlong addrGray,jlong addrTemplateMat,jlong addrTemplateKeypoints,jlong addrTemplateDescriptors,jlong addrOutput){
//	__android_log_print(ANDROID_LOG_ERROR, "jni", "Entered Native Code2.0");

	clock_t tStart;

	std::vector< DMatch > matches;
	Mat grayCanny,grayCannyHolder;
	Mat& gray = *(Mat*) addrGray;
	Mat& output = *(Mat*) addrOutput;
	Mat& rgba = *(Mat*) addrRgba;
	Mat& templateKeypointMat = *(Mat*) addrTemplateKeypoints;
	Mat& templateDescriptors = *(Mat*) addrTemplateDescriptors;
	vector<KeyPoint> templateKeypoints;
	Mat& templateImgGray =  *(Mat*) addrTemplateMat;

//	Mat templateImgGray;
//	Mat& templateImg = *(Mat*) addrTemplateMat;
//	tStart = clock();
//	cvtColor(templateImg,templateImgGray,CV_BGR2GRAY);
//	__android_log_print(ANDROID_LOG_ERROR, "info", "template CV_BGR2GRAY - %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

//	gray = templateImgGray;

	Canny(gray,grayCanny,50,150);
//    dilate(grayCanny, grayCanny, Mat(), Point(-1, -1), 2, 1, 1);
//	grayCanny = gray;
    grayCannyHolder = grayCanny.clone();

	tStart = clock();
	Mat_to_vector_KeyPoint(templateKeypointMat,templateKeypoints);
//	__android_log_print(ANDROID_LOG_ERROR, "info", "MatofKeypoint to vector - %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

    SiftFeatureDetector detector;
//    OrbFeatureDetector detector;
//    OrbDescriptorExtractor descriptor;
//    FastFeatureDetector detector;
//    SurfFeatureDetector detector;
//    SiftDescriptorExtractor descriptor;
    SurfDescriptorExtractor descriptor;
//    BriefDescriptorExtractor descriptor;

	std::vector<KeyPoint> keypoints_1;
	Mat descriptors_1;
	FlannBasedMatcher matcher;

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	RNG rng(12345);
	findContours( grayCanny, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

	double maxBoundryArea = 0, doubleHolder;
	int formContourIndex = -1;
	Rect boundRect;

	if(contours.size() > 1) {
		vector <vector<Point> > contours_poly(contours.size());

		for (size_t i = 0; i < contours.size(); i++) {
			approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
			doubleHolder = contourArea(contours[i]);
			if (doubleHolder > maxBoundryArea) {
				maxBoundryArea = doubleHolder;
				formContourIndex = i;
			}
		}
		boundRect = boundingRect(Mat(contours[formContourIndex]));
		if(maxBoundryArea > 2) {
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			rectangle(gray, boundRect.tl(), boundRect.br(), color, 2, 8, 0);
		}
	}
	__android_log_print(ANDROID_LOG_ERROR, "info", "Contour count: %d  Area: %f",contours.size(),maxBoundryArea);

	if(contours.size() > 1)
        gray = gray(boundRect);

/*

	tStart = clock();
	detector.detect(gray,keypoints_1);
//	__android_log_print(ANDROID_LOG_ERROR, "info", "Keypoint detection time- %fs Keypoints- %d\n", (double)(clock() - tStart)/CLOCKS_PER_SEC,keypoints_1.size());
	__android_log_print(ANDROID_LOG_ERROR, "info", "Keypoints- %d\n",keypoints_1.size());

	tStart = clock();
	descriptor.compute(gray,keypoints_1,descriptors_1);
//	__android_log_print(ANDROID_LOG_ERROR, "info", "Keypoint description time- %fs Keypoints - %d\n", (double)(clock() - tStart)/CLOCKS_PER_SEC,keypoints_1.size());

	tStart = clock();
	if(descriptors_1.type()!=CV_32F) {
		descriptors_1.convertTo(descriptors_1, CV_32F);
//		__android_log_print(ANDROID_LOG_ERROR, "info", "Conversion1- %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

	}
	tStart = clock();
	if(templateDescriptors.type()!=CV_32F) {
		templateDescriptors.convertTo(templateDescriptors, CV_32F);
//		__android_log_print(ANDROID_LOG_ERROR, "info", "Conversion2- %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
	}

	tStart = clock();
	matcher.match( descriptors_1, templateDescriptors, matches );
//	__android_log_print(ANDROID_LOG_ERROR, "info", "Matcher- %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
	__android_log_print(ANDROID_LOG_ERROR, "info", "Matches Count : %d\n", matches.size());

    double max_dist = 0; double min_dist = 1000;
	DMatch match;
	int goodMatchCount = 0;
	double AvgError = 0;
	std::vector< DMatch > good_matches;

    sort(matches.begin(),matches.end(),comapreDMatch);
    for(int i=0;i<min(10,(int)matches.size());i++)
    {
        goodMatchCount = 10;
        match = matches[i];
        __android_log_print(ANDROID_LOG_ERROR, "info", "Distance : %f\n", match.distance);
        good_matches.push_back(match);
    }

	for( int i = 0; i < matches.size(); i++ )
	{
		match = matches[i];
		double dist = match.distance;
		if( dist < min_dist ) min_dist = dist;
		if( dist > max_dist ) max_dist = dist;
	}
	//-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
	//-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
	//-- small)
	//-- PS.- radiusMatch can also be used here.
	std::vector<Point2f> goodTemplateKeypoints;
    std::vector<Point2f> goodFrameKeypoints;
	KeyPoint keypointHolder;
	for( int i = 0; i < matches.size(); i++ )
	{
		match = matches[i];
		if( match.distance <= max(2*min_dist, 0.02) )
//		if( match.distance <= 100 )
		{
			goodMatchCount++;
			AvgError += match.distance;
			//good_matches.push_back(match);
			keypointHolder = keypoints_1[match.queryIdx];
			goodFrameKeypoints.push_back(keypointHolder.pt);
			keypointHolder = templateKeypoints[match.trainIdx];
			goodTemplateKeypoints.push_back(keypointHolder.pt);
		}
	}

	AvgError /= goodMatchCount;
	__android_log_print(ANDROID_LOG_ERROR, "info", "min - %f max - %f\n",min_dist,max_dist);

	__android_log_print(ANDROID_LOG_ERROR, "info", "Good Matches - %d Avg Error- %f\n",goodMatchCount, AvgError);
*/
 //	drawKeypoints( grayCanny, goodFrameKeypoints, grayCanny, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
//	drawKeypoints( templateImgGray, goodTemplateKeypoints, templateImgGray, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
//    gray = gray(boundRect);
//	drawMatches( gray, keypoints_1, templateImgGray, templateKeypoints,good_matches, output, Scalar::all(-1), Scalar::all(-1),vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

///*************************************************
    std::vector<Point> templateCorners(4);
    std::vector<Point> frameCorners = contours_poly[formContourIndex];
    templateCorners[0] = Point(0,0); templateCorners[1] = Point( templateImgGray.cols, 0 );
    templateCorners[2] = Point( templateImgGray.cols, templateImgGray.rows ); templateCorners[3] = Point( 0, templateImgGray.rows );
    __android_log_print(ANDROID_LOG_ERROR, "info", "template - %d frame- %d\n",templateCorners.size(), frameCorners.size());
    Mat H = findHomography( templateCorners, frameCorners, CV_RANSAC );
//	136,515 1410,526 1388,1165 121,1154
	std::vector<Point2f> boxOriginal(4);
	std::vector<Point2f> scene_corners(4);
	boxOriginal[0] = Point(515,136);
	boxOriginal[1] = Point(526,1410);
	boxOriginal[2] = Point(1165,1388);
	boxOriginal[3] = Point(1154,121);

	perspectiveTransform( boxOriginal, scene_corners, H);
	//-- Draw lines between the corners (the mapped object in the scene - image_2 )
	Point2f point0 = scene_corners[0];
	Point2f point1 = scene_corners[1];
	Point2f point2 = scene_corners[2];
	Point2f point3 = scene_corners[3];
	line( gray, point0 + Point2f( templateImgGray.cols, 0), point1 + Point2f( templateImgGray.cols, 0), Scalar(0, 255, 0),10,8,0);
	line( gray, point1 + Point2f( templateImgGray.cols, 0), point2 + Point2f( templateImgGray.cols, 0), Scalar( 0, 255, 0), 10 ,8,0);
	line( gray, point2 + Point2f( templateImgGray.cols, 0), point3 + Point2f( templateImgGray.cols, 0), Scalar( 0, 255, 0), 10,8,0 );
	line( gray, point3 + Point2f( templateImgGray.cols, 0), point0 + Point2f( templateImgGray.cols, 0), Scalar( 0, 255, 0), 10 ,8,0);

 //*****************************************************/

/*
	if(goodTemplateKeypoints.size() > 3 && goodFrameKeypoints.size() > 3)
	{
		Mat H = findHomography( goodTemplateKeypoints, goodFrameKeypoints, CV_RANSAC );

		//-- Get the corners from the image_1 ( the object to be "detected" )
		std::vector<Point2f> obj_corners(4);
		obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( templateImgGray.cols, 0 );
		obj_corners[2] = cvPoint( templateImgGray.cols, templateImgGray.rows ); obj_corners[3] = cvPoint( 0, templateImgGray.rows );
		std::vector<Point2f> scene_corners(4);
		perspectiveTransform( obj_corners, scene_corners, H);
		//-- Draw lines between the corners (the mapped object in the scene - image_2 )
		Point2f point0 = scene_corners[0];
		Point2f point1 = scene_corners[1];
		Point2f point2 = scene_corners[2];
		Point2f point3 = scene_corners[3];
		line( rgba, point0 + Point2f( templateImgGray.cols, 0), point1 + Point2f( templateImgGray.cols, 0), Scalar(0, 255, 0),10,8,0);
		line( rgba, point1 + Point2f( templateImgGray.cols, 0), point2 + Point2f( templateImgGray.cols, 0), Scalar( 0, 255, 0), 10 ,8,0);
		line( rgba, point2 + Point2f( templateImgGray.cols, 0), point3 + Point2f( templateImgGray.cols, 0), Scalar( 0, 255, 0), 10,8,0 );
		line( rgba, point3 + Point2f( templateImgGray.cols, 0), point0 + Point2f( templateImgGray.cols, 0), Scalar( 0, 255, 0), 10 ,8,0);
//		__android_log_print(ANDROID_LOG_ERROR, "info", "%f-%f %f-%f",point0.x,point0.y,point1.x,point1.y);
	}
*/
//    rgba = grayCannyHolder;

}

void Mat_to_vector_KeyPoint(Mat& mat, vector<KeyPoint>& v_kp)
{
	v_kp.clear();
//    CHECK_MAT(mat.type()==CV_32FC(7) && mat.cols==1);
	for(int i=0; i<mat.rows; i++)
	{
		KeyPoint kp(mat.at<float>(i,0), mat.at<float>(i,1), mat.at<float>(i,2), mat.at<float>(i,3), mat.at<float>(i,4), (int)mat.at<float>(i,5), (int)mat.at<float>(i,6));
		v_kp.push_back(kp);
	}
	return;
}

bool comapreDMatch(DMatch match1,DMatch match2)
{
    return match1.distance < match2.distance;
}

}
