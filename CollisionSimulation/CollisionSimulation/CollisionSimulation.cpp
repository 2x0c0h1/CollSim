// CollisionSimulation.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include "vhs.h"
#include "Point.h"
#include "Ipl.h"
#include <stdio.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_roots.h>
#include <gsl/gsl_errno.h>
#include "CollisionDynamics.h"
#include "Gas.h"
#include <math.h>

int main()
{
	std::cout << "\n\n\n//////////////PROGRAM START///////////////" << std::endl;
	/*
	Starting of collision dynamics (finding deflection angle) testing 
	Change function parameters here: 
	miss-distance impact parameter : b, 
	mass : m,
	molecular velocity vector : c,
	constant kappa : k,
	exponent eta : n
	*/
	Gas g = Gas();
	CollisionDynamics collDy(0.5, 2.0, 2.0, 1.0, 5.0, g);

	std::cout << "\n\n\n//////////////GAS TEST START///////////////" << std::endl;

	Gas Hydrogen_Gas = Gas(5.0, 3.34 * pow(10, -27), 0.845 * pow(10, -5), 0.67);
	CollisionDynamics collDy2(0.5, 2.0, Hydrogen_Gas);

	vhs vhs;
	//Lengths are in meters
	vhs.setTarget(Particle(0.10, new Vector(1, 2.5), new Vector(0, 0)));
	vhs.addParticle(new Particle(0.10, new Vector(0, 0), new Vector(0.019, 0.05)));
	vhs.run();

	std::cout << "\n\n\n//////////////PROGRAM END///////////////" << std::endl;
	system("pause");
    return 0;
}


