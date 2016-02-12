========================================================================
This is a project for solving 2D transient heat equation by finite difference method.
Please read Demo.cpp for the whole procedure.
CFD::RectangleRegion
	RectangleRegion defines a rectangle region. you can use it to represent a rectangle region. e.g. 
	if you want define rectangle with width=0.2 and height=0.2 at the point(0.1,0.2), then, you can use the code like this:
	CFD::RectangleRegion ariRegion(0.1,0.1,0.2,0.2,20.0,1.9e-5). Note that, number 20.0 represents the temperature of air,and
	number 1.9e-5 means alpha value for air whick is a const value, for more information about it, please refer to 
	https://en.wikipedia.org/wiki/Thermal_diffusivity.
CFD::Mesher
	Mesher can generate rectangle grid for finite differenct method. Taking a chip in the air for example, in order to solve the 
	heat equation for this model quickly, you should make denser grid at the chip region, and thinner at the air region. then you can 
	using mesher.SetPartRegionMeshSize and mesher.SetProblemRegionMeshSize to define different mesh size.
CFD::SolverConfig
	it's the configuration of solver, you can define suitable time step size for iteration and the total time .
CFD::Solver
	After you have defined the regions and generated the mesh grid, you can use the solver to calculate for result.
	Note that you should set boundary condition for the whole problem region by using solver.SetMaterialAlphaValue and solver.SetTemperature

plotMesh.m
	this is a matlab script file. you can display the grid and result.
/////////////////////////////////////////////////////////////////////////////
