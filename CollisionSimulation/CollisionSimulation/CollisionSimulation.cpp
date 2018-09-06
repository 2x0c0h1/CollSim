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
	CollisionDynamics collDy(0.5, 2, 2, 1, 5);

	vhs vhs;
	//Lengths are in meters
	vhs.setTarget(Particle(0.10, new Vector(1, 2.5), new Vector(0, 0)));
	vhs.addParticle(new Particle(0.10, new Vector(0, 0), new Vector(0.019, 0.05)));
	vhs.run();

	std::cout << "\n\n\n//////////////PROGRAM END///////////////" << std::endl;
	system("pause");
    return 0;
}


