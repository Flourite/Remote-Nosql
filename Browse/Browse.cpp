// Browse.cpp : Defines the entry point for the console application.
//





#ifdef TEST_BROWSE
#include "Browse.h"
#include "../CppNoSqlDb/PayLoad/PayLoad.h"
using namespace NoSqlDb;
using namespace Repository;
using namespace BrowseSpace;
void CreateDB(DbCore<PayLoad>& db)
{
	NoSqlDb::DbElement<PayLoad> elem;

	elem.name("file1.h");
	elem.descrip("file1.h");
	elem.payLoad(PayLoad("closed", "./test/TestRepository/A_file1.h.1", {"test", "A", "header"}, ""));
	elem.children() = { "A::file2.h.1", "B::file3.h.1" };
	db.addRecord("A::file1.h.1", elem);

	elem.name("file2.h");
	elem.descrip("file2.h");
	elem.payLoad(PayLoad("closed", "./test/TestRepository/A_file2.h.1", { "test", "A", "header" }, ""));
	elem.children() = { "A::file1.h.1" };
	db.addRecord("A::file2.h.1", elem);

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

bool test1(Server& repository)
{
	std::cout << "\nWe now use query to browse files" << std::endl;
	Browse<PayLoad> browse(repository);
	std::cout << "We Input reg 'file*' to match file name" << std::endl;
	Conditions<PayLoad> cond;
	cond.name("file*");
	browse.query().select(cond);
	
	browse.ShowResult();
	return true;
}

bool test2(Server& repository) 
{
	std::cout << "Display file content by query" << std::endl;
	Browse<PayLoad> browse(repository);
	std::cout << "We open files with category 'A' " << std::endl;
	browse.query().select([](auto elem) {return elem.payLoad().hasCategory("A");});
	std::cout << "We find these files, now display their names:" << std::endl;
	browse.ShowKeys();
	if (browse.query().keys().size() > 0)
	{
		std::cout << "Open the first file of the result, which is" + (browse.query().keys())[0] << std::endl;
		std::string key = (browse.query().keys())[0];
		std::string path = browse.GetPath(key);
		browse.OpenFile(key, path);
	}
	return true;
}

int main()
{
	Utilities::Title("Testing Browse Package");

	TestExecutive ex;
	DbCore<PayLoad> db;
	Repository<PayLoad> repository(db, "./test/TestRepository");
	CreateDB(db);
	TestExecutive::TestStr ts1{ [&repository]() { return test1(repository); } , "Test1: Query and Browse Files" };
	TestExecutive::TestStr ts2{ [&repository]() { return test2(repository); }, "Test2: Display specific file contents" };

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