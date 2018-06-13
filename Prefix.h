#undef UNICODE
#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>	//Socket Programming
#include <winsock2.h>	//Socket Programming
#include <ws2tcpip.h>	//Socket Programming

#include <stdlib.h>		//Converting char to string etc.
#include <stdio.h>		//Read, write access (printf)
#include <iostream>		//Read, write access (cout, cin)
#include <string>		//String operations (parsing)
#include <math.h>		//Log, pow etc.
#include <vector>		//Vector operations
#include <thread>		//Creating, joining threads
#include <mutex>		//Synchronization
#include <chrono>		//Measuring time
#include <random>		//Generating random numbers

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 1024*512
#define DEFAULT_PORT "27015"

using namespace std;

class Prefix {

private:
	//Member Variables & Constants
	/*Critical Section*/
	int *A = NULL;
	/******************/
	int n, result = -1;
	int algorithm = 0; double ranRange[2] = { 1, 10 };
	string algName[5] = { "Seq", "Rec", "Par", "Dis", "NULL" };
	int arrType = 4, thrCnt = -1;
	bool debug = false, debugRes = true;

public:
	//CONSTRUCTORS & DECONSTRUCTORS
	Prefix();
	Prefix( int n);
	Prefix( int n, int arrType, bool debug, bool debugRes);
	Prefix(int n, int thrCnt, int arrType, bool debug, bool debugRes);
	~Prefix();

	/****************DRIVER FUNCTIONS****************/
	//DISTRIBUTED Odd/Even Prefix
	void prefixDis();
	void prefixDis(char * addr);
	void prefixDisServer();
	void prefixDisClient(char * addr);
	
	//Parallel Odd/Even Prefix
	void prefixPar();
	void prefixParWorker(int id);

	//Recursive Odd/Even Prefix
	void prefixRec();
	void prefixRec(int i, int j);

	//Sequential Odd/Even Prefix
	void prefixSeq();

	/************************************************/
	
	//Usefull Functions
	void menu(int argc, char* argv[]);

	//GETTERS & SETTERS
	int * getArr();
	int getVal(int i);
	void setArr(int * arr);
	void setRes();

	//Internal Functions
	void iniArr();
	void delArr();
	string arrToStr(int * arr, int n);
	int * strToArr(string str);
	int * arrFirstHalf(int * arr, int n);
	int * arrSecondHalf(int * arr, int n);
	int * arrMerge(int * arr1, int * arr2, int n);
	void printArr();
	void printArr(int *A, int n);
	void printRes();
	void test(int alg);
	void testAll(int range);

	//Test
	void HelloWorld();
};
