#include <iostream>
#include <string>
#include <stdio.h>
#include <cmath>
using namespace std;

class Pid {
	private:
		int uMax;
		int uMin;
		int setPoint;
		int uWorkPoint;
		double K;
		double Ti;
		double Td;
		double T;
		int uiPast;
		int ePast;
		
	public:
		Pid(double K, double Ti, double Td, double T, int setPoint, int uWorkPoint, int uMin, int uMax);
		std::pair<int, int> calculateControl(int processOutput);
};

