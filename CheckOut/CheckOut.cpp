// CheckOut.cpp : Defines the entry point for the console application.
//


#include "CheckOut.h"
#include "../NoSqlDb/PayLoad/PayLoad.h"

#ifdef TEST_CHECKOUT

using namespace NoSqlDb;
using namespace RepositorySpace;
using namespace CheckOutSpace;

void CreateDB(DbCore<PayLoad>& db)
{
	NoSqlDb::DbElement<PayLoad> elem;

	elem.name("file1.h");
	elem.descrip("file1.h");
	elem.payLoad(PayLoad("closed", "./test/TestRepository/A_file1.h.1", { "test", "A", "header" }, ""));
	elem.children() = { "B::file3.h.1" };
	db.addRecord("A::file1.h.1", elem);

	//elem.name("file2.h");
	//elem.descrip("file2.h");
	//elem.payLoad(PayLoad("closed", "./test/TestRepository/A_file2.h.1", { "test", "A", "header" }, ""));
	//elem.children() = { "A::file1.h.1" };
	//db.addRecord("A::file2.h.1", elem);

	elem.name("file3.h");
	elem.descrip("file3.h");
	elem.payLoad(PayLoad("closed", "./test/TestRepository/B_file3.h.1", { "test", "B", "header" }, ""));
	elem.children() = { "C::file4.h.1" };
	db.addRecord("B::file3.h.1", elem);

	elem.name("file4.h");
	elem.descrip("file4.h");
	elem.payLoad(PayLoad("closed", "./test/TestRepository/C_file4.h.1", { "test", "C", "header" }, ""));
	elem.children() = {};
	db.addRecord("C::file4.h.1", elem);


}

bool test1(CheckOut<PayLoad>& checkout)
{
	std::cout << "We check out file4.h. You can find it in ./test/TestCHeckOut" << std::endl;
	if (checkout.CheckOutOne("./test/TestCheckOut", "C::file4.h.1"))
	{
		std::cout << "Check out successfully" << std::endl;
	}
	else
	{
		std::cout << "Failed" << std::endl;
	}
	return true;
}

bool test2(CheckOut<PayLoad>& checkout)
{
	std::cout << "We check out file1.h and all its children" << std::endl;
	if (checkout.CheckOutWithChildren("./test/TestCheckOut", "A::file1.h.1") == checkout.repository().GetChildren("A::file1.h.1").size())
	{
		std::cout << "Check out successfully" << std::endl;
	}
	else
	{
		std::cout << "Failed" << std::endl;
	}
	return true;
}

int main()
{
	Utilities::Title("Testing CheckOut Package");

	TestExecutive ex;
	DbCore<PayLoad> db;
	CreateDB(db);
	NoSqlDb::showDb(db);
	PayLoad::showDb(db);
	Repository<PayLoad> repository(db, "./test/TestRepository");
	CheckOut<PayLoad> checkout(repository);
	TestExecutive::TestStr ts1{ [&checkout]() { return test1(checkout); } , "Test1: Check out one single file" };
	TestExecutive::TestStr ts2{ [&checkout]() { return test2(checkout); }, "Test2: Check out file and its children" };

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



