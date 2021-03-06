// CheckIn.cpp : Defines the entry point for the console application.
//

#ifdef TEST_CHECKIN
#include "CheckIn.h"
#include "../CppNoSqlDb/PayLoad/PayLoad.h"

using namespace Repository;
using namespace CheckInSpace;
using namespace NoSqlDb;



bool test1(CheckIn<PayLoad>& checkin)
{
	NoSqlDb::DbElement<PayLoad> elem1;
	NoSqlDb::DbElement<PayLoad> elem2;
	NoSqlDb::DbElement<PayLoad> elem3;
	NoSqlDb::DbElement<PayLoad> elem4;

	elem1.name("file1.h");
	elem1.descrip("file1.h");
	elem1.payLoad(PayLoad("open", "./test/TestFiles/file1.h", { "test", "A", "header" }, "A"));
	//elem1.children() = { "A::file1.cpp.1", "B::file3.h.1" };
	elem1.children() = { "B::file3.h.1" };

	//elem2.name("file1.cpp");
	//elem2.descrip("file1.cpp");
	//elem2.payLoad(PayLoad("open", "./test/TestFiles/file1.cpp", { "test", "A", "cpp" }, "A"));
	//elem2.children() = { "A::file1.h.1" };

	elem3.name("file3.h");
	elem3.descrip("file3.h");
	elem3.payLoad(PayLoad("open", "./test/TestFiles/file3.h", { "test", "B", "header" }, "B"));
	elem3.children() = { "C::file4.h.1" };

	elem4.name("file4.h");
	elem4.descrip("file4.h");
	elem4.payLoad(PayLoad("open", "./test/TestFiles/file4.h", { "test", "C", "header" }, "C"));
	elem4.children() = {};
	std::cout << "\nWe check in file3, it will fail because file4 is its child which should be checked in firstly." << std::endl;
	if (checkin.DoCheckIn("./test/TestFiles/file3.h", "file3.h", "B", elem3) == -2)
	{
		std::cout << "\tDependencies lost.\n" << std::endl;
	}
	
	std::cout << "\nWe check in file4 first then file3" << std::endl;
	checkin.DoCheckIn("./test/TestFiles/file4.h", "file4.h", "C", elem4);
	
	checkin.DoCheckIn("./test/TestFiles/file3.h", "file3.h", "B", elem3);
	std::cout << "Show db" << std::endl;
	NoSqlDb::showDb(checkin.repository().db());
	NoSqlDb::PayLoad::showDb(checkin.repository().db());
	//std::cout << "\nWe check in file1.h and file1.cpp" << std::endl;
	std::cout << "\nWe check in file1.h" << std::endl;
	checkin.DoCheckIn("./test/TestFiles/file1.h", "file1.h", "A", elem1);
	//checkin.DoCheckIn("./test/TestFiles/file1.cpp", "file1.cpp", "A", elem2);
	std::cout << "Show db" << std::endl;
	NoSqlDb::showDb(checkin.repository().db());
	NoSqlDb::PayLoad::showDb(checkin.repository().db());
	return true;
}

bool test2(CheckIn<PayLoad>& checkin)
{
	std::cout << "We modify file1.h" << std::endl;
	NoSqlDb::DbElement<PayLoad> elem;
	elem.name("file1_new.h");
	elem.descrip("new file1.h");
	elem.payLoad(PayLoad("open", "./test/TestFiles/file1.h", { "test", "A1", "header" }, "A"));
	elem.children() = { "B::file3.h.1" };
	checkin.DoCheckIn("./test/TestFiles/file1.h", "file1.h", "A", elem);
	NoSqlDb::showDb(checkin.repository().db());
	NoSqlDb::PayLoad::showDb(checkin.repository().db());
	return true;
}

bool test3(CheckIn<PayLoad>& checkin)
{
	std::cout << "close the file which has already been checked in" << std::endl;
	std::cout << "Close B::file3.h.1. Due to its children are not all be closed, its status will be closing. User should close it again when children are all closed." << std::endl;
	checkin.DoClose("B::file3.h.1", "B");
	NoSqlDb::PayLoad::showDb(checkin.repository().db());
	std::cout << "We close C::file4.h.1" << std::endl;
	checkin.DoClose("C::file4.h.1", "C");
	NoSqlDb::PayLoad::showDb(checkin.repository().db());
	//std::cout << "We close A::file1.cpp.1" << std::endl;
	//checkin.DoClose("A::file1.cpp.1", "A");
	//NoSqlDb::PayLoad::showDb(checkin.repository().db());
	std::cout << "Finally we close A::file1.h.1. All the file will be closed." << std::endl;
	checkin.DoClose("A::file1.h.1", "A");
	NoSqlDb::PayLoad::showDb(checkin.repository().db());
	return true;
}

bool test4(CheckIn<PayLoad>& checkin)
{
	std::cout << "Check in a closed file" << std::endl;
	NoSqlDb::DbElement<PayLoad> elem;
	elem.name("file1.h");
	elem.descrip("new file1.h");
	elem.payLoad(PayLoad("open", "./test/TestFiles/file1.h", { "test", "A2", "header" }, "A"));
	checkin.DoCheckIn("./test/TestFiles/file1.h", "file1.h", "A", elem);
	NoSqlDb::showDb(checkin.repository().db());
	NoSqlDb::PayLoad::showDb(checkin.repository().db());
	return true;
}

int main()
{
	Utilities::Title("Testing CheckIn Package");

	TestExecutive ex;
	DbCore<PayLoad> db;
	Repository<PayLoad> repository(db, "./test/TestRepository");
	CheckIn<PayLoad> checkin(repository);
	TestExecutive::TestStr ts1{ [&checkin]() { return test1(checkin); } , "Test1: Check in new files" };
	TestExecutive::TestStr ts2{ [&checkin]() { return test2(checkin); }, "Test2: modify an open check-in" };
	TestExecutive::TestStr ts3{ [&checkin]() { return test3(checkin); }, "Test3: close check-in" };
	TestExecutive::TestStr ts4{ [&checkin]() { return test4(checkin); }, "Test4: check in a closed file" };

	ex.registerTest(ts1);
	ex.registerTest(ts2);
	ex.registerTest(ts3);
	ex.registerTest(ts4);

	bool result = ex.doTests();
	if (result)
		std::cout << "All Test Passed" << std::endl;
	else
		std::cout << "Failed" << std::endl;
	return 0;
}
#endif

