// CollisionSimulation.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>

using namespace std;

int main()
{
	cout << "Starting program" << endl;
	




	cout << "Program ended" << endl;
	system("pause");
    return 0;
}

int vhs()
{



}

class Particle
{
	float diameter;
	Vector intialVelocity;
	Vector finalVelocity;
	public:
		Particle(float diameter)
		{
			cout << "Created Particle with diameter " << diameter << endl;
			diameter = diameter;
		}

		Vector getinitialVelocity()
		{
			cout << initialVelocity << endl;
		}
};
