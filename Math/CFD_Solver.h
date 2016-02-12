#pragma once
namespace CFD
{
	typedef struct MatrixItem
	{
		float T, X, Y,Alpha;
		int BoundaryCondition;
	}MatrixItem;
	
	typedef struct SolverConfig
	{
		float  TotalTime, Dt, Epsilon;
	}SolverConfig;

	class Solver
	{
	public:
		Solver(const std::vector<float>& gridXPos,const std::vector<float>& gridYPos);
		void SetConfiguration(const SolverConfig& config);
		//index start from 0
		void SetTemperature(int ix, int iy, float temperature,bool isBoundary);
		void SetMaterialAlphaValue(int ix, int iy, float alpha);
		int StartSolve();
		void SaveResultToFile(const std::string& fileName)const;
		~Solver();
	private:
		using TemperatureMatrix = std::vector<std::vector<MatrixItem>>;
		TemperatureMatrix _tMatrix;
		int _nx, _ny;//node number of different directions
		SolverConfig _config;
	};
}
