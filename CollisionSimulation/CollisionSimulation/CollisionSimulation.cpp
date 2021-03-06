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

#include <chrono>
#include <ctime>

#include <Eigen/Dense>

#include "path.h"

bool collDyFlag;

int main() {
	std::cout << "PROGRAM START--->" << std::endl;

	//Data file initialization
	std::ofstream dataFile, dataFile_difference;
	dataFile.open("data.csv");
	//dataFile << "Initial Pos 1,Initial Velocity 1,Initial Pos 2,Initial Velocity 2,Time,VHS_d,B,VHS Deflection Angle,VHS Final Velocity 1, VHS Final Velocity 2, IPL Deflection Angle, IPL Final Velocity 1, IPL Final Velocity 2, Initial energy, Final energy vhs, Final energy ipl e" << std::endl;
	dataFile << "b, VHS Deflection Angle, IPL Deflection Angle" << std::endl;
	dataFile_difference.open("data2.csv");
	dataFile_difference << "Number density, difference" << std::endl;

	RandomParameters rp;

	Gas Hydrogen_Gas = Gas(0.333, 3.34e-27, 273.0, 0.845e-5, 0.67, 2.92e-10);
	Gas Neon = Gas(0.333, 33.5e-27, 273.0, 2.975e-5, 0.66, 2.77e-10);
	Gas Argon = Gas(0.333, 66.3e-27, 273.0, 2.117e-5, 0.81, 4.17e-10);
	Gas Krypton = Gas(0.333, 139.1e-27, 273.0, 2.328e-5, 0.8, 4.76e-10);
	Gas Xenon = Gas(0.333, 218e-27, 273.0, 2.107e-5, 0.85, 5.74e-10);
	Gas Gas = Xenon;

	rp.setCoefficientOfViscosity(Gas.getViscosity_co());
	rp.setMass(Gas.getMol_mass());
	rp.setDiameter(Gas.getDiameter());
	rp.setTemperature(Gas.getTemperature());
	rp.setViscosityIndex(Gas.getViscosity_index());
	rp.setV();

	Eigen::IOFormat CommaInitFmt(6, Eigen::DontAlignCols, "|", "|", "", "", "", "");

	double difference = 0;
	int iterations = 2000;
	//double dN = 101325 / (1.380658e-23 * 100);
	double n = 101325. / (1.380658e-23 * 100);

	for (int u = 100; u < 450; u++) {
	//for (; n < dN * 200;n += dN) {
		rp.setTemperature(u);
		difference = 0;
		std::cout << u << std::endl;

		//double time = 0;
		//for (int j = 0; j < 3; j++) {

			iterations = 2000;

			//auto start = std::chrono::system_clock::now();
			
			for (int i = 0; i < iterations; i++) {
				collDyFlag = false;

				Eigen::Vector3d cr1 = rp.get_3D_Cr();
				Eigen::Vector3d cr2 = rp.get_3D_Cr();

				//Changing frame of reference
				Eigen::Vector3d newV = cr1 - cr2;

				double VHS_d = rp.getDiameter() * pow(rp.get_CrRef() / newV.norm(), rp.viscosity_index - 0.5);

				Eigen::Vector3d aCoord(0, 0, 0);
				Eigen::Vector3d bCoord = rp.get_coordinates_at_contact(n, rp.getDiameter(), cr1, cr2);

				double b = rp.get_B(aCoord, bCoord, newV);

				/*********************************
				Collision dynamics (Deflection angle)
				b : miss-distance impact parameter
				m : mass
				c : molecular velocity vector
				k : constant kappa
				n : exponent eta
				**********************************/
				Eigen::Vector3d xAxis(1, 0, 0);
				Eigen::Vector3d yPrime(0, newV.data()[2] * pow(pow(newV.data()[1], 2) + pow(newV.data()[2], 2), -0.5), -newV.data()[1] * pow(pow(newV.data()[1], 2) + pow(newV.data()[2], 2), -0.5));
				Eigen::Vector3d collisionPlane;
				double epsilon;

				// If cr is in the same direction as the axis, our collision plane will be 0
				// hence an if else case is required for this particular case
				if ((aCoord - bCoord).cross(newV).norm() == 0) {
					Eigen::Vector3d shifted_cr1(cr1.data()[0] + 0.1, cr1.data()[1], cr1.data()[2]);
					collisionPlane = (aCoord - bCoord).cross(shifted_cr1 - cr2);
				}
				else {
					collisionPlane = (aCoord - bCoord).cross(newV);
				}

				// If epsilon is < 0 (wrong direction), we will take -epsilon instead
				if (yPrime.cross(collisionPlane).dot(newV) < 0) {
					epsilon = acos(yPrime.dot(collisionPlane) / (yPrime.norm() * collisionPlane.norm()));
				}
				else {
					epsilon = -acos(yPrime.dot(collisionPlane) / (yPrime.norm() * collisionPlane.norm()));
				}

				CollisionDynamics collDy(b, newV.norm(), Hydrogen_Gas, cr1, cr2, newV, epsilon);

				if (collDyFlag) {
					iterations--;
					continue;
				}

				/*********************************
				Variable hard sphere collision
				b : miss-distance impact parameter
				d : diameter
				c : molecular velocity vector
				v : w - 1/2 relative speed exponent
				**********************************/
				VHS vhs(b, VHS_d);
				std::vector<Eigen::Vector3d> vhsFinalVelocities = vhs.run(aCoord, bCoord, cr1, cr2);

				Eigen::Vector3d iplFinalV1 = collDy.getFinalV1();
				Eigen::Vector3d iplFinalV2 = collDy.getFinalV2();

				//Writing to file
				/*dataFile << b << ","
					<< vhs.getDeflectionAngle() << ","
					<< collDy.getDeflectionAngle() << std::endl;*/

				Eigen::Vector3d cr1_difference = vhsFinalVelocities.at(0) - iplFinalV1;
				Eigen::Vector3d cr2_difference = vhsFinalVelocities.at(1) - iplFinalV2;
				if (!std::isnan(pow(cr1_difference.norm(), 2) + pow(cr2_difference.norm(), 2))) {
					difference += pow(cr1_difference.norm(), 2) + pow(cr2_difference.norm(), 2);
				}
				else {
					iterations--;
				}
			}

			/*auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end - start;
			time += elapsed_seconds.count();
		}*/
		//std::cout << "Time : " << time << std::endl;
		//dataFile_difference << u << "," << time/3 << std::endl;
		dataFile_difference << u << "," << sqrt(difference / iterations) << std::endl;
	}

	std::cout << "PROGRAM END..." << std::endl;
	system("pause");
	return 0;
}