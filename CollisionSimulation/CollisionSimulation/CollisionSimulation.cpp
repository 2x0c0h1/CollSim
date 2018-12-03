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
#include "A2eta.h"

#include <gsl/gsl_integration.h>
#include <gsl/gsl_roots.h>
#include <gsl/gsl_errno.h>

#include <Eigen/Dense>

int main()
{
	std::cout << "PROGRAM START--->" << std::endl;
	
	//Get A_2(eta)
	std::string etaInput;
	std::cout << "Input eta to get A_2(eta) : ";
	std::getline(std::cin, etaInput);
	double a2eta = A2eta::getA2eta(atof(etaInput.c_str()));
	std::cout << "A_2(eta) : " << a2eta << std::endl;

	RandomParameters rp;
	//double cov = (5 * rp.getMass() * pow(8.314 * rp.getTemperature() / 3.1415, 0.5) * pow(2 * rp.getMass() * 8.314 * rp.getTemperature() / 1.403/*kappa*/, 2/(eta-1)))/(8 * a2eta * tgamma(4-2/(eta-1)));
	//rp.setCoefficientOfViscosity(cov);

	//Data file initialization
	std::ofstream dataFile;
	dataFile.open("data.csv");
	dataFile << "Initial Pos 1,Initial Velocity 1,Initial Pos 2,Initial Velocity 2,B,VHS Deflection Angle,VHS Final Velocity 1, VHS Final Velocity 2, IPL Deflection Angle, IPL Final Velocity" << std::endl;
	
	std::cout << "INPUT VARIBLES FOR RANDOM PARAMETERS--->" << std::endl;

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
	
	for (int i = 0; i < 10; i++) {
		std::cout << "GENERATING DATA SET--->" << std::endl;

		double cr1[3];
		double cr2[3];
		rp.get_3D_Cr(cr1);
		rp.get_3D_Cr(cr2);
		double aCoord[3];
		double bCoord[3];
		rp.get_coordinates(aCoord);
		rp.get_coordinates(bCoord);
		/*cr1[0] = 0;
		cr1[1] = 1;
		cr1[2] = 0;
		cr2[0] = 0;
		cr2[1] = 0.5;
		cr2[2] = 0;
		aCoord[0] = 0;
		aCoord[1] = 0;
		aCoord[2] = 0;
		bCoord[0] = 1 * pow(10,-i*2);
		bCoord[1] = 2;
		bCoord[2] = 0;*/

		//Changing frame of reference
		double newV[3] = { cr1[0] - cr2[0] , cr1[1] - cr2[1] , cr1[2] - cr2[2] };
		Eigen::Vector3d newVa(cr1[0] - cr2[0], cr1[1] - cr2[1], cr1[2] - cr2[2]);

		std::cout << "SENDING TO MODELS--->" << std::endl;

		/*********************************
		Collision dynamics (Deflection angle)
		b : miss-distance impact parameter
		m : mass
		c : molecular velocity vector
		k : constant kappa
		n : exponent eta
		**********************************/
		//std::cout << "\n\n\n//////////////GAS TEST START///////////////" << std::endl;

		Gas Hydrogen_Gas = Gas(0.333, rp.getMass(), rp.getTemperature(), rp.getCoefficientOfViscosity(), rp.getViscosityIndex());
		CollisionDynamics collDy2(rp.get_B(aCoord, bCoord, newV), newVa.norm(), Hydrogen_Gas, newV);

		//std::cout << "Hydrogen gas kappa : " << Hydrogen_Gas.getKappa() << std::endl;

		/*********************************
		Variable hard sphere collision
		b : miss-distance impact parameter
		d : diameter
		c : molecular velocity vector
		v : w - 1/2 relative speed exponent
		**********************************/
		double d = rp.get_DRef() * pow(rp.get_CrRef() / newVa.norm(), rp.viscosity_index - 0.5);
		double angle = acos(rp.get_B(aCoord, bCoord, newV) / d) * 180 / 3.1415;

		VHS vhs;
		std::vector<Eigen::Vector3d> vhsFinalVelocities = vhs.run(rp.get_B(aCoord, bCoord, newV), d, aCoord, bCoord, cr1, cr2);
		Eigen::IOFormat CommaInitFmt(6, Eigen::DontAlignCols, "|", "|", "", "", "", "");

		double* iplFinalV = collDy2.getFinalV();

		dataFile << aCoord[0] << "|" << aCoord[1] << "|" << aCoord[2] << ","
			<< cr1[0] << "|" << cr1[1] << "|" << cr1[2] << "|" << ","
			<< bCoord[0] << "|" << bCoord[1] << "|" << bCoord[2] << ","
			<< cr2[0] << "|" << cr2[1] << "|" << cr2[2] << "|" << ","
			<< rp.get_B(aCoord, bCoord, newV) << ","
			<< angle << ","
			<< vhsFinalVelocities.at(0).format(CommaInitFmt) << ","
			<< vhsFinalVelocities.at(1).format(CommaInitFmt) << "|" << "," 
			<< collDy2.getDeflectionAngle() << ","  
			<< iplFinalV[0] << "|" << iplFinalV[1]<< "|" << iplFinalV[2] << std::endl;
	}

	std::cout << "PROGRAM END..." << std::endl;
    return 0;
}