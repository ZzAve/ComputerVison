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
				voxel->label = -1;
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
	initializeProjectableVoxels();
	initCenters();
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
void Reconstructor::calculatekMeans()
{
	vector<Voxel*> visVox = getVisibleVoxels();
	Mat points, bestLabels;
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
	vector<Point2f> kCenters(4);

	//Define criteria = ( type, max_iter = 10 , epsilon = 1.0 )
	TermCriteria criteria = TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0);

	//Set flags (Just to avoid line break in the code)
	int flags = KMEANS_RANDOM_CENTERS;

	//Perform kMeans
	Mat tempCenters;
	kmeans(points,4,bestLabels,criteria,100,flags,tempCenters);
	
	for (int i =0; i < tempCenters.rows; i++)
	{
		kCenters[i]=Point2f(tempCenters.at<float>(i,0),tempCenters.at<float>(i,1));
		cout << tempCenters.at<float>(i,0)<< " " << tempCenters.at<float>(i,1) << endl;
	}
	

	for (size_t v=0;v<visVox.size();v++)
	{
		visVox[v] ->label = bestLabels.at<int>(v);
	}
	setVisibleVoxels(visVox);

	setCenters(kCenters);
}


vector<vector<vector<Point2f>>> Reconstructor::reprojectVoxels(Mat labels)
{
	cout<<"Reprojecting voxels"<<endl;
	// if points with the same coordinates, only take the one that is closest
	vector<Voxel*> voxels = getVisibleVoxels();
	//int bx,by,kx,ky;
	//Point3f camlocation;
	//float distBase, distK;
	
	vector<vector<vector<Point2f>>> imgPoints(voxels[0] ->camera_projection.size());
	vector<vector<Point2f>> points(4); //  4 lists, for every label one.
	for (int c=0; c<voxels[0]->camera_projection.size();c++)
	{
		cout<<"Checking cam "<<c<<endl;
		// set all entries
		for (int base=0; base<voxels.size();base++)
		{
			cout<<" Put at: points["<<labels.at<int>(base)<<"].push_back( "<<voxels[base] -> camera_projection[c]<<" )"<<endl;
			points[labels.at<int>(base)].push_back( voxels[base] -> camera_projection[c] );
		}
		imgPoints[c]=points;
		cout<<"Put at imgPoints["<<c<<"]"<<endl;
		cout<<endl;
	}

	// bad way is done
	return imgPoints;

	/*
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
					if (voxels[k]->valid_camera_projection[c] ==1 && voxels[base]->valid_camera_projection[c]==1)
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
	*/
}


Mat Reconstructor::reprojectVoxels2(Mat &frame, int camera,int occlusionCheck=1)
{
	cout<<"Reprojecting voxels for camera "<<camera<<endl;
	
	vector<Voxel*> voxels = getVisibleVoxels();
	Point3f camLoc = getCameras()[camera]->getCameraLocation();
	cout<<"Visible voxels: "<<voxels.size();
	float distBase, distComp;
	Voxel* voxBase;
	Voxel* voxComp;

	float neigh = 0;
	if(occlusionCheck != 0)
	{
		neigh = 25;
	}

	int iter;
	vector<int> neighbour(neigh,0);
	vector<int> dummy(frame.size().height,0);
	vector<vector<int>> imgRepr(frame.size().width,dummy);
	cout<<"imgRepr.size() x imgRepr[0].size()  "<<imgRepr.size()<<" x "<<imgRepr[0].size()<<endl; 
	cout<<"frame.size() "<< frame.size()<<endl;
	for (int base=0; base<voxels.size();base++)
	{		
		
		//check if image projection is already present
		Point projection = voxels[base] ->camera_projection[camera];
		
		// check whether presented pixel is already present
		if (imgRepr[projection.x][projection.y]!=0)
		{
			//cout<<"Replace? ";
			//if so, put closest one at that position
			voxBase = voxels[base];
			distBase = sqrt((camLoc.x - voxBase->x)*(camLoc.x - voxBase->x)  + (camLoc.y - voxBase->y)*(camLoc.y - voxBase->y)
						+ (camLoc.z - voxBase->z)*(camLoc.z - voxBase->z));

			voxComp = voxels[imgRepr[projection.x][projection.y]-1];
			distComp = sqrt((camLoc.x - voxComp->x)*(camLoc.x - voxComp->x)  + (camLoc.y - voxComp->y)*(camLoc.y - voxComp->y) 
					+ (camLoc.z - voxComp->z)*(camLoc.z - voxComp->z));
			if (distBase<distComp)
			{
				imgRepr[projection.x][projection.y]=base+1;
				//cout<<"Yes!"<<endl;
				//} else { cout<<"No"<<endl;
			}
		} else if (occlusionCheck ==0 )
		{
			imgRepr[projection.x,projection.y] = base+1;
		} else
		{
			//cout<<"Check neighbourhood";
			//get neighbouring pixels
			neighbour.assign(neigh,0);
			for (int i=0;i<(int)sqrt(neigh);i++)
			{
				for (int j=0;j<(int)sqrt(neigh);j++)
				{
					if( (projection.x + (i-((int)sqrt(neigh)-2)) > 0) && (projection.x + (i-((int)sqrt(neigh)-2)) < imgRepr.size() ) 
						&& (projection.y + (j-((int)sqrt(neigh)-2)) > 0) && (projection.y + (j-((int)sqrt(neigh)-2)) << imgRepr[0].size()) )
					{
						neighbour[(sqrt(neigh)*i)+j] = imgRepr[projection.x + (i-(sqrt(neigh)-2))][projection.y + (j-(sqrt(neigh)-2))];
					}
				}
			}

			voxBase = voxels[base];
			distBase = sqrt((camLoc.x - voxBase->x)*(camLoc.x - voxBase->x)  + (camLoc.y - voxBase->y)*(camLoc.y - voxBase->y)
						+ (camLoc.z - voxBase->z)*(camLoc.z - voxBase->z));
			distComp = distBase + 10;
			// check all entries subsequently
			iter =-1;
			while(distBase < distComp && iter<neigh)
			{
				iter++;
			    //cout<< iter<<" ";
				// skip self and 0 entries
				if ( (iter != (neigh-1)/2) && (neighbour[iter] > 0) )
				{ 
					//check of current entry
					voxComp = voxels[neighbour[iter]-1];

					distComp = sqrt((camLoc.x - voxComp->x)*(camLoc.x - voxComp->x)  + (camLoc.y - voxComp->y)*(camLoc.y - voxComp->y) 
						+ (camLoc.z - voxComp->z)*(camLoc.z - voxComp->z));
				}
			}

			if (iter==neigh)
			{
				// This means that voxel base is closest, and should be projected. The others should not!
				//cout<<" Closest! ";
				for (int i=0;i<(int)sqrt(neigh);i++)
				{
					for (int j=0;j<(int)sqrt(neigh);j++)
					{
						if( (projection.x + (i-((int)sqrt(neigh)-2)) > 0) && (projection.x + (i-((int)sqrt(neigh)-2)) < imgRepr.size()) 
							&& (projection.y + (j-((int)sqrt(neigh)-2)) > 0) && (projection.y + (j-((int)sqrt(neigh)-2)) << imgRepr[0].size()) )
						{
							imgRepr[projection.x + (i-((int)sqrt(neigh)-2))][projection.y + (j-((int)sqrt(neigh)-2))] = 0;
						}
					}
				}
				// Enter new pixel;
				imgRepr[projection.x][projection.y] = base + 1;
			}
			//cout<<endl;
		}
	}
	
	cout<< ". . done!"<<endl;
	vector<Voxel*> repVoxels;
	Mat mask(frame.size(),CV_8U);
	mask = uchar(0);
	//cout<<"mask "<<mask.size()<<endl;
	cout<<"imgRepr" << imgRepr.size() << " x " << imgRepr[0].size()<<endl; 
	for(int row = 0; row < imgRepr.size();row++) 
	{
		for(int col = 0; col < imgRepr[row].size(); col++) 
		{
			 if (imgRepr[row][col] != 0 )
			 {
				 mask.at<uchar>(col,row) = (uchar)1;
				 repVoxels.push_back(voxels[imgRepr[row][col]-1]);
			 }
		}
    }
	cout<<"Repr voxels: "<<repVoxels.size()<<endl;

	setProjectableVoxels(repVoxels,camera);
	//return imgRepr;
	return mask;
}

} /* namespace nl_uu_science_gmt */
