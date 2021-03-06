#include "stdafx.h"
#include "RandomParameters.h"
#include <random>
#include <chrono>
#include <iostream>

extern bool collDyFlag;

RandomParameters::RandomParameters() {
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
	double standard_deviation = sqrt(1.380658e-23 * getTemperature() / getMass());
	std::normal_distribution<double> distribution(mean, standard_deviation);
	Eigen::Vector3d cr_vector(distribution(generator), distribution(generator), distribution(generator));
	return cr_vector;
}

Eigen::Vector3d RandomParameters::get_coordinates_at_contact(double n, double diameter, Eigen::Vector3d cr1, Eigen::Vector3d cr2) {
	double d;
	double pressure = (101325. / 273) * getTemperature();
	
	// n  >>  p = n * k * T 
	n = pressure / (T * k); 

	unsigned seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);

	// Get d
	std::uniform_real_distribution<double> unif(0, diameter * diameter);
	d = sqrt(unif(generator));
	//std::cout << "d : " << d << std::endl;

	double l_norm;
	if (1 / (n * 3.1415 * diameter * diameter) > 0){
		// Get l
		std::gamma_distribution<double> gamma(1.0, 1 / (n * 3.1415 * diameter * diameter));
		l_norm = gamma(generator);
	
		//Unit vector
		l = (l_norm / cr1.norm()) * cr1;

		double angle = atan(d / l_norm);
		double psi = asin(l.data()[2] / l.norm());

		if (cos(psi) != 0) {
			double l_prime_x = (l.data()[0] / cos(psi)) * cos(psi + angle);
			double l_prime_y = (l.data()[1] / cos(psi)) * cos(psi + angle);
			double l_prime_z = (l.data()[2] / sin(psi)) * sin(psi + angle);

			l_prime = Eigen::Vector3d(l_prime_x, l_prime_y, l_prime_z);
		}
		else {
			double l_prime_z = (l.data()[2] / sin(psi)) * sin(psi + angle);

			l_prime = Eigen::Vector3d(0, 0, l_prime_z);
		}
		l_prime = (l_prime / l_norm) * (sqrt(pow(d, 2) + pow(l_norm, 2)));

		//Generate random angle
		std::uniform_real_distribution<double> unifAngle(0, 2 * 3.1415);
		double randomAngle = unifAngle(generator);

		Eigen::Vector3d a_parr_b = l;
		Eigen::Vector3d a_perp_b = l_prime - l;
		Eigen::Vector3d w = cr1.cross(a_perp_b);

		Eigen::Vector3d coord = a_perp_b.norm() * ((cos(randomAngle) / a_perp_b.norm()) * a_perp_b + (sin(randomAngle) / w.norm()) * w) + a_parr_b;

		//std::cout << "Checking l* length = " << l_prime.norm() << " |||| sqrt(d^2 + l^2) = " << (sqrt(pow(d, 2) + pow(l_norm, 2))) << " |||| theta = " << angle << " |||| angle between l* and cr = " << acos(l_prime.dot(cr1) / (l_prime.norm() * cr1.norm())) << std::endl;
		double delta_l = sqrt(pow(diameter, 2) - pow(d, 2));
		time = (l.norm() - delta_l) / cr1.norm();

		coord = coord - (cr2 * time);

		return coord;
	} else {
		collDyFlag = true;
		return Eigen::Vector3d(0, 0, 0);
	}
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

void RandomParameters::setV() {
	v = viscosity_index - 0.5;
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

double RandomParameters::getTimeToCollision() {
	return time;
}