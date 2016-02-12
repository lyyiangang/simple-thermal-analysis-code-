#include "stdafx.h"
#include "CFD_Solver.h"
#include <algorithm>
#include <fstream>
namespace CFD
{
	Solver::Solver(const std::vector<float>& gridXPos, const std::vector<float>& gridYPos):
		_nx(gridXPos.size()),
		_ny(gridYPos.size())
	{
		_tMatrix.resize(gridYPos.size(),std::vector<MatrixItem>(gridXPos.size()));
		for (int ii = 0; ii < gridYPos.size(); ++ii)
		{
			for (int jj = 0; jj < gridXPos.size(); ++jj)
			{
				_tMatrix[ii][jj].BoundaryCondition = 0;
				_tMatrix[ii][jj].T = 0.0;
				_tMatrix[ii][jj].X = gridXPos[jj];
				_tMatrix[ii][jj].Y = gridYPos[ii];
				_tMatrix[ii][jj].Alpha = 0.0;
			}
		}
	}

	void Solver::SetConfiguration(const SolverConfig & config)
	{
		_config = config;
	}

	void Solver::SetTemperature(int ir, int ic, float temperature,bool isBoundary)
	{
		assert(ir > -1 && ir<_ny&&ic>-1 && ic < _nx);
		_tMatrix[ir][ic].T= temperature;
		_tMatrix[ir][ic].BoundaryCondition = isBoundary?1:0;
	}

	void Solver::SetMaterialAlphaValue(int ir, int ic, float alpha)
	{
		assert(ir > -1 && ir<_ny&&ic>-1 && ic < _nx);
		_tMatrix[ir][ic].Alpha = alpha;
	}

	int Solver::StartSolve()
	{
		assert(_nx > 2 && _ny > 2);
		//float minDy = 1e10, minDx = 1e10;
		//for (int jj = 1; jj < _ny; ++jj)
		//{
		//	float tmpVal = _tMatrix[jj][0].Y - _tMatrix[jj - 1][0].Y;
		//	if (tmpVal < minDy)
		//		minDy = tmpVal;
		//}
		//for (int ii = 1; ii < _nx; ++ii)
		//{
		//	float tmpVal = _tMatrix[0][ii].X - _tMatrix[0][ii - 1].X;
		//	if (tmpVal < minDx)
		//		minDx = tmpVal;
		//}

		//set intial temperature
		//for (int jj = 0; jj< _ny;++jj)
		//{
		//	for (int ii = 0; ii < _nx;++ii)
		//	{
		//		if (_tMatrix[jj][ii].BoundaryCondition < 1)
		//			_tMatrix[jj][ii].T = _config.InitialTemperature;
		//	}
		//}
		
		auto getMaxError = [](const TemperatureMatrix& mat1, const TemperatureMatrix& mat2)->float
		{
			assert(mat1.size()>0 && mat1.front().size() > 0);
			int nr = mat1.size(), nc = mat1.front().size();
			assert(nr == mat2.size() && nc == mat2.front().size());
			float maxError = 0.0;
			for (int ii = 0; ii < nr;++ii)
			{
				for (int jj = 0; jj < nc; ++jj)
				{
					float tmpError= std::abs(mat1[ii][jj].T - mat2[ii][jj].T);
					if (tmpError > maxError)
						maxError = tmpError;
				}
			}
			return maxError;
		};
		int interationTimes = _config.TotalTime / _config.Dt, n = 0;
		float maxError = 0.0,maxFo=-1.0,minFo=1e10;
		TemperatureMatrix preMatrix= _tMatrix;
		for (; n< interationTimes; ++n)
		{
			preMatrix = _tMatrix;
			for (int jj = 0; jj < _ny; ++jj)
			{
				for (int ii = 0; ii < _nx; ++ii)
				{
					if (_tMatrix[jj][ii].BoundaryCondition>0)
						continue;
					float deltaX1 = preMatrix[jj][ii].X - preMatrix[jj][ii - 1].X;
					float deltaX2 = preMatrix[jj][ii + 1].X - preMatrix[jj][ii].X;
					float deltaY1 = preMatrix[jj][ii].Y - preMatrix[jj - 1][ii].Y;
					float deltaY2 = preMatrix[jj + 1][ii].Y - preMatrix[jj][ii].Y;
					float rx = 2* preMatrix[jj][ii].Alpha*_config.Dt / (deltaX1+deltaX2);
					float ry = 2* preMatrix[jj][ii].Alpha*_config.Dt / (deltaY1+deltaY2);

					//	std::cout << "Note:Numerical stability requires 0.1<Fo=alpha*dt*(1/dx^2+1/dy^2)<=0.5,Fo=" << Fo << std::endl;
					float minDx = std::min(deltaX1, deltaX2);
					float minDy = std::min(deltaY1, deltaY2);
					float tmpFo = preMatrix[jj][ii].Alpha*_config.Dt*(1 / (minDx*minDx) + 1 / (minDy*minDy));
					if (tmpFo > maxFo)
						maxFo = tmpFo;
					if (tmpFo < minFo)
						minFo = tmpFo;
					_tMatrix[jj][ii].T = preMatrix[jj][ii].T + rx*((preMatrix[jj][ii + 1].T - preMatrix[jj][ii].T) / deltaX2 + (preMatrix[jj][ii - 1].T - preMatrix[jj][ii].T) / deltaX1)
															 + ry*((preMatrix[jj + 1][ii].T - preMatrix[jj][ii].T) / deltaY2 + (preMatrix[jj - 1][ii].T - preMatrix[jj][ii].T) / deltaY1);

					//_tMatrix[jj][ii].T = preMatrix[jj][ii].T + rx*(preMatrix[jj][ii + 1].T - 2 * preMatrix[jj][ii].T + preMatrix[jj][ii - 1].T)
					//	+ ry*(preMatrix[jj + 1][ii].T - 2 * preMatrix[jj][ii].T + preMatrix[jj - 1][ii].T);
				}
			}
			maxError = getMaxError(preMatrix, _tMatrix);
			std::cout << "iteration " << n << " error:" << maxError << std::endl;
			if (maxError < _config.Epsilon)
				break;
		}
		std::cout << "iteration times:" << n << std::endl;
		std::cout << "maxFo=" << maxFo<<",minFo="<<minFo<< std::endl;
		return 0;
	}

	void Solver::SaveResultToFile(const std::string & fileName) const
	{
		std::ofstream stream;
		stream.open(fileName);
		for (const auto& row : _tMatrix)
		{
			for (const auto& item : row)
				stream << item.T << " ";
			stream << std::endl;
		}
		stream.close();
	}

	Solver::~Solver()
	{
	}
}