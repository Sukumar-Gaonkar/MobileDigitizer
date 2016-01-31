package com.mobiledigitizer.ace;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.os.Environment;
import android.util.Log;
import android.view.WindowManager;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;
import org.opencv.core.MatOfKeyPoint;
import org.opencv.core.Size;
import org.opencv.features2d.*;
import org.opencv.highgui.Highgui;
import org.opencv.imgproc.Imgproc;


public class MainActivity extends Activity implements CvCameraViewListener2 {

    private CameraBridgeViewBase   	mOpenCvCameraView;
    Mat 							rgba,gray,templateMat,templateMatBig,templateDescriptors,templateCanny,output;
    MatOfKeyPoint 					templateKeypoints;
    String PicturesDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).toString();
    public native void imageProcess(long matAddrRgba,long matAddrGray,long templateMat,long templateKeypoints,long templateDescriptors,long addrOutput);


    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i("status", "OpenCV loaded successfully");
                    System.loadLibrary("AceNativeLib");
                    mOpenCvCameraView.enableView();

                    FeatureDetector detector = FeatureDetector.create(FeatureDetector.ORB);
                    DescriptorExtractor descriptor = DescriptorExtractor.create(DescriptorExtractor.ORB) ;
                    templateKeypoints = new MatOfKeyPoint();
                    templateDescriptors = new Mat();
                    templateCanny = new Mat();
                    templateMat = new Mat();
                    templateMatBig = Highgui.imread(PicturesDir+"/universityReceipt.jpg",Highgui.CV_LOAD_IMAGE_GRAYSCALE);
                    Imgproc.resize(templateMatBig, templateMat, new Size(1920, 1080));
//                    Log.e("info", PicturesDir+"/formFront1Min.jpg");
//                    long startTime = System.nanoTime();
                    Imgproc.Canny(templateMat, templateCanny, 50,150);
//                    templateCanny = templateMat;
                    Highgui.imwrite(PicturesDir+"/canny.jpg", templateCanny);
                    detector.detect(templateCanny, templateKeypoints);
//                    long endTime = System.nanoTime(); Log.e("info", "Detection- " + (float)(endTime - startTime)/1E9);
//                    startTime = System.nanoTime();
                    descriptor.compute(templateCanny, templateKeypoints, templateDescriptors);
//                    endTime = System.nanoTime(); Log.e("info", "Description- " + (float)(endTime - startTime)/1E9);
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

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
//		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_main);
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
    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
        // TODO Auto-generated method stub
        rgba = inputFrame.rgba();
        gray = inputFrame.gray();

        long startTime = System.nanoTime();
        imageProcess(rgba.getNativeObjAddr(),gray.getNativeObjAddr(),templateCanny.getNativeObjAddr(),templateKeypoints.getNativeObjAddr(),templateDescriptors.getNativeObjAddr(),output.getNativeObjAddr());
        long endTime = System.nanoTime();
        Log.e("info", "Native Code Time- " + (float)(endTime - startTime)/1E9);
        Highgui.imwrite(PicturesDir+"/op.jpg", output);

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

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
//		getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
//		int id = item.getItemId();
//		if (id == R.id.action_settings) {
//			return true;
//		}
        return super.onOptionsItemSelected(item);
    }
}
/*

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
*/