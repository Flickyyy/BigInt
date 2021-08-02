#include <iostream>
#include "BigInt.h"

int main()
{
	BigInt value = 2;
	for (BigInt i = 1; i <= 2048; i*=2) {
		cout << "2 ^ " << i << " = " << value << endl;
		value *= value;
	}
}