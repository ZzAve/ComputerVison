/*
 * VoxelReconstruction.cpp
 *
 *  Created on: Nov 13, 2013
 *      Author: coert
 */

#include "VoxelReconstruction.h"

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

vector<int> getBestHsv(Camera *cam,Scene3DRenderer scene, Mat photoshoped,int cameraNr)
{
	vector<int> hsv;
	scene.setCamera(cameraNr);
	for(int h=0;h<255;h++)
		for(int s=0;s<255;h++)
			for(int v=0;v<255;v++)
			{
				scene.setHThreshold(h);
				scene.setSThreshold(s);
				scene.setVThreshold(v);
				Mat foreground = cam->getForegroundImage();

			}

	return hsv;

}


vector<vector<int>> determine_best_hsv(vector<Camera *> cams,Scene3DRenderer render)
{
	vector<vector<int>> hsv;
	int i,j,k;
	Mat photoshoped1,photoshoped2,photoshoped3,photoshoped4;

	for(i=0;i<4;i++)
	{

		if(i == 0)
		{
			vector<int> cam1;
			cam1 = getBestHsv(cams[i],render,photoshoped1,0);
			hsv.push_back(cam1);
		}
		else if(i == 1)
		{
			vector<int> cam2;
			cam2 = getBestHsv(cams[i],render,photoshoped2,1);
			hsv.push_back(cam2);
			
		}
		else if(i == 2)
		{
			vector<int> cam3;
			cam3 = getBestHsv(cams[i],render,photoshoped3,2);
			hsv.push_back(cam3);
		}
		else if(i == 3)
		{
			vector<int> cam4;
			cam4 = getBestHsv(cams[i],render,photoshoped4,3);
			hsv.push_back(cam4);
		}
	
	}
	return hsv;
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
	//scene3d.setCamera(0);
	//scene3d.setCurrentFrame(135);

	// Nasty part
	// ==================================================
	// Camera 4 -> 128
	// Canera 1 -> 135
	// Camera 2 -> 160
	// Camera 3 -> 

	Vector<Camera*> cams = scene3d.getCameras();
	//cams[0]->setVideoFrame(135);
	//scene3d.processForeground(cams[0]);
	imwrite("camera1.jpg",cams[0]->getVideoFrame(135));
	imwrite("camera2.jpg",cams[1]->getVideoFrame(160));
	imwrite("camera3.jpg",cams[2]->getVideoFrame(150));
	imwrite("camera4.jpg",cams[3]->getVideoFrame(128));
	//scene3d.processForeground(cams[0]);
	imwrite("foregroundCamera1.jpg",cams[0]->getForegroundImage());
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
