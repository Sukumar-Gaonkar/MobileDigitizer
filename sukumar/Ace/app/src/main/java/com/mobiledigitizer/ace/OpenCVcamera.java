package com.mobiledigitizer.ace;

import android.os.Bundle;
import android.app.Activity;
import android.os.Environment;
import android.util.Log;
import android.view.WindowManager;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.MatOfKeyPoint;
import org.opencv.core.Size;
import org.opencv.features2d.DescriptorExtractor;
import org.opencv.features2d.FeatureDetector;
import org.opencv.highgui.Highgui;
import org.opencv.imgproc.Imgproc;

public class OpenCVcamera extends Activity implements CameraBridgeViewBase.CvCameraViewListener2 {

    private CameraBridgeViewBase   	mOpenCvCameraView;
    Mat 							rgba,rgbaHolder,gray,templateMat,templateMatBig,templateDescriptors,templateCanny,output;
    MatOfKeyPoint templateKeypoints;
    String PicturesDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).toString();
    public native void imageProcess(long matAddrRgba,long matAddrGray,long templateMat,long templateKeypoints,long templateDescriptors,long addrOutput);


    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i("status", "OpenCV loaded successfully");
                    System.loadLibrary("gnustl_shared");
                    System.loadLibrary("nonfree");
                    System.loadLibrary("AceNativeLib");
                    mOpenCvCameraView.enableView();

                    FeatureDetector detector = FeatureDetector.create(FeatureDetector.SIFT);
                    DescriptorExtractor descriptor = DescriptorExtractor.create(DescriptorExtractor.SURF) ;
                    templateKeypoints = new MatOfKeyPoint();
                    templateDescriptors = new Mat();
                    templateCanny = new Mat();
                    templateMat = new Mat();
                    templateMatBig = Highgui.imread(PicturesDir + "/templateICCPCT.jpg", Highgui.CV_LOAD_IMAGE_GRAYSCALE);

                    if(templateMatBig.height() > templateMatBig.width())
                    {
                        templateMatBig = templateMatBig.t();
                        Core.flip(templateMatBig, templateMatBig, 0);
                    }
//                    int h =(int)( (double)templateMatBig.height()/templateMatBig.width() * 1920);
//                    Imgproc.resize(templateMatBig, templateMat, new Size(1920, h));

//                    Log.e("info", PicturesDir+"/formFront1Min.jpg");
//                    long startTime = System.nanoTime();
//                    Imgproc.Canny(templateMat, templateCanny, 50, 150);
//                    templateCanny = templateMat;
                    templateCanny = templateMatBig;
//                    Highgui.imwrite(PicturesDir+"/canny.jpg", templateCanny);
                    detector.detect(templateCanny, templateKeypoints);

//                    long endTime = System.nanoTime(); Log.e("info", "Detection- " + (float)(endTime - startTime)/1E9);
//                    startTime = System.nanoTime();
                    descriptor.compute(templateCanny, templateKeypoints, templateDescriptors);

//                    endTime = System.nanoTime(); Log.e("info", "Description- " + (float)(endTime - startTime)/1E9);


//                    {
                        rgbaHolder = Highgui.imread(PicturesDir+"/frameICCPCT.jpg",Highgui.CV_LOAD_IMAGE_GRAYSCALE);
/*                        output = new Mat();
                        rgbaHolder = Highgui.imread(PicturesDir+"/frameICCPCT.jpg",Highgui.CV_LOAD_IMAGE_GRAYSCALE);
                        Highgui.imwrite(PicturesDir + "/inputOriginal.jpg", rgbaHolder);
                        imageProcess(rgbaHolder.getNativeObjAddr(), rgbaHolder.getNativeObjAddr(), templateCanny.getNativeObjAddr(), templateKeypoints.getNativeObjAddr(), templateDescriptors.getNativeObjAddr(), output.getNativeObjAddr());
//                        Highgui.imwrite(PicturesDir + "/op.jpg", output);
                        Highgui.imwrite(PicturesDir+"/op2.jpg", rgbaHolder);
                    }
*/
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.camera_layout);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
//		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.javaCameraView);
        mOpenCvCameraView.setCvCameraViewListener(this);
    }

    @Override
    public void onCameraViewStarted(int width, int height) {
        // TODO Auto-generated method stub
//		rgba = new Mat(height,width,CvType.CV_8UC3);
        output = new Mat();
    }

    @Override
    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        // TODO Auto-generated method stub
        rgba = inputFrame.rgba();
        gray = inputFrame.gray();

        {
            output = new Mat();
//            Highgui.imwrite(PicturesDir + "/inputOriginal.jpg", rgbaHolder);
            imageProcess(rgba.getNativeObjAddr(), gray.getNativeObjAddr(), templateCanny.getNativeObjAddr(), templateKeypoints.getNativeObjAddr(), templateDescriptors.getNativeObjAddr(), output.getNativeObjAddr());
//            Highgui.imwrite(PicturesDir + "/op.jpg", output);
            Highgui.imwrite(PicturesDir+"/op2.jpg", rgbaHolder);
        }
        return rgba;
    }


    @Override
    public void onCameraViewStopped() {
        // TODO Auto-generated method stub
        if(rgba != null)
            rgba.release();
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_11, this, mLoaderCallback);
    }

    public void onDestroy() {
        super.onDestroy();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

}
