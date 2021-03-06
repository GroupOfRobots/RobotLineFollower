#include "bcm2835.h"
#include "l6470constants.h"
#include "motors.h"
#include <csignal>
#include <math.h>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <ctime>
#include <chrono>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <thread>
#include "UdpJpgFrameStreamer.h"
#include "ContourFinding.h"
#include "CenterFinding.h"
#include "Pid.h"
using namespace cv;
using namespace std;

using boost::asio::ip::udp;

#define GPIO_TOF_1 	RPI_V2_GPIO_P1_12
#define GPIO_TOF_2 	RPI_V2_GPIO_P1_16
#define GPIO_TOF_3 	RPI_V2_GPIO_P1_18
#define GPIO_TOF_4 	RPI_V2_GPIO_P1_29
#define GPIO_TOF_5	RPI_V2_GPIO_P1_32
#define GPIO_TOF_6 	RPI_V2_GPIO_P1_31
#define GPIO_TOF_7 	RPI_V2_GPIO_P1_33
#define GPIO_TOF_8 	RPI_V2_GPIO_P1_35
#define GPIO_TOF_9 	RPI_V2_GPIO_P1_36
#define GPIO_TOF_10 RPI_V2_GPIO_P1_37
#define NDEBUG

void stepperTest();
Motors *globalBoard;
uint16_t measurement[10];

void sigintHandler(int signum) {
	if (signum == SIGINT) {
		//globalBoard->Dump();
		globalBoard->stop();
		std::cout<<"TASK TERMINATED"<<endl;
		exit(signum);
	}
}


int main()
{
	//setup motors
	signal(SIGINT, sigintHandler);
	if (bcm2835_init() == 0) {
			fprintf(stderr, "Not able to init the bmc2835 library\n");
			return -1;
	}
	Motors board( BCM2835_SPI_CS0, GPIO_RESET_OUT);
	globalBoard = &board;
	board.setUp();
	board.resetPosition();
	//-----------------------------------------------------

	//setup camera
	UdpJpgFrameStreamer streamer(2024, 64000, 80);
	ContourFinding contourFinder(1.0/3.0, 2.0/3.0);
	CenterFinding centerFinder(6);
	Mat src;
	std::cout<<"Contour or center finding? (1/2)";
	int method;
	std::cin>>method;
	VideoCapture clipCapture(0);
	//sprawdzenie czy wczytano poprawnie
	if (!clipCapture.isOpened())
	{
	  	cout  << "Could not open reference to clip" << endl;
		exit(0);
	}
	streamer.run();
	//-----------------------------------------------------

	//main loop
	while(1){
		clipCapture.read(src);
			
		if (src.empty() || src.cols == -1 || src.rows == -1)
		{
		    	printf("No image data from clip\n");
		    	break;
		}

		else
		{	
			if(method == 1)
			{
				contourFinder.setFrame(src);
				contourFinder.setScaleFactor(0.3);//default is 0.5
				contourFinder.setThreshold(50);
				std::vector<cv::Point> centers = contourFinder.findLineCenters();
				Mat frame = contourFinder.drawPoints(centers);
				streamer.pushFrame(frame);

				//pid
				int center = round(contourFinder.getSourceFrame().cols/2);
				Pid pid(1, 10000, 0, 0.1, center, 20, 0, 40);
				pair<int, int> p = pid.calculateControl(centers[0].x);
				std::cout<<"Speed: "<< -p.first << ", " << -p.second <<endl;
				board.setSpeed(-p.first, -p.second);
			}

			else
			{
				centerFinder.setFrame(src);
				centerFinder.setScaleFactor(0.3);//default is 0.5
				std::vector<cv::Point> centers = centerFinder.findLineCenters();
				Mat frame  = centerFinder.drawPoints(centers);
				streamer.pushFrame(frame);
			}
     	}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		//clipCapture.release();
		//break;
	}

  return 0;
}

void stepperTest(){
	long positionLeft,positionRight;
	Motors board( BCM2835_SPI_CS0, GPIO_RESET_OUT);
	globalBoard = &board;
	board.setUp();
	board.resetPosition();
	positionLeft = board.getPositionLeft();
	positionRight = board.getPositionRight();
	printf("Absolute position: Left:%lu		Right:%lu \n",positionLeft, positionRight);
	board.setSpeed(-20,-20);
	bcm2835_delay(500);
	positionLeft = board.getPositionLeft();
	positionRight = board.getPositionRight();
	printf("Absolute position: Left:%lu		Right:%lu \n",positionLeft, positionRight);
	board.stop();
}
