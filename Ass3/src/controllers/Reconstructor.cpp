/*
 * Reconstructor.cpp
 *
 *  Created on: Nov 15, 2013
 *      Author: coert
 */

#include "Reconstructor.h"

using namespace std;
using namespace cv;

namespace nl_uu_science_gmt
{

/**
 * Voxel reconstruction class
 */
Reconstructor::Reconstructor(const vector<Camera*> &cs) :
		_cameras(cs)
{
	for (size_t c = 0; c < _cameras.size(); ++c)
	{
		if (_plane_size.area() > 0)
			assert(_plane_size.width == _cameras[c]->getSize().width && _plane_size.height == _cameras[c]->getSize().height);
		else
			_plane_size = _cameras[c]->getSize();
	}

	_step = 32;
	_size = 512;
	const size_t h_edge = _size * 4;
	const size_t edge = 2 * h_edge;
	_voxels_amount = (edge / _step) * (edge / _step) * (h_edge / _step);

	initialize();
}

/**
 * Free the memory of the pointer vectors
 */
Reconstructor::~Reconstructor()
{
	for (size_t c = 0; c < _corners.size(); ++c)
		delete _corners.at(c);
	for (size_t v = 0; v < _voxels.size(); ++v)
		delete _voxels.at(v);
}

/**
 * Create some Look Up Tables
 * 	- LUT for the scene's box corners
 * 	- LUT with a map of the entire voxelspace: point-on-cam to voxels
 * 	- LUT with a map of the entire voxelspace: voxel to cam points-on-cam
 */
void Reconstructor::initialize()
{
	const int h_edge = _size * 4;
	const int xL = -h_edge;
	const int xR = h_edge;
	const int yL = -h_edge;
	const int yR = h_edge;
	const int zL = 0;
	const int zR = h_edge;

	// Save the volume corners
	// bottom
	_corners.push_back(new Point3f((float) xL, (float) yL, (float) zL));
	_corners.push_back(new Point3f((float) xL, (float) yR, (float) zL));
	_corners.push_back(new Point3f((float) xR, (float) yR, (float) zL));
	_corners.push_back(new Point3f((float) xR, (float) yL, (float) zL));

	// top
	_corners.push_back(new Point3f((float) xL, (float) yL, (float) zR));
	_corners.push_back(new Point3f((float) xL, (float) yR, (float) zR));
	_corners.push_back(new Point3f((float) xR, (float) yR, (float) zR));
	_corners.push_back(new Point3f((float) xR, (float) yL, (float) zR));

	cout << "Initializing voxels";

	// Acquire some memory for efficiency
	_voxels.resize(_voxels_amount);

#ifdef PARALLEL_PROCESS
#pragma omp parallel for //schedule(static, 1)
#endif
	for (int z = zL; z < zR; z += _step)
	{
		cout << "." << flush;

		for (int y = yL; y < yR; y += _step)
		{
			for (int x = xL; x < xR; x += _step)
			{
				Voxel* voxel = new Voxel;
				voxel->x = x;
				voxel->y = y;
				voxel->z = z;
				voxel->color = -1;
				voxel->camera_projection = vector<Point>(_cameras.size());
				voxel->valid_camera_projection = vector<int>(_cameras.size(), 0);

				const int zp = ((z - zL) / _step);
				const int yp = ((y - yL) / _step);
				const int xp = ((x - xL) / _step);
				const int plane_y = (yR - yL) / _step;
				const int plane_x = (xR - xL) / _step;
				const int plane = plane_y * plane_x;
				const int p = zp * plane + yp * plane_x + xp;  // The voxel's index

				for (size_t c = 0; c < _cameras.size(); ++c)
				{
					Point point = _cameras[c]->projectOnView(Point3f((float) x, (float) y, (float) z));

					// Save the pixel coordinates 'point' of the voxel projections on camera 'c'
					voxel->camera_projection[(int) c] = point;

					if (point.x >= 0 && point.x < _plane_size.width && point.y >= 0 && point.y < _plane_size.height)
						voxel->valid_camera_projection[(int) c] = 1;
				}

				//'p' is not critical as it's unique
				_voxels[p] = voxel;
			}
		}
	}

	cout << "done!" << endl;
}

/**
 * Count the amount of camera's each voxel in the space appears on,
 * if that amount equals the amount of cameras, add that voxel to the
 * visible_voxels vector
 *
 * Optimized by inverting the process (iterate over voxels instead of camera pixels for each camera)
 */
void Reconstructor::update()
{
	_visible_voxels.clear();

#ifdef PARALLEL_PROCESS
#pragma omp parallel for //schedule(static, 1)
#endif
	for (size_t v = 0; v < _voxels_amount; ++v)
	{
		int camera_counter = 0;
		Voxel* voxel = _voxels[v];

		for (size_t c = 0; c < _cameras.size(); ++c)
		{
			if (voxel->valid_camera_projection[c])
			{
				const Point point = voxel->camera_projection[c];

				//If there's a white pixel on the foreground image at the projection point, add the camera
				if (_cameras[c]->getForegroundImage().at<uchar>(point) == 255) ++camera_counter;
			}
		}

		// If the voxel is present on all cameras
		if (camera_counter == _cameras.size())
		{
#ifdef PARALLEL_PROCESS
#pragma omp critical //push_back is critical
#endif
			_visible_voxels.push_back(voxel);
		}
	}
	
	//Calculate the labels and stuff
	/*Mat labels;
	calculatekMeans(labels);
	cout<<"Labels"<<endl<<labels<<endl<<endl;*/
}

/* calculatekMeans, clusters the currently visible voxels, based upon the top view, 
	meaning that only x and y coordinates are considered.
*/
Mat Reconstructor::calculatekMeans(Mat &bestLabels)
{
	vector<Voxel*> visVox = getVisibleVoxels();
	Mat points;
	double x,y;
	for (int i=0;i<visVox.size();i++)
	{
		x = visVox.at(i)->x;
		y = visVox.at(i)->y;
		Point2f point = Point2f(x,y);
		points.push_back(point);
	}

	//Apply KMeans
	//double kmeans(InputArray data, int K, InputOutputArray bestLabels, 
	//				TermCriteria criteria, int attempts, int flags, OutputArray centers=noArray() )
	Mat kCenters;

	//Define criteria = ( type, max_iter = 10 , epsilon = 1.0 )
	TermCriteria criteria = TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0);

	//Set flags (Just to avoid line break in the code)
	int flags = KMEANS_RANDOM_CENTERS;

	//Perform kMeans
	kmeans(points,4,bestLabels,criteria,100,flags,kCenters);
	
	// sort the labels based on the centers
	/*
	Mat sortedCenters;
	sortIdx(kCenters,sortedCenters,CV_SORT_EVERY_COLUMN+CV_SORT_ASCENDING);

	for (int labelIndex=0; labelIndex<bestLabels.size().height; labelIndex++)
	{
		Point2f center = kCenters.at<Point2f>(bestLabels.at<int>(labelIndex));
		for (int i=1; i<sortedCenters.size().height;i++)
		{
			if (center.x == sortedCenters.at<Point2f>(i).x && center.y == sortedCenters.at<Point2f>(i).y)
			{
				cout<<"Here too"<<endl;
				bestLabels.at<int>(labelIndex)=i;
				break;
			}
		}
	}*/
	return kCenters;
}


vector<vector<vector<Point2f>>> Reconstructor::reprojectVoxels(Mat labels)
{
	// if points with the same coordinates, only take the one that is closest
	vector<Voxel*> voxels = getVisibleVoxels();
	int bx,by,kx,ky;
	Point3f camlocation;
	float distBase, distK;
	//check each view
	for (int c=0; c<voxels[0]->camera_projection.size();c++)
	{
		// compare every entry	
		for (int base=0; base<voxels.size()-1;base++)
		{
			if (voxels[base]->valid_camera_projection[c] == 1)
			{
				for (int k=base+1; k<voxels.size(); k++)
				{
					if (voxels[k]->valid_camera_projection[c] ==1)
					{
						//compare the two voxel projections
						bx = (voxels[base] -> camera_projection[c]).x;
						by = (voxels[base] -> camera_projection[c]).y;
						kx = (voxels[k] -> camera_projection[c]).x;
						ky = (voxels[k] -> camera_projection[c]).y;	

						if (bx==kx && by==ky)
						{
							//two equal points. Now include the one closest to the camera
							// get camera c
							// get distances
							 camlocation = getCameras()[c] -> getCameraLocation();
							
							distBase = (camlocation.x - voxels[base]->z )*(camlocation.x - voxels[base]->z )+
							(camlocation.y - voxels[base]->y )*(camlocation.y - voxels[base]->y )+
							(camlocation.z - voxels[base]->z )*(camlocation.z - voxels[base]->z );

							distK = (camlocation.x - voxels[k]->z )*(camlocation.x - voxels[k]->z )+
							(camlocation.y - voxels[k]->y )*(camlocation.y - voxels[k]->y )+
							(camlocation.z - voxels[k]->z )*(camlocation.z - voxels[k]->z );

							if (distBase >= distK)						
								voxels[base] -> valid_camera_projection[c] = 0;
							else 
								voxels[k] -> valid_camera_projection[c] = 0;

						}
					}
				} // end base if
			} // end base loop
		}
	}
	vector<vector<vector<Point2f>>> imgPoints;
	vector<vector<Point2f>> points(4);
	for (int c=0; c<voxels[0]->camera_projection.size(); c++)
	{
			for (size_t v=0;v<voxels.size();v++)
			{
				if (voxels[v] -> valid_camera_projection[c] == 1)
					points[labels.at<int>(v)].push_back(voxels[v]->camera_projection[c]);
			}
			imgPoints.push_back(points);
	}
	
	return imgPoints;
}

} /* namespace nl_uu_science_gmt */
