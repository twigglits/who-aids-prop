#include "discretefunction.h"
#include "gslrandomnumbergenerator.h"
#include "util.h"
#include <iostream>

using namespace std;

int main0(void)
{
	vector<double> data;
	
	data.push_back(0.1);
	data.push_back(0.2);
	data.push_back(0.4);
	data.push_back(0.8);
	data.push_back(2.2);

	DiscreteFunction f(0, 1, data);
	int n = 100;

	for (int i = 0 ; i < n ; i++)
	{
		double x = f.getMinX() + (f.getMaxX()-f.getMinX())/(double)n * (double)i;

		cout << x << " " << f(x) << endl;
	}
	cout << endl;
	cout << endl;
	cout << endl;

	vector<pair<double,double> > data2;
	
	data2.push_back(pair<double,double>(0,0.1));
	data2.push_back(pair<double,double>(0.7,0.8));
	data2.push_back(pair<double,double>(1.0,2.2));

	DiscreteFunction f2(data2);
	n = 100;

	for (int i = 0 ; i < n ; i++)
	{
		double x = f2.getMinX() + (f2.getMaxX()-f2.getMinX())/(double)n * (double)i;

		cout << x << " " << f2(x) << endl;
	}
	cout << endl;
	cout << endl;
	cout << endl;
	return 0;
}

int main1(void)
{
	GslRandomNumberGenerator r;
	double zeta = 0;
	double sigma = 0.25;
	int n = 1000000;

	for (int i = 0 ; i < n ; i++)
		cout << r.pickLogNorm(zeta, sigma) << endl;

	return 0;
}

int main(void)
{
	string s = " \t abc  \n\r  \t";
	string t = trim(s);

	cout << t << endl;
	cout << t.length() << endl;

	string u = "";
	double x;
	if (parseAsDouble(u, x))
		cout << x << endl;
	else
		cout << "Can't parse as double" << endl;

	cout << replace("%%%bla amaiamai x%%%", "%", "BLA") << endl;
	return 0;
}
