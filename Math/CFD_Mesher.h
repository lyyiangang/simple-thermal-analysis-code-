#pragma once
namespace CFD
{
	typedef struct Point2
	{
	public:
		Point2(float x,float y):
			X(x),
			Y(y)
		{}
		float X=0.0, Y=0.0;
	}Point2;

	typedef struct Interval
	{
	public:
		Interval() :
			_x0(0.0),
			_x1(0.0)
		{
		}
		Interval(float x0, float x1) :
			_x0(x0),
			_x1(x1)
			{
		}
		float X0()const
		{
			return _x0;
		}
		float X1()const
		{
			return _x1;
		}
		void SetX0(float val)
		{
			_x0 = val;
		}
		void SetX1(float val)
		{
			_x1 = val;
		}

		bool IsEqual(const Interval& otherInterval)const
		{
			return std::abs(_x0 - otherInterval._x0) < TOL&&
				std::abs(_x1 - otherInterval._x1) < TOL&&
				IsPartRegion == otherInterval.IsPartRegion;
		}
		bool Contain(float val)const;
		float Length()const;
		std::vector<Interval> SplitByOtherInterval(const Interval& otherInterval)const;
		
		bool IsPartRegion = false;
	private:
		float _x0, _x1;
	}Interval;
	//   p4                             p3           
	//   |¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
	//   |                             |
	// h |                             |
	//   |                             | 
	//   |                             |
	//   |_____________________________|
	//  p1 (x,y)          w            p2
	//    
	//   
	class RectangleRegion
	{
	public:
		RectangleRegion(float x, float y, float w, float h,float temperature,float alpha);
		float& X() { 
			return _x; };
		float& Y() {
			return _y;
		};
		float& W() {
			return _w;
		};
		float& H() {
			return _h;
		};
		const Point2& P1()const
		{
			return _p1;
		};
		const Point2& P2()const
		{
			return _p2;
		}
		const Point2& P3()const
		{
			return _p3;
		}
		const Point2& P4() const
		{
			return _p4;
		}
		float Termperature()const
		{
			return _temperature;
		}
		float Alpha()const
		{
			return _alpha;
		}
		bool Contain(const Point2& pos)const;
		bool Contain(const RectangleRegion& region)const ;

	private:
		float _x, _y, _w, _h,_temperature, _alpha;
		Point2 _p1, _p2, _p3, _p4;

	};

	class Mesher
	{
	public:
		//problem Region： 整个问题所占区域
		//partRegions: 元器件所占区域
		Mesher(const RectangleRegion& problemRegion,const std::vector<RectangleRegion>& partRegions);
		~Mesher();

		void StartMesh();
		const std::vector<float>& XMeshPos()const;
		const std::vector<float>& YMeshPos()const;
		void SetProblemRegionMeshSize(float meshSize);
		void SetPartRegionMeshSize(float meshSize);
		void SaveToFile(const std::string& fileName);
		float GetMinDx()const;
		float GetMinDy()const;
	private:
		std::vector<float> MeshInterval(const Interval& interval)const ;
		void MeshOneInterval(std::vector<Interval>* oneDirectionIntervals, bool needSplitXInterval);

		RectangleRegion _problemRegion;
		std::vector<RectangleRegion> _partRegions;
		std::vector<float> _meshXPos, _meshYPos;
		float _problemRegionMeshSize, _partRegionMeshSize,_minDx,_minDy;
	};
}
