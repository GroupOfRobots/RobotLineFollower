#include "Pid.h"

Pid::Pid(double K, double Ti, double Td, double T, int setPoint, int uWorkPoint, int uMin, int uMax){
	this->K = K;
	this->Ti = Ti;
	this->Td = Td;
	this->T = T;
	this->uMin = uMin;
	this->uMax = uMax;
	this->setPoint = setPoint;
	this->uiPast = 0;
	this->ePast = 0;
	this->uWorkPoint = uWorkPoint;
}

std::pair<int, int> Pid::calculateControl(int processOutput){
	int e = setPoint - processOutput;
	double up = K*e;
	double ui = uiPast + (K/Ti) * T * (ePast+e)/2;
	double ud = K*Td*(e-ePast)/T;
	int u = round(up + ui + ud);
	ePast = e;
	uiPast = ui;
	if(u>uMax) u=uMax;
	if(u<uMin) u=uMin;
	return std::make_pair(uWorkPoint-u, uWorkPoint+u);
}
