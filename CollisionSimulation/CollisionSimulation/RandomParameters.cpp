#include "stdafx.h"
#include "RandomParameters.h"
#include <random>
#include <chrono>
#include <iostream>

RandomParameters::RandomParameters() {
}

double RandomParameters::get_DRef() {
	//Currently just set to 1.0
	return 2.96e-10;
}

double RandomParameters::get_CrRef() {
	//Total collision cross section
	double sigmaT = 3.1415 * diameter * diameter;
	double inner = (15./8. * sqrt(3.1415*mass*k) * pow(4*k/mass, v) * pow(T, 0.5+v)) /
					(coefficient_of_viscosity * sigmaT * tgamma(4-v));
	double Cr_ref = pow(sqrt(inner), 1/v);
	return Cr_ref;
}

Eigen::Vector3d RandomParameters::get_3D_Cr() {
	//From normal distribution of velocity of air molecules generate random 3D vector Cr
	unsigned seed = (unsigned) std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	int mean = 0;
	double standard_deviation = sqrt(1.380658e-23/ 3.34e-27 * 273);
	std::normal_distribution<double> distribution(mean, standard_deviation);
	Eigen::Vector3d cr_vector(distribution(generator), distribution(generator), distribution(generator));
	return cr_vector;
}

Eigen::Vector3d RandomParameters::get_coordinates_at_contact(double diameter, Eigen::Vector3d cr1) {
	double n = 101325 / (273 * k) * mass; //TODO Get n  >>  p = n * (k/m) * T 

	unsigned seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	
	//Get d
	std::uniform_real_distribution<double> unif(0 , diameter * diameter);
	double d = sqrt(unif(generator));
	std::cout << "d : " << d << std::endl;

	//Get l
	std::gamma_distribution<double> gamma(1.0, n * 3.1415 * diameter * diameter);
	double l = gamma(generator);

	Eigen::Vector3d scaled = (l / cr1.norm()) * cr1;

	double angle = atan(d/l);

	Eigen::Matrix3d rotationMatrix;
	rotationMatrix << 1, 0, 0,
		0, cos(angle), -sin(angle),
		0, sin(angle), cos(angle);

	Eigen::Vector3d raisedByd = rotationMatrix * scaled;

	//Generate random angle
	std::uniform_real_distribution<double> unifAngle(0, 2 * 3.1415);
	double randomAngle = unifAngle(generator);

	//Rotating raisedByd by random angle about cr1
	Eigen::Vector3d a_parr_b = raisedByd.dot(cr1) / cr1.dot(cr1) * cr1;
	Eigen::Vector3d a_perp_b = raisedByd - a_parr_b;
	Eigen::Vector3d w = cr1.cross(a_perp_b);

	Eigen::Vector3d coord = a_perp_b.norm() * ((cos(randomAngle) / a_perp_b.norm()) * a_perp_b + (sin(randomAngle) / w.norm()) * w) + a_parr_b;
	return coord;
}

Eigen::Vector3d RandomParameters::get_coordinates() {
	double alpha = 1.35; //1 for now
	double meanFreePath = (4*alpha*(5-2* viscosity_index)*(7-2* viscosity_index))/
							(5 * (alpha + 1) * (alpha + 2)) * 
							sqrt(mass / (2 * 3.1415 * k * T)) *
							(coefficient_of_viscosity / density) * 0.08;

	//std::cout << "Mean free path for hydrogen : " << meanFreePath << std::endl;
	
	//Choose 2 points within cube of width meanFreePath
	unsigned seed = (unsigned) std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<double> unif(0, meanFreePath);
	Eigen::Vector3d coord(unif(generator), unif(generator), unif(generator));
	return coord;
}

double RandomParameters::get_B(Eigen::Vector3d a_coord, Eigen::Vector3d b_coord, Eigen::Vector3d newV) {
	double numerator = (newV.cross(a_coord - b_coord)).norm();
	double denominator = newV.norm();
	return numerator / denominator;
}

RandomParameters::~RandomParameters() {
}

void RandomParameters::setCoefficientOfViscosity(double cov) {
	coefficient_of_viscosity = cov;
}

void RandomParameters::setMass(double mass) {
	this->mass = mass;
}

void RandomParameters::setViscosityIndex(double vi) {
	viscosity_index = vi;
}

void RandomParameters::setDiameter(double diameter) {
	this->diameter = diameter;
}

void RandomParameters::setTemperature(double T) {
	this->T = T;
}

double RandomParameters::getCoefficientOfViscosity() {
	return coefficient_of_viscosity;
}

double RandomParameters::getMass() {
	return mass;
}

double RandomParameters::getViscosityIndex() {
	return viscosity_index;
}

double RandomParameters::getDiameter() {
	return diameter;
}

double RandomParameters::getTemperature() {
	return T;
}