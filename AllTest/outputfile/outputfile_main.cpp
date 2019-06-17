#include <iostream>
#include <fstream>
#include <string>
using namespace std;
int main()
{
/*
	ofstream file("C:\\11\\log.txt",iostream::app);
	double * p3 = new double [3];
	p3[0] = 0.2;
	p3[1] = 0.5;
	p3[2] = 0.8;
	file << "p3[1] is " << p3[1] << ".\n";
	p3 = p3 + 1;
	file << "Now p3[0] is " << p3[0] << " and ";
	file << "p3[1] is " << p3[1] << ".\n";
	p3 = p3 - 1;
	delete [] p3;
	char sz[512] = {0};
	file << sz<<"\n";
	file.close();*/


	string a, b;
	while (cin >> a)
	{
		if (b == a)
			break;
		else
			b = a;
	}
	if (a == b && cin)
		cout << a << endl;
	else
		cout << "no" << endl;


	system("pause");
	return 0;
}
