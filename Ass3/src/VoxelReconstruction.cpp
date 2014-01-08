/*
 * VoxelReconstruction.cpp
 *
 *  Created on: Nov 13, 2013
 *      Author: coert
 */

#include "VoxelReconstruction.h"
#include <math.h>

using namespace nl_uu_science_gmt;
using namespace std;
using namespace cv;

namespace nl_uu_science_gmt
{

/**
 * Main constructor, initialized all cameras
 */
VoxelReconstruction::VoxelReconstruction(const string &dp, const int cva) :
		_data_path(dp), _cam_views_amount(cva)
{
	const string cam_path = _data_path + "cam";

	for (int v = 0; v < _cam_views_amount; ++v)
	{
		stringstream full_path;
		full_path << cam_path << (v + 1) << PATH_SEP;
		assert(
				(General::fexists(full_path.str() + General::BackgroundImageFile) || General::fexists(
							full_path.str() + General::BackgroundVideoFile))
				&& General::fexists(full_path.str() + General::VideoFile));
		assert(
				!General::fexists(full_path.str() + General::ConfigFile) ?
						General::fexists(full_path.str() + General::IntrinsicsFile) && General::fexists(
								full_path.str() + General::CheckerboadVideo) :
						true);
		_cam_views.push_back(new Camera(full_path.str(), General::ConfigFile, v));
	}
}

/**
 * Main destructor, cleans up pointer vector memory of the cameras
 */
VoxelReconstruction::~VoxelReconstruction()
{
	for (size_t v = 0; v < _cam_views.size(); ++v)
		delete _cam_views[v];
}

/**
 * What you can hit
 */
void VoxelReconstruction::showKeys()
{
	cout << "VoxelReconstruction v" << VERSION << endl << endl;
	cout << "Use these keys:" << endl;
	cout << "q       : Quit" << endl;
	cout << "p       : Pause" << endl;
	cout << "b       : Frame back" << endl;
	cout << "n       : Next frame" << endl;
	cout << "r       : Rotate voxel space" << endl;
	cout << "s       : Show/hide arcball wire sphere (Linux only)" << endl;
	cout << "v       : Show/hide voxel space box" << endl;
	cout << "g       : Show/hide ground plane" << endl;
	cout << "c       : Show/hide cameras" << endl;
	cout << "i       : Show/hide camera numbers (Linux only)" << endl;
	cout << "o       : Show/hide origin" << endl;
	cout << "t       : Top view" << endl;
	cout << "1,2,3,4 : Switch camera #" << endl << endl;
	cout << "Zoom with the scrollwheel while on the 3D scene" << endl;
	cout << "Rotate the 3D scene with left click+drag" << endl << endl;
}

/*
*		In this function we compare 2 matrixes pixel by pixel in the rgb range. It gives the number of 
*	similar pixels between the 2 images which are used to maximize similarity between the foreground
*	image that we create with processForeground() and the one we photoshoped.
*/

int VoxelReconstruction::compare_images(Mat foreground, Mat photoshoped)
{
	double blueF, greenF, redF, blueP, greenP, redP;
	int record = 0;
	//cout<<"Compare images"<<endl;
	/*if(foreground.rows != photoshoped.rows || foreground.cols != photoshoped.cols)
	{
			cout<<"Different dimensions for the 2 images.. Can't compuuuute.... Can't compuuuuteeee...."<<endl;
			return  -1;
	}*/
	//cout<<"The image size is: "<<foreground.rows<<" by "<<foreground.cols<<endl;
	
   // #pragma omp parallel shared(foreground,background) private(i,j)
	//{
		//#pragma omp for schedule(dynamic,1) nowait
		for(int i=0; i<foreground.rows; i++)	
			for(int j=0; j<foreground.cols; j++)
			{
				//cout<<"Working ... "<<endl;
				Vec3f intensity = foreground.at<Vec3f>(j, i);
				blueF = floor(intensity[0]);
				greenF = floor(intensity[1]);
				redF = floor(intensity[2]);
				
				cout<<blueF<<" "<<greenF<<" "<<redF<<endl;
					
				intensity = photoshoped.at<Vec3f>(j, i);
				blueP = floor(intensity[0]);
				greenP = floor(intensity[1]);
				redP = floor(intensity[2]);
				cout<<blueP<<" "<<greenP<<" "<<redP<<endl;
				waitKey();
				if( ((blueF != 0 && greenF != 0 && redF !=0 ) &&  
					(blueP != 0 && greenP != 0 && redP !=0 )) || 
					((blueF == 0 && greenF == 0 && redF == 0 ) &&  
					(blueP == 0 && greenP == 0 && redP == 0 ))
				)
				record ++;
			}
	 //}
	return record;

}

/*
*	* getBestHsv ranges between 0 and 255 for the values h,s and v. For every value of h,s and v we calculate
* as foreground. With the foreground we compare it with the photoshoped image to see the similarities. The one 
* that maximizez the similarities is chosen to be the best hsv value.
*    * We set the values h,s and v in the scene3DRenderer and we create the foregound with the porcess foreground
* function.
*/
vector<int> VoxelReconstruction::getBestHsv(Camera *cam,Scene3DRenderer scene, Mat photoshoped,int cameraNr)
{
	int ok, max_similarity;
	vector<int> hsv;
	int bestH = -1,bestS = -1,bestV = -1;
	scene.setCamera(cameraNr);
	scene.setCurrentFrame(150);
	imwrite("test1.jpg",cam->getFrame());
	//imwrite("test2.jpg",photoshoped);
	cout<<"Working:";
	Mat foreground, foreground2;
	for(int h=5;h<50;h++)
		for(int s=5;s<70;s++)
		{
			cout<<h <<"_"<<s<<", ";
			for(int v=0;v<10;v++)
			{
				//cout<<"h : "<<h<<" s : "<<s<<" v : "<<v<<endl;
				scene.setHThreshold(h);
				scene.setSThreshold(s);
				scene.setVThreshold(v);
				scene.processForeground(cam);
				max_similarity = 0;
				Mat foreground = cam->getForegroundImage().clone();
				//cvtColor(foreground,foreground2,CV_8U);
				ok = compare_images(foreground,photoshoped);
				if(ok > max_similarity)
				{
					bestH = h;
					bestS = s;
					bestV = v;
					max_similarity = ok;
				}
			}
		}
	cout<<endl;

	hsv.push_back(bestH);
	hsv.push_back(bestS);
	hsv.push_back(bestV);
	cout<<"Here!"<<endl;
	cout<<hsv[0]<<endl<<hsv[1]<<endl<<hsv[2]<<endl;
	return hsv;

}

/*
*	    Determine_best_hsv was initially designed to go through all the 4 cameras to
*	determine the best image , but considering that the h,s,w values are the same for
*	every camera we decided to use just one of the cameras.
*
*/
vector<int> VoxelReconstruction::determine_best_hsv(vector<Camera *> cams,Scene3DRenderer render)
{
	
	int i,j,k;
	Mat photoshoped, photoshoped2;
    photoshoped = imread("photoshoped.jpg", CV_LOAD_IMAGE_COLOR);
	//cvtColor(photoshoped,photoshoped2,CV_8U);

	vector<int> cam;
	cam = getBestHsv(cams[2],render,photoshoped,3);
	cout<<"Determined best HSVs"<<endl;
	return cam;
	/*
	if(cam.size() != 0)
		hsv.push_back(cam);
	/*
	for(i=0;i<4;i++)
	{

		if(i == 0)
		{
			vector<int> cam1;
			cam1 = getBestHsv(cams[i],render,photoshoped1,1);
			if(cam1.size() != 0)
				hsv.push_back(cam1);
		}
		else if(i == 1)
		{
			vector<int> cam2;
			cam2 = getBestHsv(cams[i],render,photoshoped2,2);
			if(cam2.size() != 0)
				hsv.push_back(cam2);
			
		}
		else if(i == 2)
		{
			vector<int> cam3;
			cam3 = getBestHsv(cams[i],render,photoshoped3,3);
			if(cam3.size() != 0)
				hsv.push_back(cam3);
		}
		else if(i == 3)
		{
			vector<int> cam4;
			cam4 = getBestHsv(cams[i],render,photoshoped4,4);
			if(cam4.size() != 0)
				hsv.push_back(cam4);
		}
	
	}*/
}



/**
 * - If the xml-file with camera intrinsics, extrinsics and distortion is missing,
 *   create it from the checkerboard video and the measured camera intrinsics
 * - After that initialize the scene rendering classes
 * - Run it!
 */
void VoxelReconstruction::run(int argc, char** argv)
{
	for (int v = 0; v < _cam_views_amount; ++v)
	{
		bool has_cam = Camera::detExtrinsics(_cam_views[v]->getDataPath(), General::CheckerboadVideo,
				General::IntrinsicsFile, _cam_views[v]->getCamPropertiesFile());
		if (has_cam) has_cam = _cam_views[v]->initialize();
		assert(has_cam);
	}

	destroyAllWindows();
	namedWindow(VIDEO_WINDOW, CV_WINDOW_KEEPRATIO);


	Reconstructor reconstructor(_cam_views);
	Scene3DRenderer scene3d(reconstructor, _cam_views);
	
	// Nasty part
	// ==================================================
	// Camera 4 -> 128
	// Canera 1 -> 135
	// Camera 2 -> 160
	// Camera 3 -> 150

	vector<Camera*> cams = scene3d.getCameras();
	//cams[0]->setVideoFrame(135);
	imwrite("camera1.jpg",cams[0]->getVideoFrame(135));
	imwrite("camera2.jpg",cams[1]->getVideoFrame(160));
	imwrite("camera3.jpg",cams[2]->getVideoFrame(150));
	imwrite("camera4.jpg",cams[3]->getVideoFrame(128));
	vector<int> hsv;
	//hsv = VoxelReconstruction::determine_best_hsv(cams,scene3d);
	/*
	cout<<"setting settings"<<endl;
	scene3d.setHThreshold(hsv.at(0));
	scene3d.setSThreshold(hsv.at(1));
	scene3d.setVThreshold(hsv.at(2));
	cout<< "SETTINGS:" <<endl;
	cout<<hsv.at(0)<<endl;
	cout<<hsv.at(1)<<endl;
	cout<<hsv.at(2)<<endl;
	//determine_best_hsv(cams);
	/*
	for(int i=0;i<4;i++)
	{
		if(i == 1)
		{
			cams[1]->setVideoFrame(135);
			bestHsv(cams[1]->getVideoFrame(135));
		}
	}
	*/
	/*
	Vector<Camera*> cams = scene3d.getCameras();
	for(int i=0;i<cams.size();i++)
	{
		scene3d.setCurrentCamera(i);
		cout<<"Reconstructing camera "<<i<<" "<<endl;
		cout<<"======================================"<<endl;
		for(int j=0;j<scene3d.getNumberOfFrames()-1;j++)
		{
			scene3d.setCurrentFrame(j);
			//Mat frame = cams[i]->getFrame();
			Mat foreground = cams[i]->getForegroundImage();
			vector<Mat> hsv = cams[i]->getBgHsvChannels();
			int h = scene3d.getHThreshold();
			int s = scene3d.getSThreshold();
			int v = scene3d.getVThreshold();
			cout<<"The matrix is"<<foreground<<endl;
			cout<<"H = "<<h<<" S = "<<s<<" V = "<<v<<endl;
			cout<<"Going through frame "<<j<<endl;
		}
		cout<<"======================================="<<endl;
	}
	cout<<"Ended going through the images";
	// ==================================================
	*/
	Glut glut(scene3d);

#ifdef __linux__
	glut.initializeLinux(SCENE_WINDOW.c_str(), argc, argv);
#elif defined _WIN32
	glut.initializeWindows(SCENE_WINDOW.c_str());
	glut.mainLoopWindows();
#endif
	 
}

} /* namespace nl_uu_science_gmt */

int main(int argc, char** argv)
{
	VoxelReconstruction::showKeys();
	VoxelReconstruction vr("data" + string(PATH_SEP), 4);
	vr.run(argc, argv);

	return EXIT_SUCCESS;
}
