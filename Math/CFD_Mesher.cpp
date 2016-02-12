#include "stdafx.h"
#include "CFD_Mesher.h"
#include <list>
#include <algorithm> 
#include <fstream>
namespace CFD
{
	namespace
	{
		//如果两个值相差TOL(1e-8),那么可以认为这两个点相同
		bool IsEqual(double val1, double val2)
		{
			return std::abs(val1 - val2) < TOL;
		}
	}
	Mesher::Mesher(const RectangleRegion & problemRegion, const std::vector<RectangleRegion>& partRegions) :
		_problemRegion(problemRegion),
		_partRegions(partRegions),
		_meshXPos(),
		_meshYPos(),
		_problemRegionMeshSize(0.05),
		_partRegionMeshSize(0.01),
		_minDx(0.0),
		_minDy(0.0)
	{
	}

	Mesher::~Mesher()
	{
	}

	void Mesher::StartMesh()
	{
		//确保问题域包含了所有的零件域
		for (int ii = 0; ii < _partRegions.size(); ++ii)
		{
			assert(_problemRegion.Contain(_partRegions[ii]));
		}
		std::vector<Interval> xResultIntervals,yResultIntervals;
		Interval xProblemInterval(_problemRegion.P1().X, _problemRegion.P2().X);
		xProblemInterval.IsPartRegion = false;
		xResultIntervals.push_back(xProblemInterval);
		Interval yProblemInterval(_problemRegion.P1().Y, _problemRegion.P4().Y);
		yProblemInterval.IsPartRegion = false;
		yResultIntervals.push_back(yProblemInterval);

		MeshOneInterval(&xResultIntervals, true);
		MeshOneInterval(&yResultIntervals, false);

		auto getMinInterval = [](const std::vector<float>& pos)
		{
			assert(pos.size() >1);
			float minDist = 1e10;
			for (int ii = 1; ii < pos.size(); ++ii)
			{
				float tmpVal = pos[ii] > pos[ii - 1];
				assert(tmpVal>0);
				if (tmpVal < minDist)
					tmpVal = minDist;
			}
			return minDist;
		};
		_minDx = getMinInterval(_meshXPos);
		_minDy = getMinInterval(_meshYPos);
	}

	const std::vector<float>& Mesher::XMeshPos() const
	{
		return _meshXPos;
	}

	const std::vector<float>& Mesher::YMeshPos() const
	{
		return _meshYPos;
	}

	void Mesher::SetProblemRegionMeshSize(float meshSize)
	{
		_problemRegionMeshSize = meshSize;
	}

	void Mesher::SetPartRegionMeshSize(float meshSize)
	{
		assert(meshSize > 0);
		_partRegionMeshSize = meshSize;
	}

	void Mesher::SaveToFile(const std::string & fileName) 
	{
		//导出文件后可以用matlab画出网格
		std::ofstream fileStream;
		fileStream.open(fileName);
		fileStream << "#num of problem regions and part regions" << std::endl;
		fileStream <<1<<" "<<_partRegions.size() << std::endl;

		fileStream << "#problem region and part regions(x,y,w,h):" << std::endl;
		fileStream << _problemRegion.X() << " " << _problemRegion.Y() << " " << _problemRegion.W() << " " << _problemRegion.H() << std::endl;
		for (auto& item : _partRegions)
		{
			fileStream << item.X()<< " " << item.Y() << " " << item.W() << " " << item.H() << std::endl;
		}
		fileStream << "#x coordinate array and y coordinate array" << std::endl;
		for (auto val : _meshXPos)
			fileStream << val << " ";
		fileStream << std::endl;
		for (auto val : _meshYPos)
			fileStream << val << " ";
		fileStream << std::endl;
		fileStream.close();
	}

	float Mesher::GetMinDx() const
	{
		return _minDx;
	}

	float Mesher::GetMinDy() const
	{
		return _minDy;
	}

	std::vector<float> Mesher::MeshInterval(const Interval & interval) const
	{
		assert(interval.Length() > TOL);
		std::vector<float> resultPos;
		float meshSize = _problemRegionMeshSize;
		if (interval.IsPartRegion)
		{
			//如果是零件的话划分的应该更密一些
			meshSize = _partRegionMeshSize;
		}
		const int nSegments = (int)(interval.Length() / meshSize);
		const float stepSize = interval.Length() / nSegments;
		for (int ii = 0; ii < nSegments;++ii)
			resultPos.push_back(interval.X0() + stepSize*ii);
		//interval.X1不需要加进去
		return resultPos;
	}

	void Mesher::MeshOneInterval(std::vector<Interval>* oneDirectionIntervals, bool needSplitXInterval)
	{
		for (int ii = 0; ii < _partRegions.size(); ++ii)
		{
			Interval partInterval;
			if (needSplitXInterval)
			{
				partInterval.SetX0(_partRegions[ii].P1().X);
				partInterval.SetX1(_partRegions[ii].P2().X);
			}
			else
			{
				partInterval.SetX0(_partRegions[ii].P1().Y);
				partInterval.SetX1(_partRegions[ii].P4().Y);
			}
			partInterval.IsPartRegion = true;

			//std::vector<Interval> allSplitedIntervals;
			for (auto it = (*oneDirectionIntervals).begin(); it != (*oneDirectionIntervals).end();)
			{
				std::vector<Interval> splitedIntervals = it->SplitByOtherInterval(partInterval);
				if (splitedIntervals.empty())
				{
					++it;
					continue;
				}
				else if (!splitedIntervals.empty() && splitedIntervals[0].IsEqual(*it))
				{
					//如果已经存在
					++it;
					continue;
				}
				else
				{
					it = (*oneDirectionIntervals).erase(it);
					it=(*oneDirectionIntervals).insert(it, splitedIntervals.begin(), splitedIntervals.end());
				}
			}
		}
		std::sort((*oneDirectionIntervals).begin(), (*oneDirectionIntervals).end(), [](const Interval& interval1, const Interval& interval2)
		{
			return interval1.X0() < interval2.X0();
		});
		std::vector<float>& targetMeshPos = needSplitXInterval ? _meshXPos : _meshYPos;
		for (auto it = (*oneDirectionIntervals).begin(); it != (*oneDirectionIntervals).end(); ++it)
		{
			std::vector<float> points = MeshInterval(*it);
			targetMeshPos.insert(targetMeshPos.end(), points.begin(), points.end());
			if (it == (*oneDirectionIntervals).end() - 1)
				targetMeshPos.push_back(it->X1());
		}
	}


	RectangleRegion::RectangleRegion(float x, float y, float w, float h,float temperature,float alpha):
		_x(x),
		_y(y),
		_w(w),
		_h(h),
		_p1( x,y ),
		_p2( x + w,y ),
		_p3( x + w,y + h ),
		_p4(x,y+h),
		_temperature(temperature),
		_alpha(alpha)
	{
	}

	bool RectangleRegion::Contain(const Point2& pos)const
	{
		if(pos.X<_x-TOL||pos.X>(_x+_w+ TOL)||pos.Y<_y- TOL ||pos.Y>(_y+_h+ TOL))
			return false;
		return true;
	}

	bool RectangleRegion::Contain(const RectangleRegion & region)const
	{
		if (Contain(region.P1()) && Contain(region.P2()) && Contain(region.P3()) && Contain(region.P4()))
			return true;
		return false;
	}

	bool Interval::Contain(float val) const
	{
		if (val<_x0-TOL || val>_x1+TOL)
			return false;
		return true;
	}

	float Interval::Length() const
	{
		return std::abs(_x0 - _x1);
	}

	std::vector<Interval> Interval::SplitByOtherInterval(const Interval & otherInterval) const
	{
		std::vector<Interval> resultIntervals;
		if (this->IsEqual(otherInterval))
			return resultIntervals;
		resultIntervals.reserve(3);
		//两区域完全不相交
		if (_x1<(otherInterval.X0() - TOL )|| _x0>(otherInterval.X1() + TOL))
			return resultIntervals;
		//otherinterval只有右边界落在区域
		if (Contain(otherInterval.X1()) && !Contain(otherInterval.X0()))
		{
			Interval tmpInterval1(_x0, otherInterval.X1());
			tmpInterval1.IsPartRegion = otherInterval.IsPartRegion;
			if (tmpInterval1.Length() < TOL)
				return resultIntervals;
			resultIntervals.push_back(tmpInterval1);
			Interval tmpInterval2(otherInterval.X1(), _x1);
			tmpInterval2.IsPartRegion = this->IsPartRegion;
			if (tmpInterval2.Length() < TOL)
				return resultIntervals;
			resultIntervals.push_back(tmpInterval2);
		}
		else if (Contain(otherInterval.X0()) && !Contain(otherInterval.X1()))
		{//只有左边界落在区域
			Interval tmpInterval1(_x0, otherInterval.X0());
			tmpInterval1.IsPartRegion = this->IsPartRegion;
			if (tmpInterval1.Length() < TOL )
				return resultIntervals;
			resultIntervals.push_back(tmpInterval1);
			Interval tmpInterval2(otherInterval.X0(), _x1);
			tmpInterval2.IsPartRegion = otherInterval.IsPartRegion;
			if(tmpInterval2.Length()>TOL)
				resultIntervals.push_back(tmpInterval2);
		}
		else if (Contain(otherInterval.X0()) && Contain(otherInterval.X1()))
		{//完全包含otherInterval
			Interval tmpInterval1(_x0, otherInterval.X0());
			tmpInterval1.IsPartRegion = this->IsPartRegion;
			if (tmpInterval1.Length() > TOL)
				resultIntervals.push_back(tmpInterval1);
			Interval tmpInterval2(otherInterval.X1(), _x1);
			tmpInterval2.IsPartRegion = this->IsPartRegion;
			if (tmpInterval2.Length() > TOL)
				resultIntervals.push_back(tmpInterval2);
			resultIntervals.push_back(otherInterval);
		}
		else if (otherInterval.Contain(_x0) && otherInterval.Contain(_x1))
		{//otherInterval 包含当前interval
			Interval tmpInterval = *this;
			tmpInterval.IsPartRegion = otherInterval.IsPartRegion;
			if (tmpInterval.Length() > TOL)
				resultIntervals.push_back(tmpInterval);
		}

		return resultIntervals;
	}

}
