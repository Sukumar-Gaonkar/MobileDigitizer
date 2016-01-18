package mobiledigitizer.core;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.MatOfKeyPoint;
import org.opencv.features2d.*;
import org.opencv.highgui.Highgui;
import com.alpha.mobiledigitizer.R;
import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;

public class MainActivity extends Activity implements CvCameraViewListener2 {

	private CameraBridgeViewBase   	mOpenCvCameraView;
	Mat 							rgba,gray,templateMat,templateDescriptors;
	MatOfKeyPoint 					templateKeypoints;
	String PicturesDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).toString();
	public native void imageProcess(long matAddrGray,long templateMat,long templateKeypoints,long templateDescriptors);
	
	
	private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i("status", "OpenCV loaded successfully");
                    System.loadLibrary("MobileDigitizerNativeLib");
                    mOpenCvCameraView.enableView();
                    
                    FeatureDetector detector = FeatureDetector.create(FeatureDetector.FAST);
                    DescriptorExtractor descriptor = DescriptorExtractor.create(DescriptorExtractor.ORB) ;
                    templateKeypoints = new MatOfKeyPoint();
                    templateDescriptors = new Mat();
                    templateMat = Highgui.imread(PicturesDir+"/formBack1Min.jpg",Highgui.CV_LOAD_IMAGE_GRAYSCALE);
                    Log.e("info", PicturesDir+"/formBack1Min.jpg");
                    long startTime = System.nanoTime();
                    detector.detect(templateMat, templateKeypoints);
                    long endTime = System.nanoTime(); Log.e("info", "Detection- " + (float)(endTime - startTime)/1E9);
                    startTime = System.nanoTime();
                    descriptor.compute(templateMat, templateKeypoints, templateDescriptors);
                    endTime = System.nanoTime(); Log.e("info", "Description- " + (float)(endTime - startTime)/1E9);
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
	}
	
	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		// TODO Auto-generated method stub
		rgba = inputFrame.rgba();
		gray = inputFrame.gray();
		
		long startTime = System.nanoTime();
		imageProcess(gray.getNativeObjAddr(),templateMat.getNativeObjAddr(),templateKeypoints.getNativeObjAddr(),templateDescriptors.getNativeObjAddr());
		long endTime = System.nanoTime();
		Log.e("info", "Native Code Time- " + (float)(endTime - startTime)/1E9);
		return rgba;
	}
	
	@Override
	public void onCameraViewStopped() {
		// TODO Auto-generated method stub
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
