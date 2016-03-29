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
JNIEXPORT void JNICALL Java_com_mobiledigitizer_ace_OpenCVcamera_imageProcess(JNIEnv* env,jobject thisObj,jlong addrRgba,jlong addrGray,jlong addrTemplateMat,jlong addrTemplateKeypoints,jlong addrTemplateDescriptors,jlong addrOutput,jintArray markupAddr);
void Mat_to_vector_KeyPoint(Mat& mat, vector<KeyPoint>& v_kp);
bool comapreDMatch(DMatch match1,DMatch match2);

JNIEXPORT void JNICALL Java_com_mobiledigitizer_ace_OpenCVcamera_imageProcess(JNIEnv* env,jobject thisObj,jlong addrRgba,jlong addrGray,jlong addrTemplateMat,jlong addrTemplateKeypoints,jlong addrTemplateDescriptors,jlong addrOutput,jintArray markupAddr){
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

	jsize makrupLen = env->GetArrayLength(markupAddr);
	jint *markup = env->GetIntArrayElements(markupAddr, 0);

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
	vector <vector<Point> > contours_poly(contours.size());

	if(contours.size() > 1) {

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
			rectangle(rgba, boundRect.tl(), boundRect.br(), color, 2, 8, 0);
		}
	}
	__android_log_print(ANDROID_LOG_ERROR, "info", "Contour count: %d  Area: %f",contours.size(),maxBoundryArea);

//	if(contours.size() > 1)
//        gray = gray(boundRect);

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
	//-- or a small arbitrary value ( 0.02 ) in the event that min_dist is very
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

    std::vector<Point2f> frameCorners(4);// = contours_poly[formContourIndex];

    frameCorners[0] = Point2f (boundRect.tl().x,boundRect.tl().y); frameCorners[1] = Point2f( boundRect.tl().x + boundRect.width, boundRect.tl().y );
    frameCorners[2] = Point2f( boundRect.br().x,boundRect.br().y ); frameCorners[3] = Point2f(boundRect.tl().x, boundRect.tl().y+boundRect.height );

    std::vector<Point2f> templateCorners(4);

    templateCorners[0] = Point2f (0,0); templateCorners[1] = Point2f( templateImgGray.cols, 0 );
    templateCorners[2] = Point2f( templateImgGray.cols, templateImgGray.rows ); templateCorners[3] = Point2f( 0, templateImgGray.rows );

//    int npoints = templateCorners.getMat().checkVector(2);
//    int npoin = frameCorners.getMat().checkVector(2);
//    __android_log_print(ANDROID_LOG_ERROR, "info", "template check vector - %d frame check vector- %d\n",npoints,npoin);
    Mat H = findHomography( templateCorners, frameCorners, CV_RANSAC );

    std::vector<vector<Point2f> > mappingConatiner;
    std::vector<Point2f> templateDigitizeSection(2);
    std::vector<Point2f> frameDigitizeSection(2);
///*
	for (int i=0; i<makrupLen; i+=4){

		templateDigitizeSection[0] = Point2f(markup[i],markup[i+1]);
		templateDigitizeSection[1] = Point2f(markup[i+2],markup[i+3]);
		perspectiveTransform( templateDigitizeSection, frameDigitizeSection, H);
		rectangle(rgba, frameDigitizeSection[0], frameDigitizeSection[1], Scalar(0, 255, 0), 2, 8, 0);

	}
// */
    /*
	{
        templateDigitizeSection[0] = Point2f(691,588);
        templateDigitizeSection[1] = Point2f(766,1021);
        perspectiveTransform( templateDigitizeSection, frameDigitizeSection, H);
        rectangle(rgba, frameDigitizeSection[0], frameDigitizeSection[1], Scalar(0, 255, 0), 2, 8, 0);

        templateDigitizeSection[0] = Point2f(766,588);
        templateDigitizeSection[1] = Point2f(840,1021);
        perspectiveTransform( templateDigitizeSection, frameDigitizeSection, H);
        rectangle(rgba, frameDigitizeSection[0], frameDigitizeSection[1], Scalar(0, 255, 0), 2, 8, 0);

        templateDigitizeSection[0] = Point2f(1020,587);
        templateDigitizeSection[1] = Point2f(1092,1020);
        perspectiveTransform( templateDigitizeSection, frameDigitizeSection, H);
        rectangle(rgba, frameDigitizeSection[0], frameDigitizeSection[1], Scalar(0, 255, 0), 2, 8, 0);

        templateDigitizeSection[0] = Point2f(1020,151);
        templateDigitizeSection[1] = Point2f(1092,587);
        perspectiveTransform( templateDigitizeSection, frameDigitizeSection, H);
        rectangle(rgba, frameDigitizeSection[0], frameDigitizeSection[1], Scalar(0, 255, 0), 2, 8, 0);

        templateDigitizeSection[0] = Point2f(1165,587);
        templateDigitizeSection[1] = Point2f(1240,1020);
        perspectiveTransform( templateDigitizeSection, frameDigitizeSection, H);
        rectangle(rgba, frameDigitizeSection[0], frameDigitizeSection[1], Scalar(0, 255, 0), 2, 8, 0);

        templateDigitizeSection[0] = Point2f(1240,587);
        templateDigitizeSection[1] = Point2f(1310,1020);
        perspectiveTransform( templateDigitizeSection, frameDigitizeSection, H);
        rectangle(rgba, frameDigitizeSection[0], frameDigitizeSection[1], Scalar(0, 255, 0), 2, 8, 0);

    }
	*/


/*******Side by Side Mapping*******************
    Mat imgMatches(templateImgGray.rows+gray.rows,templateImgGray.cols+gray.cols,CV_8UC1);
    Mat left(imgMatches, Rect(0, 0, templateImgGray.cols, templateImgGray.rows)); // Copy constructor
    templateImgGray.copyTo(left);
    Mat right(imgMatches, Rect(templateImgGray.cols, 0, gray.cols, gray.rows)); // Copy constructor
    gray.copyTo(right);

    line( imgMatches, templateDigitizeSection[0] ,frameDigitizeSection[0] + Point2f(templateImgGray.cols,0), Scalar(0, 255, 0),10,8,0);
    line( imgMatches, templateDigitizeSection[1] ,frameDigitizeSection[1] + Point2f(templateImgGray.cols,0), Scalar(0, 255, 0),10,8,0);

    output = imgMatches;

    resize(imgMatches,rgba,Size(rgba.cols,rgba.rows));

//    __android_log_print(ANDROID_LOG_ERROR, "info", "Template X %f %f %f %f\n",boxOriginal[0].x,boxOriginal[1].x,boxOriginal[2].x,boxOriginal[3].x);
//    __android_log_print(ANDROID_LOG_ERROR, "info", "Template Y %f %f %f %f\n",boxOriginal[0].y,boxOriginal[1].y,boxOriginal[2].y,boxOriginal[3].y);
//    __android_log_print(ANDROID_LOG_ERROR, "info", "frame X %f %f %f %f\n",scene_corners[0].x,scene_corners[1].x,scene_corners[2].x,scene_corners[3].x);
//    __android_log_print(ANDROID_LOG_ERROR, "info", "frame Y %f %f %f %f\n",scene_corners[0].y,scene_corners[1].y,scene_corners[2].y,scene_corners[3].y);

*****************************************************/

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

/**********Data*********
TemplateDemoForm Sections to digitize
    Value section
    573,691 - 140,766
    573,766 - 140,840

    574,1020 - 141,1092
    574,1092 - 141,1165
    574,1165 - 141,1240
    574,1240 - 141,1310

    Key Section
    151,1020 - 587,1092 (Groundnut)

TemplateICCPCT big area
    136,515 1410,526 1388,1165 121,1154

 ***********************/

}


