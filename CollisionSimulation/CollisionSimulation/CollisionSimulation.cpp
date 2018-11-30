// CollisionSimulation.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>
#include <chrono>
#include <math.h>
#include <stdio.h>
#include <fstream>

#include "VHS.h"
#include "Ipl.h"
#include "CollisionDynamics.h"
#include "Gas.h"
#include "RandomParameters.h"

#include <gsl/gsl_integration.h>
#include <gsl/gsl_roots.h>
#include <gsl/gsl_errno.h>

#include <Eigen/Dense>

int main()
{
	std::cout << "PROGRAM START--->" << std::endl;
	std::ofstream dataFile;
	dataFile.open("data.csv");
	dataFile << "Initial Pos 1,Initial Velocity 1,Initial Pos 2,Initial Velocity 2,VHS Final Velocity 1, VHS Final Velocity 2" << std::endl;
	
	std::cout << "INPUT VARIBLES FOR RANDOM PARAMETERS--->" << std::endl;
	
	RandomParameters rp;

	std::string input;
	std::cout << "Input coefficient of viscosity (0.845 * 10^-5): ";
	std::getline(std::cin, input);
	
	if (!input.empty()) {
		rp.setCoefficientOfViscosity(atof(input.c_str()) * pow(10, -5));
		input.clear();
	}

	std::cout << "Input mass of atom (3.34 * 10^-27): ";
	std::getline(std::cin, input);

	if (!input.empty()) {
		rp.setMass(atof(input.c_str()) * pow(10, -27));
		input.clear();
	}

	std::cout << "Input viscosity index (0.67): ";
	std::getline(std::cin, input);

	if (!input.empty()) {
		rp.setViscosityIndex(atof(input.c_str()));
		input.clear();
	}
	
	std::cout << "Input diameter (2.92 * 10^-10): ";
	std::getline(std::cin, input);

	if (!input.empty()) {
		rp.setDiameter(atof(input.c_str()) * pow(10,-10));
		input.clear();
	}

	std::cout << "Input temperature (273.0): ";
	std::getline(std::cin, input);

	if (!input.empty()) {
		rp.setTemperature(atof(input.c_str()));
		input.clear();
	}

	//std::cout << "NUMBER OF TESTS 100--->" << std::endl;

	double cr1[3];
	double cr2[3];
	rp.get_3D_Cr(cr1);
	rp.get_3D_Cr(cr2);
	double aCoord[3];
	double bCoord[3];
	rp.get_coordinates(aCoord);
	rp.get_coordinates(bCoord);

	//Changing frame of reference to b
	double newV[3] = { cr1[0] - cr2[0] , cr1[1] - cr2[1] , cr1[2] - cr2[2] };
	std::cout << "newV : " << newV[0] << " " << newV[1] << " " << newV[2] << std::endl;

	std::cout << "1st Cr of air molecules : " << *(cr1 + 0) << " , " << *(cr1 + 1) << " , " << *(cr1 + 2) << std::endl;
	std::cout << "Coordinates : " << *(aCoord + 0) << " , " << *(aCoord + 1) << " , " << *(aCoord + 2) << std::endl;
	std::cout << "2nd Cr of air molecules : " << *(cr2 + 0) << " , " << *(cr2 + 1) << " , " << *(cr2 + 2) << std::endl;
	std::cout << "Coordinates : " << *(bCoord + 0) << " , " << *(bCoord + 1) << " , " << *(bCoord + 2) << std::endl;

	std::cout << "Cr_ref of hydrogen : " << rp.get_CrRef() << std::endl;

	std::cout << "B of particles : " << rp.get_B(aCoord, bCoord, newV) << std::endl;

	std::cout << "SENDING TO MODELS--->" << std::endl;

	/*********************************
	Collision dynamics (Deflection angle)
	b : miss-distance impact parameter 
	m : mass
	c : molecular velocity vector
	k : constant kappa
	n : exponent eta
	**********************************/
	//Gas g = Gas(0.333, rp.getMass(), rp.getCoefficientOfViscosity(), rp.getViscosityIndex());
	//CollisionDynamics collDy(b, cRef, g);

	//std::cout << "\n\n\n//////////////GAS TEST START///////////////" << std::endl;

	//Gas Hydrogen_Gas = Gas(3.0, 3.34 * pow(10, -27), 0.845 * pow(10, -5), 0.67);
	//CollisionDynamics collDy2(0.5, 2.0, Hydrogen_Gas);

	//std::cout << "Hydrogen gas kappa : " << Hydrogen_Gas.getKappa() << std::endl;

	/*********************************
	Variable hard sphere collision
	b : miss-distance impact parameter 
	d : diameter
	c : molecular velocity vector
	v : w - 1/2 relative speed exponent
	**********************************/
	double d = 2.92 * pow(10, -8);// = rp.get_DRef() * pow(rp.get_CrRef() / newV, rp.viscosity_index - 0.5);
	
	VHS vhs;
	std::vector<Eigen::Vector3d> vhsFinalVelocities = vhs.run(rp.get_B(aCoord, bCoord, newV), d, aCoord, bCoord, cr1, cr2);
	
	std::stringstream str1;
	str1 << vhsFinalVelocities.at(0);
	std::string vhsFinalVelocity1 = str1.str();
	vhsFinalVelocity1.erase(std::remove(vhsFinalVelocity1.begin(), vhsFinalVelocity1.end(), '\n'), vhsFinalVelocity1.end());

	std::stringstream str2;
	str2 << vhsFinalVelocities.at(1);
	std::string vhsFinalVelocity2 = str2.str();
	vhsFinalVelocity2.erase(std::remove(vhsFinalVelocity2.begin(), vhsFinalVelocity2.end(), '\n'), vhsFinalVelocity2.end());
	
	dataFile << aCoord[0] << "|" << aCoord[1] << "|" << aCoord[2] << ","
		<< cr1[0] << "|" << cr1[1] << "|" << cr1[2] << "|" << ","
		<< bCoord[0] << "|" << bCoord[1] << "|" << bCoord[2] << ","
		<< cr2[0] << "|" << cr2[1] << "|" << cr2[2] << "|" << ","
		<< vhsFinalVelocity1 << ","
		<< vhsFinalVelocity2 << std::endl;
	
	std::cout << "PROGRAM END..." << std::endl;
    return 0;
}