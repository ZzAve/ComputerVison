/*
 * Reconstructor.h
 *
 *  Created on: Nov 15, 2013
 *      Author: coert
 */

#ifndef RECONSTRUCTOR_H_
#define RECONSTRUCTOR_H_

#include <vector>
#include "opencv2/opencv.hpp"
#include "Camera.h"

namespace nl_uu_science_gmt
{

class Reconstructor
{
public:
	struct Voxel
	{
		int x, y, z;
		int label;
		std::vector<cv::Point> camera_projection;
		std::vector<int> valid_camera_projection;
	};

private:
	const std::vector<Camera*> &_cameras;

	int _step;
	int _size;

	std::vector<cv::Point3f*> _corners;

	size_t _voxels_amount;
	cv::Size _plane_size;

	std::vector<Voxel*> _voxels;
	std::vector<Voxel*> _visible_voxels;
	std::vector<std::vector<Voxel*>> _projectable_voxels;

	std::vector<cv::Point2f> _centers;

	void initialize();

public:
	Reconstructor(const std::vector<Camera*> &);
	virtual ~Reconstructor();

	void update();
	void calculatekMeans();
	std::vector<std::vector<std::vector<cv::Point2f>>> reprojectVoxels(cv::Mat);
	cv::Mat reprojectVoxels2(cv::Mat &, int,int);
	cv::Mat reprojectVoxels2(cv::Mat &, int);

	const std::vector<Voxel*>& getVisibleVoxels() const
	{
		return _visible_voxels;
	}

	const std::vector<Voxel*>& getProjectableVoxels(int camera) const
	{
		return _projectable_voxels[camera];
	}
	const std::vector<Voxel*>& getProjectableVoxels() const
	{
		return _projectable_voxels[0];
	}

	const std::vector<Voxel*>& getVoxels() const
	{
		return _voxels;
	}

	const int getLabel(int VoxelId) const
	{
		return _visible_voxels[VoxelId] ->label;
	}
	void setVisibleVoxels(const std::vector<Voxel*>& visibleVoxels)
	{
		_visible_voxels = visibleVoxels;
	}

	void setProjectableVoxels(const std::vector<Voxel*>& projectableVoxels,int camera)
	{
		_projectable_voxels[camera] = projectableVoxels;
	}
	
	void setLabel(int voxelId, int colorId)
	{ 
		_visible_voxels[voxelId] -> label = colorId;
	}

	void setVoxels(const std::vector<Voxel*>& voxels)
	{
		_voxels = voxels;
	}
	
	void setCenters(std::vector<cv::Point2f> centers)
	{
		_centers = centers;
	}
	std::vector<cv::Point2f> getCenters()
	{
		return _centers;
	}
	const std::vector<cv::Point3f*>& getCorners() const
	{
		return _corners;
	}

	int getSize() const
	{
		return _size;
	}

	const cv::Size& getPlaneSize() const
	{
		return _plane_size;
	}

	const std::vector<Camera*>& getCameras()
	{
		return _cameras;
	}


	const void initializeProjectableVoxels()
	{
		_projectable_voxels.resize(_cameras.size());
	}

	const void initCenters()
	{
		_centers.resize(4);
	}
};

} /* namespace nl_uu_science_gmt */

#endif /* RECONSTRUCTOR_H_ */
