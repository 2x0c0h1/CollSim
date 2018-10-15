#include "stdafx.h"
#include "RandomParameters.h"
#include <random>
#include <chrono>
#include <iostream>

RandomParameters::RandomParameters() {
}

double RandomParameters::get_DRef() {
	//Currently just set to 1.0
	return 1.0;
}

double RandomParameters::get_CrRef() {
	//Total collision cross section
	double sigmaT = 3.1415 * 2.92 * pow(10, -10) * 2.92 * pow(10, -10); //Diameter of hydrogen : 1.2 * 10^-10 TODO Get from book
	double inner = (15/8 * sqrt(3.1415*mass*k) * pow(4*k/mass, v) * pow(T, 0.5+v)) /
					(viscosity_index * sigmaT * tgamma(4-v));
	double Cr_ref = pow(sqrt(inner), 1/v);
	return Cr_ref;
}

double* RandomParameters::get_3D_Cr() {
	//From normal distribution of velocity of air molecules generate random 3D vector Cr
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	int mean = 0;
	double standard_deviation = sqrt(8.314 * 298);
	std::normal_distribution<double> distribution(mean, standard_deviation);
	
	double cr_vector[3] = { distribution(generator) , distribution(generator) , distribution(generator) };
	
	return cr_vector;
}

double RandomParameters::get_B() {
	double alpha = 1; //1 for now
	double meanFreePath = (4*alpha*(5-2*viscosity_index)*(7-2*viscosity_index))/
							(5 * (alpha + 1) * (alpha + 2)) * 
							sqrt(mass / (2 * 3.1415 * k * T)) *
							(coefficient_of_viscosity / density);
	std::cout << "Mean free path for hydrogen : " << meanFreePath << std::endl;
	
	//Choose 2 points within cube of width meanFreePath
	std::uniform_real_distribution<double> unif(0, meanFreePath);
	std::default_random_engine re;
	double a_coordinate[3] = { unif(re), unif(re), unif(re)};
	double b_coordinate[3] = { unif(re), unif(re), unif(re)};

	std::cout << "Random coordinates : " << a_coordinate[0] << "," << a_coordinate[1] << "," << a_coordinate[2] << std::endl;

	return 0.0f;
}


RandomParameters::~RandomParameters() {
}
