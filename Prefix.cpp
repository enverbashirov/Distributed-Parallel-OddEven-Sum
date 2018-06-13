#include "Prefix.h"

//CONSTRUCTORS & DECONSTRUCTORS
Prefix::Prefix() {	if (debug) cout << "\n Prefix initialized.\n" << endl; }
Prefix::Prefix(int n) {
	Prefix::n = n;
	if (debug) cout << "\n Prefix initialized.\n" << endl;
}
Prefix::Prefix(int n, int arrType, bool debug, bool debugRes) {
	Prefix::n = n;
	Prefix::debug = debug;
	Prefix::debugRes = debugRes;
	Prefix::arrType = arrType;
	if (debug) cout << "\n Prefix initialized.\n" << endl;
}
Prefix::Prefix(int n, int thrCnt, int arrType, bool debug, bool debugRes) {
	Prefix::n = n;
	Prefix::debug = debug;
	Prefix::debugRes = debugRes;
	Prefix::arrType = arrType;
	Prefix::thrCnt = (thrCnt == -1) ? n : thrCnt;
	if (debug) cout << "\n Prefix initialized.\n" << endl;
}
Prefix::~Prefix() {	if ( debug ) cout << "\nPrefix destroyed." << endl; }


/****************DRIVER FUNCTIONS****************/	
//DISTRIBUTED Odd/Even Prefix
void Prefix::prefixDis() { //Server
	if (n == 0) { result = 0; return; }
	if (debug) cout << "Dis" << " | Init | " << endl;
	prefixDisServer();
	if (debugRes) printRes();
}
void Prefix::prefixDis(char * addr) { //Client
	prefixDisClient(addr);
}
void Prefix::prefixDisServer() {
	WSADATA wsaData;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iRecResult, iSendResult;
	char *sendbuf = "ready";
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;


	// Initialize Winsock
	iRecResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRecResult != 0) {
		printf("DIS SER | WSAStartup failed with error: %d\n", iRecResult);
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iRecResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iRecResult != 0) {
		printf("DIS SER | getaddrinfo failed with error: %d\n", iRecResult);
		WSACleanup();
		return;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("DIS SER | socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// Setup the TCP listening socket
	iRecResult = ::bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iRecResult == SOCKET_ERROR) {
		printf("DIS SER | bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	freeaddrinfo(result);

	iRecResult = listen(ListenSocket, SOMAXCONN);
	if (iRecResult == SOCKET_ERROR) {
		printf("DIS SER | listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("DIS SER | accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	// No longer need server socket
	closesocket(ListenSocket);

	// COMMUNICATION
	int localn = Prefix::n, n1 = (int)floor((double)localn / 2), n2 = (int)ceil((double)localn / 2);
	int *arr1 = arrFirstHalf(A, localn), *arr2 = arrSecondHalf(A, localn);
	do {
		// SERVER RECIEVE
		iRecResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

		if (iRecResult > 0) {
			if (debug) cout << "DIS SER | Received: " << recvbuf << " Bytes: " << iRecResult << endl;

			// COMMUNICATION CHECK
			if (strcmp(recvbuf, "clientready") == 0) {
				string arrStr = to_string(n1) + "n" + arrToStr(arr1, n1);
				sendbuf = new char[arrStr.size() + 1];
				strcpy_s(sendbuf, arrStr.size() + 1, arrStr.c_str());
			}
			else if (strcmp(recvbuf, "clientleaving") == 0)
				sendbuf = "serverleaving";
			else {
				arr1 = strToArr(recvbuf); arr1[0] += arr2[0];
				Prefix::A = arrMerge(arr1, arr2, localn); Prefix::result = Prefix::A[0];
				sendbuf = "serversuccess";
			}

			// SERVER SEND
			iSendResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("DIS SER | send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return;
			}
			else if (debug) cout << "DIS SER | Sent: " << sendbuf << " Bytes: " << iRecResult << endl;

			if (strcmp(recvbuf, "clientready") == 0) {
				delArr(); Prefix::A = arr2;	Prefix::n = n2;	prefixPar(); Prefix::n = localn;
			}
		}
		else if (strcmp(recvbuf, "clientleaving") == 0) {
			if (debug) printf("DIS SER | Connection closing...\n"); break; }
		else {
			printf("DIS SER | recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}
	} while (iRecResult > 0 && strcmp(recvbuf, "clientleaving") != 0);

	// shutdown the connection since we're done
	iRecResult = shutdown(ClientSocket, SD_SEND);
	if (iRecResult == SOCKET_ERROR) {
		printf("DIS SER | shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return;
}
void Prefix::prefixDisClient(char * addr) {

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,	*ptr = NULL, hints;

	int iRecResult, iSendResult;
	char *sendbuf = "clientready";
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iRecResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRecResult != 0) {
		printf("DIS CLI | WSAStartup failed with error: %d\n", iRecResult);
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iRecResult = getaddrinfo(addr, DEFAULT_PORT, &hints, &result);
	if (iRecResult != 0) {
		printf("DIS CLI | getaddrinfo failed with error: %d\n", iRecResult);
		WSACleanup();
		return;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL;ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("DIS CLI | socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return;
		}

		// Connect to server.
		iRecResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iRecResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("DIS CLI | Unable to connect to server!\n");
		WSACleanup();
		return;
	}

	// COMMUNICATION
	do {
		// CLIENT SEND
		iSendResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf) + 1, 0);
		if (iSendResult == SOCKET_ERROR) {
			printf("DIS CLI | send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return;
		}
		if (debug) cout << "DIS CLI | Sent: " << sendbuf << " Bytes: " << iRecResult << endl;

		// CLIENT RECEIVE
		iRecResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

		if (iRecResult > 0) {
			if (debug) cout << "DIS CLI | Received: "  << recvbuf << " Bytes: " << iRecResult << endl;

			// COMMUNICATION CHECK
			if (strcmp(recvbuf, "serversuccess") == 0)
				sendbuf = "clientleaving";
			else if (strcmp(recvbuf, "serverleaving") == 0)
				break;
			else {
				delArr(); Prefix::A = strToArr( string(recvbuf)); 
				prefixPar(); string arrStr = arrToStr(Prefix::A, Prefix::n);
				sendbuf = new char[arrStr.size() + 1];
				strcpy_s(sendbuf, arrStr.size() + 1, arrStr.c_str());
			}
		}
		else if (iRecResult == 0 || strcmp(recvbuf, "serverleaving") == 0) {
			if (debug) printf("DIS CLI | Connection closed\n");
		}
		else
			printf("DIS CLI | recv failed with error: %d\n", WSAGetLastError());
	} while (iRecResult > 0 && strcmp(recvbuf, "serverleaving") != 0);

	// shutdown the connection since no more data will be sent
	iRecResult = shutdown(ConnectSocket, SD_SEND);
	if (iRecResult == SOCKET_ERROR) {
		printf("DIS CLI | shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return;
}

//PARALLEL Odd/Even Prefix
//Non-Member Variables & Constants
static mutex mtx;
int depth, r, level;
/******************/

void Prefix::prefixPar() { //Main Thread
	if (n == 0 || thrCnt <= 0) { cout << "\n" << "Par" << " | Error | "; result = 0; return; }
	if (debug) cout << "\n" << "Par" << " | Init | ";
	depth = (int)ceil(log2((n))); level = 0; r = 1;

	while (level < depth) { //Thread creations
		vector<thread> threads;
		for (int i = (r * 2), j = 1; i < n - 1 && j < thrCnt; i += (r * 2), j++) {
			if (debug) { mtx.lock(); cout << "\n" << "Par" << " | Creating Child " << i; mtx.unlock(); }
			threads.push_back(thread(&Prefix::prefixParWorker, *this, i));
		}
		if (debug) { mtx.lock(); cout << "\n" << "Par" << " | Creating Parent "; mtx.unlock(); }
		prefixParWorker(0);
		for (auto& th : threads) th.join();
		level++, r *= 2;
	}

	if (debug) cout << "\n" << "Par" << " | Completed. " << endl;

	setRes();
	if (debugRes) printRes();
}
void Prefix::prefixParWorker(int id) { //Child Threads

	for (int i = id; ((i % (r * 2)) == 0 && (i + r < Prefix::n)); i += ((thrCnt)*(r*2))) {
		//if () { return; } // Exit condition
		A[i] += A[i + r]; //Addition operation
		if (debug) { mtx.lock(); cout << "\nPar | id:" << id << " r:" << r << " ind:[" << i << "]=" << A[i]; mtx.unlock(); }
		if (thrCnt == 0) return;
	}
}

//RECURSIVE Odd/Even Prefix
void Prefix::prefixRec() {
	if (n == 0) { result = 0; return; }
	if (n > 1) {
		int j = (int)pow(2, ceil(log2(n))), r = j / 2, i = j - r;
		prefixRec(i, j);
	}
	setRes();
	if (debugRes) printRes();
	if (debug && getVal(0) != n) cout << "Rec | Result: " << getVal(0) << " != n:" << n << endl;
}
void Prefix::prefixRec(int i, int j) {
	int jt = j / 2, rt = jt / 2, it = jt - rt;
	int r = j / 2;
	if (j > 2 && i == (j-r))
		prefixRec(it, j / 2);
	if (i + j < n)
		prefixRec(i + j, j);
	A[i - (j - r)] += A[i];
	if (debug) {
		cout << "\nRec | j:" << j << " i:" << i << " r:" << r;
		cout << " ind:[" << i - (j - r) << "]=" << A[i - (j - r)];
	}
}

//SEQUENTIAL Odd/Even Prefix
void Prefix::prefixSeq() {
	if (n == 0) { result = 0; return; }
	for (int j = 2; j <= (int)pow(2, ceil(log2(n))); j = j * 2) {
		int r = j / 2;
		for (int i = j - r; i < n; i += j) {
			A[i - (j - r)] += A[i];
			if (debug) {
				cout << "\nSeq | j:" << j << " i:" << i << " r:" << r;
				cout << " ind:[" << i - (j - r) << "]=" << A[i - (j - r)];
			}
		}
	}
	setRes();
	if (debugRes) printRes();
}
/************************************************/

//Internal Functions
void Prefix::iniArr() {
	if (n == 0) { A = NULL; return; }
	delArr();
	A = new int[n];
	
	random_device rd;
	mt19937 mt(rd());
	uniform_real_distribution<double> dist(ranRange[0], ranRange[1]);

	for (int i = 0; i < n; i++) {
		switch (arrType) {
			case 0: A[i] = 1; break;
			case 1: A[i] = i; break;
			case 2: A[i] = (int)dist(mt); break;
			default: A[i] = 1; break;
		}
	}
}
void Prefix::delArr() {
	delete[] A;
	A = NULL;
}
string Prefix::arrToStr(int * arr, int n) {
	string str;
	for (int i = 0; i < n; i++) str += to_string(arr[i]) + ",";
	return str;
}
int * Prefix::strToArr(string str) {
	string delimiter;
	size_t pos = 0;
	string token;
	delimiter = "n";
	if ((pos = str.find(delimiter)) != string::npos) {
		token = str.substr(0, pos);
		Prefix::n = stoi(token);
		str.erase(0, pos + delimiter.length());
	} delimiter = ",";
	int *arr = new int[n];
	for (int i = 0; (pos = str.find(delimiter)) != string::npos; i++) {
		token = str.substr(0, pos);
		arr[i] = stoi(token);
		str.erase(0, pos + delimiter.length());
	}
	return arr;
}
int * Prefix::arrFirstHalf(int * arr, int n) {
	int *arr1, n1 = (int)floor((double)n / 2);
	arr1 = new int[n1];
	for (int i = 0; i < n1; i++) {
		arr1[i] = arr[i];
	}
	return arr1;
}
int * Prefix::arrSecondHalf(int * arr, int n) {
	int *arr2, n2 = (int)ceil((double)n / 2);
	arr2 = new int[n2];
	for (int i = 0; i < n2; i++) {
		arr2[i] = arr[i+n2-1];
	}
	return arr2;
}
int * Prefix::arrMerge(int * arr1, int * arr2, int n) {
	int n1 = (int)floor((double)n / 2), n2 = (int)ceil((double)n / 2);
	int *arr3 = new int[n1 + n2];
	for (int i = 0; i < n1; i++) {
		arr3[i] = arr1[i];
	}
	for (int i = n2 - 1; i < n1 + n2; i++) {
		arr3[i] = arr2[i - n2];
	}
	return arr3;
}
void Prefix::printArr() {
	cout << "\n*****\t*****\t*****\t*****\t   Start of Print    \t****\t*****\t*****\t*****" << endl;
	for (int i = 0; i < n; i++) {
		cout << "[" << i << "]" << ":" << A[i] << "\t";
		if (i % 10 == 9)
			cout << endl;
	}
	cout << "\n*****\t*****\t*****\t*****\t    End of Print     \t*****\t*****\t*****\t*****" << endl;
}
void Prefix::printArr( int *A, int n) {
	cout << "\n*****\t*****\t*****\t*****\t   Start of Print    \t****\t*****\t*****\t*****" << endl;
	for (int i = 0; i < n; i++) {
		cout << "[" << i << "]" << ":" << A[i] << "\t";
		if (i % 10 == 9)
			cout << endl;
	}
	cout << "\n*****\t*****\t*****\t*****\t    End of Print     \t*****\t*****\t*****\t*****" << endl;
}
void Prefix::printRes() {	
	cout << algName[algorithm] << " | Result | " << result << endl; 
}
void Prefix::test(int alg) {
	cout << "\nTESTING " << algName[alg] << " FOR N:" << n << endl;
	auto start = chrono::steady_clock::now();
	switch (alg) {
		case 1: prefixSeq(); break;
		case 2: prefixRec(); break;
		case 3: prefixPar(); break;
		//case 4: prefixDis(); break;
	}
	auto end = chrono::steady_clock::now();
	cout << algName[alg] << " | TEST " << (result == n ? "IS SUCCESFULL" : "FAILED!") << endl;
	cout << "Elapsed time: "
		<< chrono::duration_cast<chrono::nanoseconds>(end - start).count()
		<< " ns" << endl;
}
void Prefix::testAll(int range) {
	arrType = 0; debug = false; debugRes = false;
	bool test[4];
	cout << "\nTESTING N: 0 to " << range << endl;
	for (int j = 1; j < 4; j++) {
		for (int i = 0; i <= range; i++) {
			n = i;
			switch (j) {
				case 1: prefixSeq(); if (result != n) { test[0] = false; cout << "Seq | Result: " << result << " != n:" << n << endl; } break;
				case 2: prefixRec(); if (result != n) { test[1] = false; cout << "Rec | Result: " << result << " != n:" << n << endl; } break;
				case 3: prefixPar(); if (result != n) { test[2] = false; cout << "Par | Result: " << result << " != n:" << n << endl; } break;
			}
		}
		switch (j) {
			case 1: cout << "Seq | TEST " << (test[0] ? "IS SUCCESFULL" : "FAILED!") << endl; break;
			case 2: cout << "Rec | TEST " << (test[1] ? "IS SUCCESFULL" : "FAILED!") << endl; break;
			case 3: cout << "Par | TEST " << (test[2] ? "IS SUCCESFULL" : "FAILED!") << endl; break;
		}
	}
	cout << "FINISHED TESTING" << endl;
}

//GETTERS & SETTERS
int * Prefix::getArr() { return A; }
int Prefix::getVal(int i) {	return A[i]; }
void Prefix::setArr(int * arr) { A = arr; }
void Prefix::setRes() { result = A[0]; }

//Usefull Functions
void Prefix::menu(int argc, char* argv[]) {
	int n = 0;
	int thrCnt = 1;
	int algorithm = 0;
	int arrType = 0;
	bool debug = 0;
	bool debugRes = 0;
	char * addr = new char[32];
	bool test = 0;

	if ((string)argv[1] == "-h" || (string)argv[1] == "-help") {
		cout << "\nUsage: " << argv[0] << " <host_type> [options]\n";
		cout << "\n<host_type>" << "\nValues\t| Description\n";
		cout << "local\t| Running prefix algorithms locally." << endl;
		cout << "server\t| Running a server for distributed prefix algorithm." << endl;
		cout << "client\t| Joining to a server for distributed prefix algorithm." << endl;
		cout << "\nFor more information: <host_type> -h." << endl;
		return;
	}	
	else if ((string)argv[2] == "-h" || (string)argv[2] == "-help") {
		if ((string)argv[1] == "local") {
			cout << "\nLength of Array.";
			cout << "\nOption: -n [value]." << endl;

			cout << "\nNumber of Processing units.";
			cout << "\nOption: -p [value] (Default: POSSIBLE_MAX)." << endl;

			cout << "\nAlgorithm Type";
			cout << "\nOption: -algorithm / -a [value]" << "\nValues\t| Description\n";
			cout << "0\t| Sequential Odd/Even Prefix (Default)." << endl;
			cout << "1\t| Recursive Odd/Even Prefix." << endl;
			cout << "2\t| Parallel Odd/Even Prefix." << endl;

			cout << "\nDistribution Type: defining values of the array.";
			cout << "\nOption: -distribution / -dist [value1] [value2]" << "\nValues\t| Description\n";
			cout << "0\t| All values are 1." << endl;
			cout << "1\t| Starting from 0, values are incremented by 1." << endl;
			cout << "2\t| Randomly generated integers from [value1] to [value2]" << endl;

			cout << "\nDebug.";
			cout << "\nOption: -debug / -d\n" << endl;

			cout << "\nPrint Result.";
			cout << "\nOption: -res \n" << endl;
			return;
		}
		else if ((string)argv[1] == "server") {
			cout << "\nServer: Distributed and Parallel Odd/Even Prefix algorithm will be performed. ";
			cout << "\nIn order to proceed, client must be joined" << endl;
			cout << "\nUsage: " << argv[0] << " server -n [value] [options]\n";

			cout << "\nLength of Array.";
			cout << "\nOption: -n [value]." << endl;

			cout << "\nNumber of Processing units.";
			cout << "\nOption: -p [value] (Default: POSSIBLE_MAX)." << endl;

			cout << "\nDistribution Type: defining values of the array.";
			cout << "\nOption: -distribution / -dist [value1] [value2]" << "\nValues\t| Description\n";
			cout << "0\t| All values are 1." << endl;
			cout << "1\t| Starting from 0, values are incremented by 1." << endl;
			cout << "2\t| Randomly generated integers from [value1] to [value2]" << endl;

			cout << "\nDebug.";
			cout << "\nOption: -debug / -d\n" << endl;

			cout << "\nPrint Result.";
			cout << "\nOption: -res \n" << endl;
			return;
		}
		else if ((string)argv[1] == "client") {
			cout << "\nClient: Distributed and Parallel Odd/Even Prefix algorithm will be performed. ";
			cout << "\nIn order to proceed, must be joining to waiting server." << endl;
			cout << "\nUsage: " << argv[0] << " client [ip] [options]\n";

			cout << "\nNumber of Processing units.";
			cout << "\nOption: -p [value] (Default: POSSIBLE_MAX)." << endl;

			cout << "\nDebug.";
			cout << "\nOption: -debug / -d\n" << endl;

			cout << "\nPrint Result.";
			cout << "\nOption: -res \n" << endl;
			return;
		} else { cout << "Error: Input is not recognized/correct.\n-help for more information." << endl; return; }
	} 
	else if (argc < 3) {
		cout << "Error: Input is not recognized/correct.\n-help for more information." << endl;
		return;
	}
	if ((string)argv[1] == "local") {
		for (int i = 2; i < argc; i++) {
			if ((string)argv[i] == "-d" || (string)argv[i] == "-debug") debug = 1;
			else if ((string)argv[i] == "-res") debugRes = 1;
			else if ((string)argv[i] == "-test") test = 1;
			else if ((string)argv[i] == "-n") { n = atoi(argv[i + 1]) > 0 ? atoi(argv[i + 1]) : 0; i++; }
			else if ((string)argv[i] == "-p") { thrCnt = atoi(argv[i + 1]) >= -1 ? atoi(argv[i + 1]) : -1; i++; }
			else if ((string)argv[i] == "-distribution" || (string)argv[i] == "-dist") {
				arrType = (atoi(argv[i + 1]) < 3 && atoi(argv[i + 1]) > 0) ? atoi(argv[i + 1]) : 0; i++;
				if (arrType == 2) { ranRange[0] = atoi(argv[i + 1]); ranRange[1] = atoi(argv[i + 2]); i+=2; }
			}
			else if ((string)argv[i] == "-algorithm" || (string)argv[i] == "-a") { algorithm = (atoi(argv[i + 1]) < 3 && atoi(argv[i + 1]) > 0) ? atoi(argv[i + 1]) : 0; i++; }
			else { std::cout << "Not enough or invalid arguments, please try again.\n"; exit(0); }
		}
	}
	else if ( argc > 3 && (string)argv[1] == "server") {
		algorithm = 3;
		for (int i = 2; i < argc; i++) {
			if ((string)argv[i] == "-d" || (string)argv[i] == "-debug") debug = 1;
			else if ((string)argv[i] == "-res") debugRes = 1;
			else if ((string)argv[i] == "-n") { n = atoi(argv[i + 1]) > 0 ? atoi(argv[i + 1]) : 0; i++; }
			else if ((string)argv[i] == "-p") { thrCnt = atoi(argv[i + 1]) > 0 ? atoi(argv[i + 1]) : -1; i++; }
			else if ((string)argv[i] == "-distribution" || (string)argv[i] == "-dist") {
				arrType = (atoi(argv[i + 1]) < 3 && atoi(argv[i + 1]) > 0) ? atoi(argv[i + 1]) : 0; i++;
				if (arrType == 2) { ranRange[0] = atoi(argv[i + 1]); ranRange[1] = atoi(argv[i + 2]); i += 2; }
			}
			else {
				std::cout << "Not enough or invalid arguments, please try again.\n";
				exit(0);
			}
		}
	}
	else if ((string)argv[1] == "client") {
		algorithm = 3;
		addr = argv[2]; n = -1;
		for (int i = 3; i < argc; i++) {
			if ((string)argv[i] == "-d" || (string)argv[i] == "-debug") debug = 1;
			else if ((string)argv[i] == "-res") debugRes = 1;
			else if ((string)argv[i] == "-p") { thrCnt = atoi(argv[i + 1]) > 0 ? atoi(argv[i + 1]) : 0; i++; }
			else {
				std::cout << "Not enough or invalid arguments, please try again.\n";
				exit(0);
			}
		}
	}
	else { cout << "Error: Input is not recognized/correct.\n-help for more information." << endl; return; }
	if (n == 0) { std::cout << "Not enough or invalid arguments, please try again.\n"; exit(0); }

	Prefix::n = n;
	Prefix::thrCnt = (thrCnt <= 0) ? n : thrCnt;
	Prefix::algorithm = algorithm;
	Prefix::debug = debug;
	Prefix::debugRes = debugRes;
	Prefix::arrType = arrType;

	if ( n > 0 ) iniArr();

	if (test) {
		for (int i = 2; i < 17; i *= 2) {
			iniArr();
			float t1, tp, speedup, efficiency, charfunc;
			Prefix::thrCnt = thrCnt = i;
			//Prefix::thrCnt = thrCnt = (int)floor(n / 2);

			auto start = chrono::steady_clock::now();
			prefixPar();
			auto end = chrono::steady_clock::now();
			tp = (float)chrono::duration_cast<chrono::microseconds>(end - start).count();

			Prefix::thrCnt = thrCnt = 1;
			iniArr();
			start = chrono::steady_clock::now();
			prefixPar();
			end = chrono::steady_clock::now();

			Prefix::thrCnt = thrCnt = i;
			//Prefix::thrCnt = thrCnt = (int)floor(n / 2);
			t1 = (float)chrono::duration_cast<chrono::microseconds>(end - start).count();
			speedup = t1 / tp;
			efficiency = speedup / thrCnt;
			charfunc = ((tp - t1) / thrCnt) / (t1 * (1 - (1 / thrCnt)));
			cout << "\nP: " << thrCnt
				<< "\nT(1): " << t1 << "(microsec)"
				<< "\nT(P): " << tp << "(microsec)"
				<< "\nS(P): " << speedup << "\nE(P): " << efficiency << "\nf: " << charfunc << " (characteristics function) " << endl << endl;
		}
	}
	else {
		auto start = chrono::steady_clock::now();
		switch (algorithm) {
		case 0: { prefixSeq(); break; }
		case 1: { prefixRec(); break; }
		case 2: { prefixPar(); break; }
		case 3: {
			if ((string)argv[1] == "server") prefixDis();
			else if ((string)argv[1] == "client") prefixDis(argv[2]);
			break;
		}
		default: { break; }
		}
		auto end = chrono::steady_clock::now();
		cout << "Total time: "
			<< chrono::duration_cast<chrono::nanoseconds>(end - start).count()
			<< " ns" << endl;
		cout << "Total time: "
			<< chrono::duration_cast<chrono::milliseconds>(end - start).count()
			<< " ms" << endl;
	}
}

//Test
void Prefix::HelloWorld() {
	cout << "Hello World | test" << endl;
}