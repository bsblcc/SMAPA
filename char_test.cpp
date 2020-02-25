#include <cstring>
#include <iostream>

using namespace std;


int main()
{
	char test[50];
	char b[50];
	cin >> test;

	
	strcpy(b, test);

	cout << strlen(b);


	return 0;
}