package com.mobiledigitizer.ace;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.app.Activity;
import android.os.Environment;
import android.util.Log;
import android.view.MotionEvent;
import android.view.WindowManager;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.MatOfKeyPoint;
import org.opencv.core.Rect;
import org.opencv.features2d.DescriptorExtractor;
import org.opencv.features2d.FeatureDetector;
import org.opencv.highgui.Highgui;

import java.io.FileInputStream;
import java.util.Scanner;

import com.googlecode.tesseract.android.TessBaseAPI;


public class OpenCVcamera extends Activity implements CameraBridgeViewBase.CvCameraViewListener2 {

    private CameraBridgeViewBase   	mOpenCvCameraView;
    Mat 							rgba,rgbaHolder,gray,templateMat,templateMatBig,templateDescriptors,templateCanny,output;
    Bitmap ORCImageHolder;
    MatOfKeyPoint templateKeypoints;
    String AceDirectory = Environment.getExternalStorageDirectory().toString() +"/ACE";
    public final String DATA_PATH = AceDirectory;
    public String lang = "eng";
    Bitmap bitmap;
    TessBaseAPI baseApi = new TessBaseAPI();
    int[] coordinates;
    public native void imageProcess(long matAddrRgba,long matAddrGray,long templateMat,long templateKeypoints,long templateDescriptors,long addrOutput,int[] markup);

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
                    templateMatBig = Highgui.imread(AceDirectory + "/templateDemoForm.jpg", Highgui.CV_LOAD_IMAGE_GRAYSCALE);

                    if(templateMatBig.height() > templateMatBig.width())
                    {
                        templateMatBig = templateMatBig.t();
                        Core.flip(templateMatBig, templateMatBig, 0);
                    }
//                    int h =(int)( (double)templateMatBig.height()/templateMatBig.width() * 1920);
//                    Imgproc.resize(templateMatBig, templateMat, new Size(1920, h));

//                    Log.e("info", AceDirectory+"/formFront1Min.jpg");
//                    long startTime = System.nanoTime();
//                    Imgproc.Canny(templateMat, templateCanny, 50, 150);
//                    templateCanny = templateMat;
                    templateCanny = templateMatBig;
//                    Highgui.imwrite(AceDirectory+"/canny.jpg", templateCanny);
                    detector.detect(templateCanny, templateKeypoints);

//                    long endTime = System.nanoTime(); Log.e("info", "Detection- " + (float)(endTime - startTime)/1E9);
//                    startTime = System.nanoTime();
                    descriptor.compute(templateCanny, templateKeypoints, templateDescriptors);

//                    endTime = System.nanoTime(); Log.e("info", "Description- " + (float)(endTime - startTime)/1E9);


//                    {
                        rgbaHolder = Highgui.imread(AceDirectory +"/frameICCPCT.jpg",Highgui.CV_LOAD_IMAGE_GRAYSCALE);
/*                        output = new Mat();
                        rgbaHolder = Highgui.imread(AceDirectory+"/frameICCPCT.jpg",Highgui.CV_LOAD_IMAGE_GRAYSCALE);
                        Highgui.imwrite(AceDirectory + "/inputOriginal.jpg", rgbaHolder);
                        imageProcess(rgbaHolder.getNativeObjAddr(), rgbaHolder.getNativeObjAddr(), templateCanny.getNativeObjAddr(), templateKeypoints.getNativeObjAddr(), templateDescriptors.getNativeObjAddr(), output.getNativeObjAddr());
//                        Highgui.imwrite(AceDirectory + "/op.jpg", output);
                        Highgui.imwrite(AceDirectory+"/op2.jpg", rgbaHolder);
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

        ///**********Read CSV File**********************

        try{
//            Scanner sc = new Scanner(this.getResources().openRawResource(R.raw.templatedemoformmarkup));
            Scanner sc = new Scanner(new FileInputStream(AceDirectory +"/templatedemoformmarkup.csv"));
            String[] coordinatesHolder = sc.nextLine().split(",");
            coordinates = new int[coordinatesHolder.length];

            for(int i=0;i<coordinatesHolder.length;i++)
                coordinates[i] = Integer.parseInt(coordinatesHolder[i]);
        }
        catch (Exception e)
        {}

        //*********************************************/

        ///**********Initialize Tessaract API**********************
        baseApi.setDebug(true);
        baseApi.init(DATA_PATH, lang);
        //*********************************************/
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
//            Highgui.imwrite(AceDirectory + "/inputOriginal.jpg", rgbaHolder);
            imageProcess(rgba.getNativeObjAddr(), gray.getNativeObjAddr(), templateCanny.getNativeObjAddr(), templateKeypoints.getNativeObjAddr(), templateDescriptors.getNativeObjAddr(), output.getNativeObjAddr(), coordinates);
//            Highgui.imwrite(AceDirectory + "/op.jpg", output);
            Highgui.imwrite(AceDirectory + "/op2.jpg", rgbaHolder);
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
    public boolean onTouchEvent(MotionEvent event) {

        for(int i=0;i<coordinates.length;i++)
        {

            Mat m = new Mat(gray,new Rect(coordinates[i*4],coordinates[i*4+1],coordinates[i*4+2]-coordinates[i*4],coordinates[i*4+3]-coordinates[i*2]));
            Utils.matToBitmap(m,bitmap);
            baseApi.setImage(bitmap);
            String recognizedText = baseApi.getUTF8Text();
            Log.e("info","OCR : "+recognizedText);
        }
        return super.onTouchEvent(event);
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
        baseApi.end();
    }

}
