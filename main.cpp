#include "Prefix.h"

//Basic Usage : <filename> <host_type> <array_size> <no_of_threads>
int main(int argc, char* argv[]) {

	//auto start = chrono::steady_clock::now();
	//this_thread::sleep_for(2s);
	//auto end = chrono::steady_clock::now();
	//auto t = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
	//int t1 = (int)t;
	//cout << "\nElapsed time: " << t1 - 10000000 << " ns" << endl;
	//cout << "Elapsed time: "
	//	<< chrono::duration_cast<chrono::milliseconds>(end - start).count()
	//	<< " ms" << endl;


	Prefix::Prefix().menu(argc, argv);

	//int n = (int)atoi(argv[1]);
	//int thrCnt = (int)atoi(argv[2]);

	//Prefix p = Prefix::Prefix(n);
	//Prefix p = Prefix::Prefix(n, thrCnt, 0, 1, 1);

	//if ((string)argv[1] == "client") {
	//	p.prefixDis(argv[2]);
	//}
	//else if ((string)argv[1] == "server") {
	//	p.prefixDis();
	//}
	//else if ((string)argv[1] == "local") {

	//	p.menu(argc, argv);
	//	p.prefixSeq();
	//	p.printArr();
	//	p.prefixRec();
	//	p.printArr();
	//p.iniArr();
	//p.prefixPar();
	//p.printArr();

	//	//p.testAll(n);

	//	p.test(1); //Seq
	//	p.test(2); //Rec
	//	p.test(3); //Par
	//}

	return 0;
}