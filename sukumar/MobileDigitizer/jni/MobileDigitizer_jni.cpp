
#include <android/log.h>
#include <jni.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

using namespace std;
using namespace cv;

extern "C" {
JNIEXPORT void JNICALL Java_mobiledigitizer_core_MainActivity_imageProcess(JNIEnv* env,jobject thisObj, jlong addrRgba, jlong addrGray);

JNIEXPORT void JNICALL Java_mobiledigitizer_core_MainActivity_imageProcess(JNIEnv* env,jobject thisObj, jlong addrRgba, jlong addrGray){
	__android_log_print(ANDROID_LOG_ERROR, "jni", "Entered Native Code2.0");

	Mat& rgb = *(Mat*) addrRgba;
	Mat& gray = *(Mat*) addrGray;
//ORB::ORB();

//	FeatureDetector::create("ORB");
//	Ptr<FeatureDetector> dec = FeatureDetector::create("ORB");
	OrbFeatureDetector detector;
	OrbDescriptorExtractor descriptor;
	std::vector<KeyPoint> keypoints_1, keypoints_2;
	detector.detect(gray,keypoints_1);
	detector.detect(gray,keypoints_2);
	Mat descriptors_1,descriptors_2;
	descriptor.compute(gray,keypoints_1,descriptors_1);
	descriptor.compute(gray,keypoints_2,descriptors_2);
//	cv::FeatureDetector * detector2 = new cv::OrbFeatureDetector();
//	detector2->detect(gray,keypoints_1);
	FlannBasedMatcher matcher;
	std::vector< DMatch > matches;
	if(descriptors_1.type()!=CV_32F) {
		descriptors_1.convertTo(descriptors_1, CV_32F);
		descriptors_2.convertTo(descriptors_2, CV_32F);
	}
	matcher.match( descriptors_1, descriptors_2, matches );
}

}
