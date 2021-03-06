// Version.cpp : Defines the entry point for the console application.
//

#ifdef TEST_VERSION
#include "Version.h"
#include "../NoSqlDb/Utilities/StringUtilities/StringUtilities.h"
#include "../NoSqlDb/Utilities/TestUtilities/TestUtilities.h"
#include <iostream>


using namespace VersionSpace;
bool test1(Version& versions)
{
	std::cout << "We add two files to versions" << std::endl;
	versions.SetVersion("file1", 1);
	versions.SetVersion("file2", 1);
	
	std::cout << "Now we show you the version map:" << std::endl;
	for (auto iter = versions.begin(); iter != versions.end(); ++iter)
	{
		std::cout << iter->first << " has version " << iter->second << std::endl;
	}
	return true;
}

bool test2(Version& versions)
{
	std::cout << "We firstly update version of file1" << std::endl;
	versions.UpdateVersion("file1");
	versions.ShowVersion("file1");
	std::cout << "Now we remove file1's version" << std::endl;
	versions.RemoveFile("file1");
	versions.ShowVersion("file1");
	return true;
}
int main()
{
	Utilities::Title("Testing Version Package");

	Version versions;
	TestExecutive ex;
	TestExecutive::TestStr ts1{ [&versions]() {return test1(versions); }, "Test1: Create a version map\n" };
	TestExecutive::TestStr ts2{ [&versions]() {return test2(versions); }, "Test2: Operators in Version map\n" };

	ex.registerTest(ts1);
	ex.registerTest(ts2);

	bool result = ex.doTests();
	if (result)
		std::cout << "All Test Passed" << std::endl;
	else
		std::cout << "Failed" << std::endl;
	return 0;
}


#endif