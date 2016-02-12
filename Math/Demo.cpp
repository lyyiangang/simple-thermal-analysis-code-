// Math.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "CFD_Mesher.h"
#include "CFD_Solver.h"
void RemoveOldDataFiles(const std::string& fileName)
{
	
}
void Test()
{
	//remove the old files
	std::string meshFile("d:\\cfd.mesh"),resultFile("d:\\cfd_result_file.dat");
	remove(meshFile.c_str());
	remove(resultFile.c_str());
	//define the problem region(air)
	float intialTemperature = 10.0;
	std::vector<CFD::RectangleRegion> allPartRegions;
	CFD::RectangleRegion problemRegion(0.0, 0.0, 1.0, 1.0, intialTemperature,1.9e-5);//air

	//define the very part,e.g. a copper block with height equals 0.2 and width equals 0.2
	CFD::RectangleRegion partRegion1(0.2, 0.2, 0.2, 0.2,40.0, 1.11e-4);//copper( 0.000111)
	allPartRegions.push_back(partRegion1);
	CFD::RectangleRegion partRegion2(0.6, 0.6, 0.1, 0.1,30.0, 1.11e-4);
	allPartRegions.push_back(partRegion2);

	//generate mesh
	CFD::Mesher mesher(problemRegion,allPartRegions);
	mesher.SetPartRegionMeshSize(0.01);//1/20
	mesher.SetProblemRegionMeshSize(0.015);
	mesher.StartMesh();
	//save the mesh data to a file, which will be imported to matlab script for displaying.
	mesher.SaveToFile(meshFile);

	//set solver
	//copper:1.11e-4
	//water:1.43e-07
	//air:1.9e-5
	CFD::SolverConfig config;
	config.Dt = 0.1;//时间步长，数值越小，需要迭代的次数越多，计算越慢
	config.TotalTime = 100;//数值越大，迭代次数越多，计算越慢
	config.Epsilon = 1e-4;//求解精度，如果本次迭代与上次迭代的数值之差小于该值，则计算可以结束
	CFD::Solver solver(mesher.XMeshPos(), mesher.YMeshPos());
	solver.SetConfiguration(config);

	//set part region termerature
	int nr = mesher.YMeshPos().size(), nc = mesher.XMeshPos().size();
	for (int ii = 0; ii < nr; ++ii)
	{
		for (int jj = 0; jj < nc; ++jj)
		{
			solver.SetMaterialAlphaValue(jj, ii, problemRegion.Alpha());//default material is air
			solver.SetTemperature(jj, ii, intialTemperature,false);
			CFD::Point2 pt(mesher.XMeshPos()[jj], mesher.YMeshPos()[ii]);
			for (const auto& item : allPartRegions)
			{
				if (item.Contain(pt))
				{
					solver.SetTemperature(jj, ii, item.Termperature(),false);
					solver.SetMaterialAlphaValue(jj, ii, item.Alpha());
				}
			}
		}
	}

	//set boundary temperature condition
	//intial temperature:10
	//top,[:,1]=0
	//bottom,[:,0]=50
	//left,[0,:]=25
	//right,[1,:]=25
	for (int ii = 0; ii < nr; ++ii)
	{
		solver.SetTemperature(ii, 0, 25.0, true);//left
		solver.SetTemperature(ii, nc - 1, 25.0, true);//right
	}
	for (int ii = 0; ii < nc; ++ii)
	{
		solver.SetTemperature(0, ii, 50.0, true);//bottom
		solver.SetTemperature(nr - 1, ii, 0, true);//top
	}
	int error= solver.StartSolve();
	if (error > 0)
	{
		std::cout << "error occurs when solving" << std::endl;
		return;
	}
	solver.SaveResultToFile(resultFile);
}

int main()
{
	Test();
	getchar();
    return 0;
}

