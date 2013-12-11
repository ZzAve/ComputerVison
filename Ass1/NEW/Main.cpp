//#include <iostream>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv\cv.h>
//
//using namespace cv;
//using namespace std;
//
//int numBoards = 3;
//int nrHorizontalCorners = 3;
//int nrVerticalCorners = 3;
//
//
//class Line{
//
//	int RED;
//	int BLUE;
//	int GREEN;
//	int ORANGE;
//	Point startPoint,stopPoint;
//	int thikness,lineType;
//	Mat image;
//
//	public:Line(Mat img,Point start, Point end)
//	{
//		Line::startPoint = start;
//		Line::stopPoint = end;
//		Line::image = img;
//		thikness = 1;
//		lineType = 1;
//		RED = 0;
//		GREEN = 1;
//		BLUE = 2;
//		ORANGE = 3;
//	}
//	
//	void setStart(Point start)
//	{
//		Line::startPoint = start;
//	}
//
//	void setStop(Point stop)
//	{
//		Line::stopPoint = stop;
//	}
//
//	void draw(int color)
//	{
//		int thickness = 2;
//		int lineType = 8;
//
//		if(color == BLUE)
//		{
//			line(	image,
//					startPoint,
//					stopPoint,
//					Scalar( 0, 0, 255),
//					thickness,
//					lineType 
//				);
//
//		}else if(color == RED){
//			line(	image,
//					startPoint,
//					stopPoint,
//					Scalar( 255,0, 0 ),
//					thickness,
//					lineType 
//				);
//
//		}else if(color == GREEN){
//			line(	image,
//					startPoint,
//					stopPoint,
//					Scalar( 0, 255, 0 ),
//					thickness,
//					lineType 
//				);
//
//		}else if(color == ORANGE){
//			line(	image,
//					startPoint,
//					stopPoint,
//					Scalar( 0, 0, 0),
//					thickness,
//					lineType 
//				);
//		}
//		
//	}
//
//};
//
//
//void matrixMultiplication(double a[10][10],double b[10][10],double multiplicationStorage[10][10],int aNrLines, int aNrCols,int bNrLines,int bNrCols)
//{
//	
//	int i,j,k;
//
//	for(i=0;i<aNrLines;i++)
//		for(j=0;j<bNrCols;j++)
//				multiplicationStorage[i][j] = 0;
//
//	for(i=0;i<aNrLines;i++)
//		for(j=0;j<bNrCols;j++)
//			for(k=0;k<bNrLines;k++)
//				multiplicationStorage[i][j] += a[i][k]*b[k][j];
//	/*
//	cout<<"matrixMultiplication:"<<endl;
//	cout<<"================================="<<endl;
//	for(i=0;i<aNrLines;i++)
//	{
//		for(j=0;j<bNrCols;j++)
//				cout<<multiplicationStorage[i][j]<<" ";
//		cout<<endl;
//	} 
//	cout<<"=================================="<<endl;
//	*/
//}
//
//* Implements the projection transformation P=K[R|t];*/
//Point transformRKT(Point3f point, Mat intrinsic, Mat rotation, Mat translation)
//{
//	int i,j;
//	double pointB[10][10],pointC[10][10];
//	double intrinsicB[10][10];
//	double rotationB[10][10],translationB[10][10];
//	double RTMatrix[10][10];
//	double P[10][10];
//	
//	for(i=0;i<intrinsic.rows;i++)
//		for(j=0;j<intrinsic.cols;j++)
//			intrinsicB[i][j] =  intrinsic.at<double>(i,j);
//
//	for(i=0;i<rotation.rows;i++)
//		for(j=0;j<rotation.cols;j++)
//			rotationB[i][j] = rotation.at<double>(i,j);
//
//	for(i=0;i<rotation.rows;i++)
//		for(j=0;j<rotation.cols;j++)
//			rotationB[i][j] = rotation.at<double>(i,j);
//
//	for(i=0;i<translation.rows;i++)
//		for(j=0;j<translation.cols;j++)
//			translationB[i][j] = translation.at<double>(i,j);
//
//	for(i=0;i<3;i++)
//		for(j=0;j<3;j++)
//			RTMatrix[i][j] = rotationB[i][j];
//	
//	for(i=0;i<3;i++)
//			RTMatrix[i][3] = translationB[i][0];
//
//
//	matrixMultiplication(intrinsicB,RTMatrix,P,3,3,3,4);
//
//	
//	cout<<"transformRKT"<<endl;
//	cout<<"================================="<<endl;
//	for(i=0;i<3;i++)
//	{
//		for(j=0;j<4;j++)
//				cout<<P[i][j]<<" ";
//		cout<<endl;
//	} 
//	cout<<"=================================="<<endl;
//	
//	
//	pointB[0][0] = point.x;
//	pointB[1][0] = point.y;
//	pointB[2][0] = point.z;
//	pointB[3][0] = 1;
//	
//	matrixMultiplication(P,pointB,pointC,3,4,4,1);
//
//	cout<<"Transform rkt, the point is: ("<<pointC[0][0]<<" "<<pointC[0][1]<<")"<<endl;
//	//vector<Point3f> points = vector<Point3f>();
//	//points.push_back(point);
//	//projectPoints(points,rvec,tvec,intrinsic,distCoeffs,imgPoints);
//	return Point(pointC[0][0],pointC[0][1]);
//}
//
//
//
//Point3f inverseTransformRKT1(Point2f point, Mat intrinsic, vector<Mat> rvecs, vector<Mat> tvecs,Mat distCoeff)
//{
//	/*
//	vector<Point3f> vec = vector<Point3f>();
//	vector<Point2f> img = vector<Point2f>();
//	//img.push_back(Point2f(0,0));
//	vec.push_back(point);
//	projectPoints(vec,rvecs[rvecs.size()-1],tvecs[tvecs.size()-1],intrinsic,distCoeff,img);
//	return img.back();
//	*/
//	vector<Point2f> vec = vector<Point2f>();
//	vector<Point3f> img = vector<Point3f>();
//	//img.push_back(Point2f(0,0));
//	vec.push_back(point);
//	projectPoints(vec,rvecs[rvecs.size()-1],tvecs[tvecs.size()-1],intrinsic,distCoeff,img);
//	return img.front();
//}
//
//Point2f transformRKT1(Point3f point, Mat intrinsic, vector<Mat> rvecs, vector<Mat> tvecs,Mat distCoeff)
//{
//	
//	vector<Point3f> vec = vector<Point3f>();
//	vector<Point2f> img = vector<Point2f>();
//	//img.push_back(Point2f(0,0));
//	vec.push_back(point);
//	projectPoints(vec,rvecs[rvecs.size()-1],tvecs[tvecs.size()-1],intrinsic,distCoeff,img);
//	return img.front();
//	
//}
//
//* Function that draws the coordinate system and the cube :D */
//void draw(Mat image, Mat intrinsic, vector<Point2f>corners, Mat rotation, Mat translation)
//{
//	
//	Point point =  transformRKT(Point3f(0,0,3),intrinsic,rotation,translation);
//	Line OX = Line(image,corners[0],corners[nrHorizontalCorners-1]);
//	OX.draw(1);
//	Line OY = Line(image,corners[0],corners[6]);
//	OY.draw(2);
//	
//	Line OZ = Line(image,corners[0],point);
//	cout<<"<"<<point.x<<";"<<">"<<point.y<<endl;
//	OZ.draw(3);
//	
//}
//
//void draw1(Mat intrinsic,vector<Mat>rvecs,vector<Mat> tvecs,Mat distCoeff, vector<Point2f> corners, Mat image)
//{
//
//	//Point3f point3 = inverseTransformRKT1(corners[0],intrinsic,rvecs,tvecs,distCoeff);
//	
//	Point2f pnt1 = transformRKT1(Point3f(0.0,0.0,0.0),intrinsic,rvecs,tvecs,distCoeff);
//	Point2f pnt2 = transformRKT1(Point3f(0.0,1.0,0.0),intrinsic,rvecs,tvecs,distCoeff);
//	Point2f pnt3 = transformRKT1(Point3f(1.0,0.0,0.0),intrinsic,rvecs,tvecs,distCoeff);
//	Point2f pnt4 = transformRKT1(Point3f(0.0,0.0,1.0),intrinsic,rvecs,tvecs,distCoeff);
//	Line OX = Line(image,pnt1,pnt2);
//	OX.draw(1);
//	Line OY = Line(image,pnt1,pnt3);
//	OY.draw(2);
//	Line OZ = Line(image,pnt1,pnt4);
//	OZ.draw(3);
//	
//	vector<Point3f> axis = vector<Point3f>();
//	axis[0] = Point3f(0.0,0.0,0.0);
//	axis[1] = Point3f(1.0,0.0,0.0);
//	axis[2] = Point3f(0.0,1.0,0.0);
//	axis[4] = Point3f(0.0,0.0,1.0);
//
//
//	OX = Line(image,corners[0],corners[nrHorizontalCorners-1]);
//	OX.draw(0);
//
//	OY = Line(image,corners[0],corners[6]);
//	OY.draw(1);
//
//	Point2f point2 = transformRKT1(Point3f(0,0,1.0),intrinsic,rvecs,tvecs,distCoeff);
//	OZ = Line(image,corners[0],point2);
//	OZ.draw(2);
//	
//}
//
//
//
//
//void draw2(Mat intrinsic,vector<Mat> rvec,vector<Mat> tvec,Mat distCoeff, vector<Point2f> corners, Mat image)
//{
//
//	int i;
//	vector<Point3f> axis = vector<Point3f>();
//	vector<Point3f> cube = vector<Point3f>();
//	vector<Point2f> imgPoints = vector<Point2f>();
//	vector<Point2f> bottomFace = vector<Point2f>();
//	vector<Point2f> upperFace = vector<Point2f>();
//
//	// Here i draw the axis:
//	/*===================================================*/
//	axis.push_back(Point3f(0.0,0.0,0.0));
//	axis.push_back(Point3f(3.0,0.0,0.0));
//	axis.push_back(Point3f(0.0,3.0,0.0));
//	axis.push_back(Point3f(0.0,0.0,3.0));
//
//	Mat rotationMatrix(3,3,cv::DataType<double>::type);
//	Rodrigues(rvec[rvec.size()-1],rotationMatrix);
//	projectPoints(axis, rotationMatrix, tvec[tvec.size()-1], intrinsic, distCoeff,imgPoints);
//
//	Line OX = Line(image,corners[0],imgPoints[1]);
//	OX.draw(0);
//
//	Line OY = Line(image,corners[0],imgPoints[2]);
//	OY.draw(1);
//
//	Line OZ = Line(image,corners[0],imgPoints[3]);
//	OZ.draw(2);
//	/*======================================================*/
//
//	// Here I draw the cube:
//	/*======================================================*/
//	// Bottom face:
//	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	axis.clear();
//	axis.push_back(Point3f(2.0,0.0,0.0));
//	axis.push_back(Point3f(2.0,2.0,0.0));
//	axis.push_back(Point3f(0.0,2.0,0.0));
//	bottomFace.push_back(corners[0]);
//	projectPoints(axis, rotationMatrix, tvec[tvec.size()-1], intrinsic, distCoeff,imgPoints);
//	for(i=0;i<imgPoints.size();i++)
//		bottomFace.push_back(imgPoints[i]);
//	for(i=0;i<bottomFace.size()-1;i++)
//	{
//		Line faceLine = Line(image,bottomFace[i],bottomFace[i+1]);
//		faceLine.draw(3);
//	}
//	Line faceLine = Line(image,bottomFace[i],corners[0]);
//	faceLine.draw(3);
//	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//	
//	// Upper face:
//	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	axis.clear();
//	axis.push_back(Point3f(0.0,0.0,2.0));
//	axis.push_back(Point3f(2.0,0.0,2.0));
//	axis.push_back(Point3f(2.0,2.0,2.0));
//	axis.push_back(Point3f(0.0,2.0,2.0));
//	projectPoints(axis, rotationMatrix, tvec[tvec.size()-1], intrinsic, distCoeff,imgPoints);
//	for(i=0;i<imgPoints.size();i++)
//		upperFace.push_back(imgPoints[i]);
//	for(i=0;i<upperFace.size()-1;i++)
//	{
//		Line faceLine = Line(image,upperFace[i],upperFace[i+1]);
//		faceLine.draw(3);
//	}
//	faceLine = Line(image,upperFace[i],upperFace[0]);
//	faceLine.draw(3);
//	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	
//	//Other faces:
//	for(i=0;i<bottomFace.size();i++)
//	{
//		faceLine = Line(image,bottomFace[i],upperFace[i]);
//		faceLine.draw(3);
//	}
//	/*======================================================*/
//}
//
//int cameraCalibration()
//{
//	/* In this function I am calibrating the camera with a chessboard image. After the intrinsics
//	and extrinsics are obtained I send them to a drawing function that implements the equation
//	K[R|t]. 
//	*/
//
//	/*
//	cout<<"Horizontal Corners"<<endl;
//	cin>>nrHorizontalCorners;
//	cout<<"Vertical Corners: "<<endl;
//	cin>>nrVerticalCorners;
//	cout<<"Number of boards you want"<<endl;
//	cin>>numBoards;
//	*/
//	/*  We need the pattern for the number of squares that we assign the program to look for.
//	    We also have to get the board size that we want our program to look for.
//	*/
//	int nrSquares = nrHorizontalCorners * nrVerticalCorners;
//	Size board_size = Size(nrHorizontalCorners,nrVerticalCorners);
//	
//
//	/* Video capture is the feed/stream of images from the camera */
//	VideoCapture capture = VideoCapture(0);
//	capture.open(0);
//	Mat img;
//	capture>>img;
//	imshow("win1",img);
//	cout<<"Please wait patiently until the webcam responds"<<endl;
//	int key =waitKey(4000);
//	capture>>img;
//	imshow("win1",img);
//	/* 2 Matrices of the physical positions of the corners
//	   and the positions in the image.
//	   physical_corners -> 3D
//	   image_corners -> 2D
//	*/
//
//	vector<vector<Point3f>> physical_corners;
//	vector<vector<Point2f>> image_corners;
//	vector<Point2f> corners;
//    int ok=0;
//	Mat image;
//    Mat gray_image;
//	int successes = 0;
//
//	vector<Point3f> obj;
//    for(int j=0;j<nrSquares;j++)
//        obj.push_back(Point3f(j/nrVerticalCorners, j%nrHorizontalCorners, 0.0f));
//    capture >> image;
//	while(successes<numBoards)
//    {
//		  cout<<"Successes: "<<successes<<endl;
//		 // break;
//		  cvtColor(image, gray_image, CV_BGR2GRAY);
//		  bool found = findChessboardCorners(image, board_size, corners, CV_CALIB_CB_ADAPTIVE_THRESH|CV_CALIB_CB_FILTER_QUADS);// CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
//		  if(found)
//		  {
//			  drawChessboardCorners(image, board_size, corners, found);
//          }
//
//		  imshow("win1", image);
//
//		  capture >> image;
// 
//          int key = waitKey(1);
//		  if(key==27) return 0;
// 
//		 if(found)
//		 {
//			image_corners.push_back(corners);
//            physical_corners.push_back(obj);
//            printf("Snap stored!\n");
// 
//            successes++;
// 
//            if(successes>=numBoards)
//                break;
//          }
//      }
//
//	 // Matrix intrinsic -> K 
//	 Mat intrinsic = Mat(3, 3, CV_32FC1);
//     Mat distCoeffs;
//
//	 // Matrix extrinsic -> rotation and translation vector
//     vector<Mat> rvecs; // -> R
//     vector<Mat> tvecs; // -> T
//	 Mat rvec,tvec;
//	 intrinsic.ptr<float>(0)[0] = 1;
//     intrinsic.ptr<float>(1)[1] = 1;
//
//	 Mat rotationMatrix(3,3,cv::DataType<double>::type);
//	 Mat translationMatrix(3,3,cv::DataType<double>::type);
//
//	 cout<<"Initial intrinsic: "<<intrinsic<<endl;
//	 try{
//			calibrateCamera(physical_corners,image_corners, image.size(), intrinsic, distCoeffs, rvecs, tvecs);
//			//cout<<"Intrinsic: "<<intrinsic<<endl;
//			Rodrigues(rvecs[0],rotationMatrix);
//			cout<<rotationMatrix<<endl;
//			Mat imageUndistorted;
//			while(1)
//			{
//
//					capture >> image;
//					undistort(image, imageUndistorted, intrinsic, distCoeffs);
//					bool found = findChessboardCorners(image, board_size, corners, CV_CALIB_CB_ADAPTIVE_THRESH|CV_CALIB_CB_FILTER_QUADS);
//					if(found)
//					{
//						image_corners.push_back(corners);
//						physical_corners.push_back(obj);
//						calibrateCamera(physical_corners,image_corners, image.size(), intrinsic, distCoeffs, rvecs, tvecs);
//						//solvePnPRansac(physical_corners,image_corners,intrinsic,distCoeffs,rvecs,tvecs);
//						//draw(image,intrinsic,corners,rotationMatrix,tvecs[tvecs.size()-1]);
//					//	draw1(intrinsic,rvecs,tvecs,distCoeffs,corners,image);
//				//		Rodrigues(rvecs[rvecs.size()-1],rotationMatrix);
//						//Rodrigues(tvecs[tvecs.size()-1],translationMatrix);
//						cout<<"Rvecs : "<<rvecs[0]<<" tvecs : "<<tvecs[0];
//						draw2(intrinsic,rvecs,tvecs,distCoeffs,corners,image);
//						//drawChessboardCorners(image, board_size, corners, found);
//					}
//				    imshow("win1", image);
//					//imshow("win2", imageUndistorted);
//					waitKey(1);
//			}
//			capture.release();
//		
//	 }catch(int exc){
//			cout<<"Caught the exception with number"<<exc<<endl;
//	 }
//	 
//	
//     return 0;
//}
//
//int main()
//{  
//    RNG rng;
//   // Mat image = cv::imread("d:/a.jpg",-1);
//    //imshow("Image",image);
//	//theBrain();
//	cameraCalibration();
//	//bool found = findChessboardCorners(image,boardsize,ptvec, CV_CALIB_CB_ADAPTIVE_THRESH );
//    waitKey(0);
//    return 0;
//}