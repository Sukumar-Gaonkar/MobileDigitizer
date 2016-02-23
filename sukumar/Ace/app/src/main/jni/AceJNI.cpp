#include <jni.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#include <time.h>
#include <typeinfo>
#include <android/log.h>
#include <vector>

using namespace std;
using namespace cv;

extern "C" {
JNIEXPORT void JNICALL Java_com_mobiledigitizer_ace_MainActivity_imageProcess(JNIEnv* env,jobject thisObj,jlong addrRgba,jlong addrGray,jlong addrTemplateMat,jlong addrTemplateKeypoints,jlong addrTemplateDescriptors,jlong addrOutput);
void Mat_to_vector_KeyPoint(Mat& mat, vector<KeyPoint>& v_kp);

JNIEXPORT void JNICALL Java_com_mobiledigitizer_ace_MainActivity_imageProcess(JNIEnv* env,jobject thisObj,jlong addrRgba,jlong addrGray,jlong addrTemplateMat,jlong addrTemplateKeypoints,jlong addrTemplateDescriptors,jlong addrOutput){
//	__android_log_print(ANDROID_LOG_ERROR, "jni", "Entered Native Code2.0");

	clock_t tStart;

	std::vector< DMatch > matches;
	Mat grayCanny;
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

	Canny(gray,grayCanny,50,100);
//	grayCanny = gray;

	tStart = clock();
	Mat_to_vector_KeyPoint(templateKeypointMat,templateKeypoints);
//	__android_log_print(ANDROID_LOG_ERROR, "info", "MatofKeypoint to vector - %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

	OrbFeatureDetector detector;
	OrbDescriptorExtractor descriptor;
	std::vector<KeyPoint> keypoints_1;
	Mat descriptors_1;
	FlannBasedMatcher matcher;

	tStart = clock();
	detector.detect(grayCanny,keypoints_1);
//	__android_log_print(ANDROID_LOG_ERROR, "info", "Keypoint detection time- %fs Keypoints- %d\n", (double)(clock() - tStart)/CLOCKS_PER_SEC,keypoints_1.size());

	tStart = clock();
	descriptor.compute(grayCanny,keypoints_1,descriptors_1);
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

	double max_dist = 0; double min_dist = 100;
	DMatch match;
	int goodMatchCount = 0;
	double AvgError = 0;
	std::vector< DMatch > good_matches;

	for( int i = 0; i < descriptors_1.rows; i++ )
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
//	std::vector< DMatch > good_matches;
	std::vector<Point2f> goodTemplateKeypoints;
	std::vector<Point2f> goodFrameKeypoints;
	KeyPoint keypointHolder;
	for( int i = 0; i < descriptors_1.rows; i++ )
	{
		match = matches[i];
		if( match.distance <= max(3*min_dist, 0.02) )
		{

			goodMatchCount++;
			AvgError += match.distance;
//			good_matches.push_back(match);
			keypointHolder = keypoints_1[match.queryIdx];
			goodFrameKeypoints.push_back( keypointHolder.pt );
			keypointHolder = templateKeypoints[match.trainIdx];
			goodTemplateKeypoints.push_back(keypointHolder.pt);
		}
	}

	AvgError /= goodMatchCount;
//	__android_log_print(ANDROID_LOG_ERROR, "info", "min - %f max - %f\n",min_dist,max_dist);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	RNG rng(12345);
	findContours( grayCanny, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
//	for( int i = 0; i< contours.size(); i++ )
//	{
//		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//		drawContours( rgba, contours, i, color, 2, 8, hierarchy, 0, Point() );
//	}

	vector<vector<Point> > contours_poly( contours.size() );
	vector<Rect> boundRect( contours.size() );

	for( size_t i = 0; i < contours.size(); i++ )
	{
		approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
		boundRect[i] = boundingRect( Mat(contours_poly[i]) );
	}
	for( size_t i = 0; i< contours.size(); i++ )
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		rectangle( rgba, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
	}
	__android_log_print(ANDROID_LOG_ERROR, "info", "Contour count: %d",contours.size());
//	__android_log_print(ANDROID_LOG_ERROR, "info", "Good Matches - %d Avg Error- %fs\n",goodMatchCount, AvgError);
//	drawKeypoints( grayCanny, keypoints_1, grayCanny, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
//	drawKeypoints( templateImgGray, keypoints_1, templateImgGray, Scalar::all(-1), DrawMatchesFlags::DEFAULT );
//	drawMatches( grayCanny, keypoints_1, templateImgGray, templateKeypoints,good_matches, output, Scalar::all(-1), Scalar::all(-1),vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

//	__android_log_print(ANDROID_LOG_ERROR, "info", "checkpoint 1");
/*
	if(goodTemplateKeypoints.size() > 3 && goodFrameKeypoints.size() > 3)
	{
		__android_log_print(ANDROID_LOG_ERROR, "info", "checkpoint 2");
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
		__android_log_print(ANDROID_LOG_ERROR, "info", "%f-%f %f-%f",point0.x,point0.y,point1.x,point1.y);
	}
*/
//    rgba = grayCanny;
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

}
