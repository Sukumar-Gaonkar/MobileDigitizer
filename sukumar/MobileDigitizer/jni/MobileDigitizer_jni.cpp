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
JNIEXPORT void JNICALL Java_mobiledigitizer_core_MainActivity_imageProcess(JNIEnv* env,jobject thisObj, jlong addrRgba, jlong addrGray,jlong addrTemplateMat,jlong templateKeypoints);
void Mat_to_vector_KeyPoint(Mat& mat, vector<KeyPoint>& v_kp);

JNIEXPORT void JNICALL Java_mobiledigitizer_core_MainActivity_imageProcess(JNIEnv* env,jobject thisObj, jlong addrRgba, jlong addrGray,jlong addrTemplateMat,jlong addrTemplateKeypoints){
	__android_log_print(ANDROID_LOG_ERROR, "jni", "Entered Native Code2.0");

	clock_t tStart;
	Mat templateImgGray;
	std::vector< DMatch > matches;

	Mat& rgb = *(Mat*) addrRgba;
	Mat& gray = *(Mat*) addrGray;
	Mat& templateKeypointMat = *(Mat*) addrTemplateKeypoints;
	Mat& templateImg = *(Mat*) addrTemplateMat;
	__android_log_print(ANDROID_LOG_ERROR, "info", "templateImg %d",templateImg.type());
	cvtColor(templateImg,templateImgGray,CV_BGR2GRAY);
	vector<KeyPoint> templateKeypoints;

	tStart = clock();
	Mat_to_vector_KeyPoint(templateKeypointMat,templateKeypoints);
	__android_log_print(ANDROID_LOG_ERROR, "info", "MatofKeypoint to vector - %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

	OrbFeatureDetector detector;
	OrbDescriptorExtractor descriptor;
	std::vector<KeyPoint> keypoints_1, keypoints_2;
	Mat descriptors_1,descriptors_2;
//	FlannBasedMatcher matcher;
	BFMatcher matcher(NORM_HAMMING,1);

	tStart = clock();
	detector.detect(gray,keypoints_1);
	__android_log_print(ANDROID_LOG_ERROR, "info", "Keypoint detection time- %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
//	detector.detect(gray,keypoints_2);

	tStart = clock();
	descriptor.compute(gray,keypoints_1,descriptors_1);
	__android_log_print(ANDROID_LOG_ERROR, "info", "Keypoint description time- %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
	descriptor.compute(templateImgGray,templateKeypoints,descriptors_2);


////	tStart = clock();
//	if(descriptors_1.type()!=CV_32F) {
//		descriptors_1.convertTo(descriptors_1, CV_32F);
////		__android_log_print(ANDROID_LOG_ERROR, "info", "Conversion1- %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
//
//	}
////	tStart = clock();
//	if(descriptors_2.type()!=CV_32F) {
//		descriptors_2.convertTo(descriptors_2, CV_32F);
////		__android_log_print(ANDROID_LOG_ERROR, "info", "Conversion2- %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
//	}


	__android_log_print(ANDROID_LOG_ERROR, "info", "%d - %d  %d - %d\n", descriptors_1.type(),descriptors_2.type(),descriptors_1.cols,descriptors_2.cols);

	tStart = clock();
	matcher.match( descriptors_1, descriptors_2, matches );
	__android_log_print(ANDROID_LOG_ERROR, "info", "Matcher- %fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
}

void Mat_to_vector_KeyPoint(Mat& mat, vector<KeyPoint>& v_kp)
{
    v_kp.clear();
//    CHECK_MAT(mat.type()==CV_32FC(7) && mat.cols==1);
    for(int i=0; i<mat.rows; i++)
    {
        Vec<float, 7> v = mat.at< Vec<float, 7> >(i, 0);
        KeyPoint kp(v[0], v[1], v[2], v[3], v[4], (int)v[5], (int)v[6]);
        v_kp.push_back(kp);
    }
    return;
}

}
