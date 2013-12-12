#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv\cv.h>
#include <windows.h>
#include <string.h>
#include <math.h>

using namespace cv;
using namespace std;


// Set the constants used for the chessboard recognition pattern and the calibration
const int numBoards = 20;
const int nrHorizontalCorners = 8;
const int nrVerticalCorners = 6;
const Size board_size = Size(nrHorizontalCorners,nrVerticalCorners);
int imgCount =0;
String videoPath;
String camNr = "3";
String camPath = "../data/cam";
String imgName = "calibrationImg";
String imgExt = ".jpg";
int startNr= 0;
int endNr = 85;

/*
*Class Line, is an extension of the already existing class line. Upon creation, the img 
* on which should be drawn is requested, together with the start and end point (which is in 2D)
*
*/
class Line{

	int RED, BLUE, GREEN, BLACK;
	Point startPoint,stopPoint;
	int thikness,lineType;
	Mat image;

	/*Line, is the constructor of the class. It request an image to draw on, a start and an end point.
	*/
	public:Line(Mat img,Point start, Point end)
	{
		Line::startPoint = start;
		Line::stopPoint = end;
		Line::image = img;
		thikness = 1;
		lineType = 1;
		RED = 0;
		GREEN = 1;
		BLUE = 2;
		BLACK = 3;
	}
	
	/* setStart sets the startPoint of the line */
	void setStart(Point start)
	{
		Line::startPoint = start;
	}

	/* setStop sets the endpoint of the line */
	void setStop(Point stop)
	{
		Line::stopPoint = stop;
	}

	/* draw is used to draw a line from the start to the endpoint.
	* Based upon the color, represented by an integer, the line will be drawn in
	* that color */
	void draw(int color)
	{
		int thickness = 2;
		int lineType = 8;

		if(color == BLUE)
		{
			line(image,startPoint,stopPoint,Scalar( 0, 0, 255),thickness,lineType);

		}else if(color == RED){
			line(image,startPoint,stopPoint,Scalar( 255, 0, 0),thickness,lineType);

		}else if(color == GREEN){
			line(image,startPoint,stopPoint,Scalar( 0, 255, 0),thickness,lineType);

		}else if(color == BLACK){
			line(image,startPoint,stopPoint,Scalar( 0, 0, 0),thickness,lineType);
		}
		
	}
};

/*drawing, the function that draws an axis system and a cube onto a specified image, using the Pmatrix (3Dworld
* to 2Dimage-matrix). The length of the axes and ribs of the cube are fixed to 3 and 2 respectively.
*
*Arguments
*  PMatrix		the 3Dworld to 2Dimage conversion matrix, K[R t]. A 3x4 matrix in Mat format
*  corners		the vector list of detected corners by findChessboardCorners() of the grid, used a reference.
*  image		the image on which the drawing shoulw be made.
*/
void drawing(Mat PMatrix,vector<Point2f> corners, Mat image)
{
	int i;
	vector<Mat> axis;
	vector<Point2f> imgPoints  = vector<Point2f>();
	vector<Point2f> bottomFace = vector<Point2f>();
	vector<Point2f> upperFace  = vector<Point2f>();

	// Draw the axes
	/*===================================================*/
	axis.push_back((Mat_<double>(4,1)<<0,0,0,1));
	axis.push_back((Mat_<double>(4,1)<<8,0,0,1));
	axis.push_back((Mat_<double>(4,1)<<0,8,0,1));
	axis.push_back((Mat_<double>(4,1)<<0,0,8,1));

	Mat solutin;
	for(i=0;i<axis.size();i++)
	{
		solutin=PMatrix*axis[i];
		imgPoints.push_back(Point2f(solutin.at<double>(0,0)/solutin.at<double>(2,0),
									solutin.at<double>(1,0)/solutin.at<double>(2,0)
									));
	}
	/*
	Line OX = Line(image,imgPoints[0],imgPoints[1]);
	OX.draw(0);

	Line OY = Line(image,imgPoints[0],imgPoints[2]);
	OY.draw(1);

	Line OZ = Line(image,imgPoints[0],imgPoints[3]);
	OZ.draw(2);
	*/
	
	Line OX = Line(image,corners[0],imgPoints[1]);
	OX.draw(0);

	Line OY = Line(image,corners[0],imgPoints[2]);
	OY.draw(1);

	Line OZ = Line(image,corners[0],imgPoints[3]);
	OZ.draw(2);
	
	/*======================================================*/
	
	// Draw the cube
	/*======================================================*/
	// Bottom face:
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	axis.clear();
	axis.push_back((Mat_<double>(4,1)<<0.0,0.0,0.0,1.0));
	axis.push_back((Mat_<double>(4,1)<<2,0,0,1));
	axis.push_back((Mat_<double>(4,1)<<2,2,0,1));
	axis.push_back((Mat_<double>(4,1)<<0,2,0,1));
	for(i=0;i<axis.size();i++)
	{
		solutin=PMatrix*axis[i];
		bottomFace.push_back(Point2f(solutin.at<double>(0,0)/solutin.at<double>(2,0),
									 solutin.at<double>(1,0)/solutin.at<double>(2,0)));
	}
	for(i=0;i<bottomFace.size()-1;i++)
	{
		Line faceLine = Line(image,bottomFace[i],bottomFace[i+1]);
		faceLine.draw(3);
	}
	Line faceLine = Line(image,bottomFace[i],bottomFace[0]);
	faceLine.draw(3);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	
	// Upper face:
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	axis.clear();
	axis.push_back((Mat_<double>(4,1)<<0.0,0.0,2.0,1.0));
	axis.push_back((Mat_<double>(4,1)<<2,0,2,1));
	axis.push_back((Mat_<double>(4,1)<<2,2,2,1));
	axis.push_back((Mat_<double>(4,1)<<0,2,2,1));
	for(i=0;i<axis.size();i++)
	{
		solutin=PMatrix*axis[i];
		upperFace.push_back(Point2f(solutin.at<double>(0,0)/solutin.at<double>(2,0),solutin.at<double>(1,0)/solutin.at<double>(2,0)));
	}
	for(i=0;i<upperFace.size()-1;i++)
	{
		Line faceLine = Line(image,upperFace[i],upperFace[i+1]);
		faceLine.draw(3);
	}
	faceLine = Line(image,upperFace[i],upperFace[0]);
	faceLine.draw(3);
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	////Other faces, from bottom face to upper face:
	for(i=0;i<upperFace.size();i++)
	{
		faceLine = Line(image,bottomFace[i],upperFace[i]);
		faceLine.draw(3);
	}
	//*======================================================*/
}

/** CameraCalibration. The function uses the inputstream videocapture to grasp images and find chessboard patterns 
* within those images. These chessboardpatterns are then used to perform the actual calibration
* of the videocapture device. This means, that the intrinsic camera matrix K is found, together with the distortion
* vector.
*
*Arguments
*	stream		this refers to the VideoCapture stream that is used to extract images from.
*	phys_corners the pattern that needs to be found on the corners of the chessboard have certain
*				   which are contained by phys_corners.
*	intrinsic	an output matrix containing the intrinsic values of the videocapturing device
*	distCoeffs  an output matrix with the distortion coefficients found from the calibration.
*
*Output
*	The functions returns 1 if the calabration executed without any problems, 0 otherwise
*
*
**/
int cameraCalibration(VideoCapture stream, vector<Point3f> phys_corners, Mat& intrinsic, Mat& distCoeffs)
{
	/* 2 Matrices of the physical positions of the corners
	   and the positions in the image.
	   physical_corners -> 3D
	   image_corners -> 2D
	*/
	vector<vector<Point3f>> physical_corners;
	vector<vector<Point2f>> image_corners;
	vector<Point2f> corners;
    int ok=0;
	Mat image,gray_image;
	int successes = 0;
	Size board_size = Size(nrHorizontalCorners,nrVerticalCorners);
	Size imgSize;
    stream >> image;
	cout<< "Camera calibration began..."<< endl
		<< "If asked to save image, hit 'spacebar' to confirm, any other key te 'cancel'"<<endl;

	// Keep capturing images as long as the number of images needed for calibration is not reached
	while(!image.empty())
    {	
		Mat proc_img = image.clone();
		  cvtColor(image, gray_image, CV_BGR2GRAY);
		  bool found = findChessboardCorners(proc_img, board_size, corners, 
								CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
		  
		  //cout<< "Processingimage!"<<endl;
		  // only proccess an image if there are chessboardcorners
		  if(found)
		  {
			  cornerSubPix(gray_image, corners, Size(11, 11), Size(-1, -1),
					 TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
			  drawChessboardCorners(proc_img, board_size, corners, found);
			  imshow("win1",proc_img);	
			  
			  // Check whether the found corners correspond to an comparable finding on 
			  // on of the other calibration images
			  double error,errorA,errorB,errorC;
			  double threshold = 600; // if the accumulated difference  of all corners
			  int countGood=0;
			  if (successes==0)
			  {
				  image_corners.push_back(corners);
				  physical_corners.push_back(phys_corners);
				  imgSize = image.size();
				  countGood++;
			  } else 
			  {
				  cout<< "NEXT IMAGE:";
				  for (int entry=0; entry<image_corners.size();entry++)
				  {   // an entry in image_corners is similar to the current found pattern
					  // up to an accumulated difference of x px (say 30px).
					  //question: How to find the accumulated difference between those

					  error=0;
					  //cout<<"checking entry "<<entry<<endl;
					  for (int cornerId=0; cornerId<nrVerticalCorners*nrHorizontalCorners;cornerId++)
					  {
					  
						  //cout<< corners.at(cornerId) << endl;
						  //cout<< image_corners.at(entry).at(cornerId)<<endl;
						  error += sqrt( pow((corners.at(cornerId).x - image_corners.at(entry).at(cornerId).x), 2) + 
							pow(corners.at(cornerId).y - image_corners.at(entry).at(cornerId).y, 2));
					  
						  //cout << corners.at(cornerId)<<endl; // some cout to see what's happening
						  //cout << image_corners.at(entry).at(cornerId)<<endl;

						  if (error >= threshold)
						  {
							  //cout<< "Entry \""<< entry<< "\" is fine! Error= "<<error<<" ("<<cornerId<<")";
							  countGood++;
							  cornerId = nrVerticalCorners*nrHorizontalCorners;
						  }
					  }
					  if (error < threshold)
					  {
						  cout<<" The image did not make it!"<<endl;
						  break;
					  }
				  }
			  }

			  //A chessboard is found on the image, ask the user whether to keep it or not
			  
			  //cout<<key<<endl;
			  if (countGood == image_corners.size())
			  {
				  cout<<"Use image for calibration? ";
			      int key = waitKey();
				  if (key==32){
					  String imgName  = "calibrationImg";
					  imgName += to_string(imgCount++);
					  imgName += ".jpg";
					  imwrite(camPath+camNr+"/"+imgName,image);
					
					  cout<< countGood<< "=="<<image_corners.size()<<endl;
					  successes++;
					  image_corners.push_back(corners);
					  physical_corners.push_back(phys_corners);
					  cout<<"Snap stored! Successes: "<<successes<<endl;
					  stream >> image; stream >> image;stream >> image;stream >> image;
				  }
				}
			 

			  cout<< "======================="<<endl;
		  } else {
			imshow("win1", image);
		  }
		  
 
          int key = waitKey(1);
		  if(key==27) return 0;
		  stream >> image;
      }
	 
	 /*At this point, there are enough images to do the actual calibration.
	 * For this the intrinsic matrix K is initialised, and the output variables
	 * (rvecs, rotation matrices and tvecs, translation matrices) are created
	 */

	cout<< "Video is done, let's calibrate!"<<endl;
	destroyWindow("win1");
	 // Matrix intrinsic -> K 
	 float intdata[] = {755.435, 0, 464.8648, 0, 566.50, 385.21,0, 0, 1};
	 intrinsic = Mat(3, 3, CV_32FC1,intdata).clone(); //initialisation
	 //intrinsic = Mat(3, 3, CV_32FC1); //initialisation
	 intrinsic.ptr<float>(0)[0] = 1;
     intrinsic.ptr<float>(1)[1] = 1;

	 

	 // Matrix extrinsic -> rotation and translation vector
     vector<Mat> rvecs; // -> R
     vector<Mat> tvecs; // -> t

	 try{
			//Now, execute the calibration. This will go in three steps: 1. compute intrinsic values. 2. estimate
			// camera pose. and 3. run an Levenberg-Marquardt algorithm to optimise the reprojection error.
		 calibrateCamera(physical_corners,image_corners, imgSize, intrinsic, distCoeffs, rvecs, tvecs);
			cout<<"Calibrated intrinsic: "<<endl<<intrinsic<<endl;
			cout << "Distortion matrix: " << endl << distCoeffs <<endl;
			return 1;
	 }catch(int exc){
			cout<<"Caught the exception with number"<<exc<<endl;
	 }
     return 0;
}

int cameraCalibration(vector<Point3f> phys_corners, Mat& intrinsic, Mat& distCoeffs)
{
	/* 2 Matrices of the physical positions of the corners
	   and the positions in the image.
	   physical_corners -> 3D
	   image_corners -> 2D
	*/
	vector<vector<Point3f>> physical_corners;
	vector<vector<Point2f>> image_corners;
	vector<Point2f> corners;
    int ok=0;
	Mat image,gray_image;
	int successes = 0;
	Size board_size = Size(nrHorizontalCorners,nrVerticalCorners);
	Size imgSize;

	for (int i=startNr;i<=endNr;i++)
	{
		// feed image
		String file = camPath+camNr+"/intrinsics"+imgName+to_string(i)+imgExt;
		image = imread(file);

		// process
		 Mat proc_img = image;
		  cvtColor(image, gray_image, CV_BGR2GRAY);
		  bool found = findChessboardCorners(proc_img, board_size, corners, 
								CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
		  
		  //cout<< "Processingimage!"<<endl;
		  // only proccess an image if there are chessboardcorners
		  
		  if(found)
		  {
			  cornerSubPix(gray_image, corners, Size(11, 11), Size(-1, -1),
					 TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
			  drawChessboardCorners(proc_img, board_size, corners, found);
			  imshow("win1",proc_img);	
			  
			  image_corners.push_back(corners);
			  physical_corners.push_back(phys_corners);
			  successes++;
		      cout<<"Snap stored! Successes: "<<successes<<"/"<<endNr<<endl;
			  successes++;
		  } else {
			imshow("win1", image);
		  }
		  
 
          int key = waitKey(1);
		  if(key==27) return 0;
      
	}

	 /*At this point, there are enough images to do the actual calibration.
	 * For this the intrinsic matrix K is initialised, and the output variables
	 * (rvecs, rotation matrices and tvecs, translation matrices) are created
	 */

	 cout<< "Video is done, let's calibrate!"<<endl;
	 destroyWindow("win1");
	 // Matrix intrinsic -> K 
	 intrinsic = Mat(3, 3, CV_32FC1); //initialisation
	 intrinsic.ptr<float>(0)[0] = 1;
     intrinsic.ptr<float>(1)[1] = 1;

	 // Matrix extrinsic -> rotation and translation vector
     vector<Mat> rvecs; // -> R
     vector<Mat> tvecs; // -> t

	 try{
			//Now, execute the calibration. This will go in three steps: 1. compute intrinsic values. 2. estimate
			// camera pose. and 3. run an Levenberg-Marquardt algorithm to optimise the reprojection error.
		 calibrateCamera(physical_corners,image_corners, imgSize, intrinsic, distCoeffs, rvecs, tvecs);
			cout<<"Calibrated intrinsic: "<<endl<<intrinsic<<endl;
			cout << "Distortion matrix: " << endl << distCoeffs <<endl;
			return 1;
	 }catch(int exc){
			cout<<"Caught the exception with number"<<exc<<endl;
	 }
     return 0;
}

/*ProcessImage. The function takes an images as an input from a calibrated camera, tries to detect a (part of a) 
*chessboard on it. The detection gives the corners of the squares on the chessboard in image coordinates. Next, the
* corners found on the image are matched to the physical_corners of the world, by using solePnP(). The result is used to 
* draw.
*
*
*Arguments
*   image		the image (with a chessboard), on which an axis system and a square is drawn.
*   physical_corners  the world coordinates of the to be discovered coordinates on the image. These are a given, based
*						upon the pattern that is looked for in the image.
*   intrinsic	the calibrated intrinsic camera matrix, given the transformation from camera coordinates.
*   distCoeffss	the distortion coefficient for the device that captured the image.
*
*/
void proccessImage(Mat image,vector<Point3f> physical_corners, Mat intrinsic, Mat distCoeffs){
	vector<Point2f> image_corners;
	Mat gray_image;
	cvtColor(image, gray_image, CV_BGR2GRAY);

	bool found = findChessboardCorners(gray_image, Size(nrHorizontalCorners,nrVerticalCorners), image_corners, CV_CALIB_CB_ADAPTIVE_THRESH+CV_CALIB_CB_FILTER_QUADS+CALIB_CB_FAST_CHECK);
			
	if(found)
	{
		Mat rvec,tvec,rmat;
		cornerSubPix(gray_image, image_corners, Size(11, 11), Size(-1, -1),
					 TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
		drawChessboardCorners(image, Size(nrHorizontalCorners,nrVerticalCorners), image_corners, found);
		
		//Solve the PnP problem: given both the 2D and 3D point, it gives the rotation and translation vector 
		//which transform the 3D point into a 2D point.
		solvePnP(physical_corners,image_corners,intrinsic,distCoeffs,rvec,tvec);
		Rodrigues(rvec,rmat); // get the Rvector as a R matrix of (3*3), whereas rvec is a 3*1 vector) 
		
		Mat Rtmat, Pmatrix;
		hconcat(rmat,tvec,Rtmat); // put R and t together -> [R t]
		Pmatrix = intrinsic*Rtmat; // NOTE: a 3x4 matrix

		drawing(Pmatrix,image_corners,image);//draw on image, given the Pmatrix.
		imshow("win1",image);
		cout<< "Save image?";
		int key=waitKey(1);
		char s[12];
		char s1[20] = "screenshot";
		if(key==32){
			cout<<" yes, prease!";
			_itoa(imgCount++,s,10);
			cout<<"s:"<<s<<endl;
			strcat_s(s1,s);
			cout<<s1<<endl;
			strcat_s(s1,".jpg");
			imwrite(s1,image);
		}
		cout<<endl;
	}
}

int main()
{  
	/**Initilisation phase 
	* Initially, a video stream is set up using the built-in webcam.
	* In order to cope with some start-up problems with the webcam, a first image is opened
	* and then the program is continued.
	**/
	/* Video capture is the feed/stream of images from the camera */


	videoPath = "video.avi";
	cout<< camPath+camNr+"/"+videoPath <<endl;
	VideoCapture capture = VideoCapture(camPath+camNr+"/"+videoPath);
	capture.open(0);
	
	Mat image;
	/*capture>>image;
	cout<<"Please wait patiently until the webcam responds . . ."<<endl;
	for(int count=4;count>0;count--){
		cout<<count<<", ";
		Sleep(1000);
		capture>>image;
	}
	cout<<"0!"<<endl;
	/* END of initialisation */


	/* Set up initial, constant values of the world coordinates of the corners on the chessboard
	  We need the pattern for the number of squares that we assign the program to look for.
	  We also have to get the board size that we want our program to look for.
	*/
	vector<Point3f> physical_corners;
	int nrSquares = nrVerticalCorners*nrHorizontalCorners;
	for(int j=0;j<nrSquares;j++)
		physical_corners.push_back(Point3f(j/nrVerticalCorners, j%nrHorizontalCorners, 0.0f));
	
	/* Now calibrate the camera */
	Mat intrinsic, distCoeffs;
	cameraCalibration(capture,physical_corners,intrinsic,distCoeffs);
	cout<<"The camera is calibrated. Now, the drawing can commence!"<<endl;
	FILE *intrinz;
	intrinz = fopen("intrinsics.txt","w");
	for (int i=0; i<intrinsic.size().height;i++){
		for (int j=0; j<intrinsic.size().width;j++){
			fprintf(intrinz,"%f \t",&intrinsic.at<double>(i,j));
		}
		fprintf(intrinz,"\n");
	}
	fclose(intrinz);

	/*Now, for every next iteration use the next frame to find the chessboard, and draw a square on it*/
	capture= VideoCapture(videoPath);
	capture.open(0);
	capture>>image;
	while(!image.empty()){
		proccessImage(image,physical_corners,intrinsic,distCoeffs); // get rvec, tvec for the image and draw!
		imshow("win1",image);
		capture>>image;
		int key = waitKey(1);
		if (key==27) break;
	}
	
	waitKey(0);
    return 0;
}
