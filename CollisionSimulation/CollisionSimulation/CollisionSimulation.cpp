// CollisionSimulation.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <fstream>

#include "VHS.h"
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
	
	//Data file initialization
	std::ofstream dataFile;
	dataFile.open("data.csv");
	dataFile << "Initial Pos 1,Initial Velocity 1,Initial Pos 2,Initial Velocity 2,B,VHS Deflection Angle,VHS Final Velocity 1, VHS Final Velocity 2, IPL Deflection Angle, IPL Final Velocity 1, IPL Final Velocity 2" << std::endl;
	
	std::cout << "INPUT VARIBLES FOR RANDOM PARAMETERS--->" << std::endl;

	RandomParameters rp;

	//Get A_2(eta)
	/*std::string etaInput;
	std::cout << "Input eta to get A_2(eta) : ";
	std::getline(std::cin, etaInput);
	double a2eta = A2eta::getA2eta(atof(etaInput.c_str()));
	std::cout << "A_2(eta) : " << a2eta << std::endl;*/

	//double cov = (5 * rp.getMass() * pow(8.314 * rp.getTemperature() / 3.1415, 0.5) * pow(2 * rp.getMass() * 8.314 * rp.getTemperature() / 1.403/*kappa*/, 2/(eta-1)))/(8 * a2eta * tgamma(4-2/(eta-1)));
	//rp.setCoefficientOfViscosity(cov);

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

		Eigen::Vector3d cr1 = rp.get_3D_Cr();
		Eigen::Vector3d cr2 = rp.get_3D_Cr();
		Eigen::Vector3d aCoord = rp.get_coordinates();
		Eigen::Vector3d bCoord = rp.get_coordinates();
		
		/*Eigen::Vector3d cr1(0,1,0);
		Eigen::Vector3d cr2(0,0.5,0);
		Eigen::Vector3d aCoord(0,0,0);
		Eigen::Vector3d bCoord(1 * pow(10, -i * 2), 2, 0);*/

		//Changing frame of reference
		Eigen::Vector3d newV = cr1 - cr2;

		double b = rp.get_B(aCoord, bCoord, newV);

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
		CollisionDynamics collDy2(b, newV.norm(), Hydrogen_Gas, cr1, cr2, newV);

		//std::cout << "Hydrogen gas kappa : " << Hydrogen_Gas.getKappa() << std::endl;

		/*********************************
		Variable hard sphere collision
		b : miss-distance impact parameter
		d : diameter
		c : molecular velocity vector
		v : w - 1/2 relative speed exponent
		**********************************/
		double d = rp.get_DRef() * pow(rp.get_CrRef() / newV.norm(), rp.viscosity_index - 0.5);
		double angle = acos(b / d) * 180 / 3.1415;

		VHS vhs;
		std::vector<Eigen::Vector3d> vhsFinalVelocities = vhs.run(b, d, aCoord, bCoord, cr1, cr2);
		Eigen::IOFormat CommaInitFmt(6, Eigen::DontAlignCols, "|", "|", "", "", "", "");

		Eigen::Vector3d iplFinalV1 = collDy2.getFinalV1();
		Eigen::Vector3d iplFinalV2 = collDy2.getFinalV2();

		//Writing to file
		dataFile << aCoord.data()[0] << "|" << aCoord.data()[1] << "|" << aCoord.data()[2] << ","
			<< cr1.data()[0] << "|" << cr1.data()[1] << "|" << cr1.data()[2] << "|" << ","
			<< bCoord.data()[0] << "|" << bCoord.data()[1] << "|" << bCoord.data()[2] << ","
			<< cr2.data()[0] << "|" << cr2.data()[1] << "|" << cr2.data()[2] << "|" << ","
			<< b << ","
			<< angle << ","
			<< vhsFinalVelocities.at(0).format(CommaInitFmt) << ","
			<< vhsFinalVelocities.at(1).format(CommaInitFmt) << "|" << ","
			<< collDy2.getDeflectionAngle() << ","
			<< iplFinalV1.format(CommaInitFmt) << ","
			<< iplFinalV2.format(CommaInitFmt) << std::endl;
	}

	std::cout << "PROGRAM END..." << std::endl;
	system("pause");
    return 0;
}