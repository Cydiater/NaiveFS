#include <iostream>
#include <fstream>
#include <string>
#include <random>

int main(int argc, char ** argv)
{
	std::mt19937 rd(1);
	std::fstream test, ans;
	test.open("../build/disk/1", std::ios::out);
	ans.open("ans", std::ios::out);
	std::string str;
	for (long long i = std::stoll(argv[1]); i > 0; i--)
		str += ('0' + rd() % 10);
	test << str;
	ans << str;
	test.close(), ans.close();
	return 0;
}
