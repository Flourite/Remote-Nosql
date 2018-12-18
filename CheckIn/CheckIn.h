#pragma once

///////////////////////////////////////////////////////////////////////
// Checkin.h - application defined checkin files                     //
// ver 2.0                                                           //
// Implemented by Congyou Liu                                        //
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
*  -------------------
*  DoCheckIn: Interface for the client to run check-in process
*  DoCheckInOpen: Check in open file
*  DoCheckInClose: check-in closed file
*  DoCheckInNew: check in new file
*  OwnerCheck: check the owner of the file
*
*  Required Files:
*  ---------------
*    ServerPrototype.h
*
*  Maintenance History:
*  --------------------
*  Ver 2.0 : 30 Apr 2018
*  - change repository type to 'Server'
*  Ver 1.0 : 10 Feb 2018
*  - first release
*
*/

#ifndef CHECKIN_H
#define CHECKIN_H

#include "../ServerPrototype/ServerPrototype.h"
namespace CheckInSpace
{
	using namespace Repository;
	using namespace VersionSpace;
	using namespace NoSqlDb;

	template <typename T>
	class CheckIn
	{
	public:
		CheckIn(Server& repository) :repository_(repository) {};
		enum STATUSCODE { SUCCESS = 1,FAILED = 0,OWNER = -1,CHILDREN = -2,NOKEY = -3,NOFILE = -4};

		int DoCheckIn(std::string path, std::string name, std::string nameSpace, DbElement<T> elem);
		int DoClose(std::string key, std::string owner);
		bool OwnerCheck(std::string key, std::string owner);
		static void identity() { std::cout << "\n  \"" << __FILE__ << "\""; };
		Server& repository() { return repository_; };
		Server repository() const { return repository_; };
		void repository(Server& repository) { repository_ = repository; };
	protected:
		int DoCheckInOpen(std::string path, std::string name, std::string nameSpace, DbElement<T>& elem);
		int DoCheckClose(std::string path, std::string name, std::string nameSpace, DbElement<T>& elem);
		int DoCheckNew(std::string path, std::string name, std::string nameSpace, DbElement<T>& elem);
	private:
		Server& repository_;
	};

	//----< check if owner match >----------------------
	template <typename T>
	bool CheckIn<T>::OwnerCheck(std::string key, std::string owner)
	{
		return repository_.db().contains(key) && ((repository_.db())[key].payLoad().owner() == owner);
	}

	//----< close checkin >----------------------
	template <typename T>
	int CheckIn<T>::DoClose(std::string key, std::string owner)
	{
		int flag = NOKEY;
		if (repository_.db().contains(key))
		{
			if (!OwnerCheck(key, owner))
			{
				return OWNER;
			}
			(repository_.db())[key].payLoad().closing();
			int flag1 = SUCCESS;
			std::vector<std::string> children = repository_.GetChildren(key);
			for (auto child : children)
			{
				if (key != child && !(repository_.db())[child].payLoad().checkClose())
				{
					flag1 = CHILDREN;
					break;
				}
			}
			flag = flag1;
			if (flag1 > 0)
			{
				for (auto child : children)
				{
					(repository_.db())[child].payLoad().closed();
				}
			}
		}
		return flag;
	}

	//----< main check in process>----------------------
	template <typename T>
	int CheckIn<T>::DoCheckIn(std::string path, std::string name, std::string nameSpace, DbElement<T> elem)
	{
		std::string key = nameSpace + "::" + name;
		int ver = repository_.versions().GetLatestVersion(key);
		elem.payLoad().open();
		
		if (ver != -1)
		{
			
			std::string filekey = key + "." + std::to_string(ver);
			
			DbElement<T> elem1 = (repository_.db())[filekey];
			
			if (elem1.payLoad().checkClose())
			{
				
				return DoCheckClose(path, name, nameSpace, elem);
			}
			else
			{
				
				return DoCheckInOpen(path, name, nameSpace, elem);
			}
		}
		else
		{
			
			return DoCheckNew(path, name, nameSpace, elem);
		}
		return FAILED;
	}

	//----< Checkin the closed file >----------------------
	template <typename T>
	int CheckIn<T>::DoCheckClose(std::string path, std::string name, std::string nameSpace, DbElement<T>& elem)
	{
		std::string key1 = nameSpace + "::" + name;
		std::string oldkey = key1 + "." + std::to_string(repository_.versions().GetLatestVersion(key1));
		std::string keynew = repository_.versions().UpdateVersion(key1);
		std::cout << keynew << std::endl;
		if (!repository_.contains(oldkey))
		{
			return NOKEY;
		}
		else
		{
			if (!OwnerCheck(oldkey, elem.payLoad().owner()))
			{
				return OWNER;
			}
			elem.payLoad().value() = repository_.root() + "/" + Repository::Server::KeyToPath(keynew);
			for (auto child : elem.children()) {
				if (!repository_.contains(child)) return CHILDREN;
			}

			if (repository_.CopyFileToRepo(path, elem.payLoad().value())) 
			{
				if (repository_.db().addRecord(keynew, elem)) 
				{
					repository_.versions().UpdateVersion(key1);
					return SUCCESS;
				}
			}
			else {
				return NOFILE;
			}
		}
		return FAILED;
	}

	//----< Checkin open file >----------------------
	template <typename T>
	int CheckIn<T>::DoCheckInOpen(std::string path, std::string name, std::string nameSpace, DbElement<T>& elem)
	{
		std::string key1 = nameSpace + "::" + name;
		std::string key = key1 + "." + std::to_string(repository_.versions().GetLatestVersion(key1));

		if (!repository_.contains(key))
		{
			return NOKEY;
		}
		else
		{
			
			if (!OwnerCheck(key, elem.payLoad().owner()))
			{
				
				return OWNER;
			}
			elem.payLoad().value() = (repository_.db())[key].payLoad().value();
			
			for (auto child : elem.children()) 
			{
				if (!repository_.contains(child)) return CHILDREN;
			}
			if (repository_.CopyFileToRepo(path, elem.payLoad().value()))
			{
				
				(repository_.db())[key] = elem;
				return SUCCESS;
			}
			else 
			{
				return NOFILE;
			}
		}
		return FAILED;
	}

	//----< check in new file >----------------------
	template <typename T>
	int CheckIn<T>::DoCheckNew(std::string path, std::string name, std::string nameSpace, DbElement<T>& elem)
	{
		std::string key1 = nameSpace + "::" + name;
		std::string key = repository_.versions().NewVersion(key1);
		if (repository_.contains(key))
		{
			return FAILED;
		}
		else
		{
			elem.payLoad().value() = repository_.root() + "/" + Repository::Server::KeyToPath(key);
			
			for (auto child : elem.children()) 
			{
				if (!repository_.contains(child)) return CHILDREN;
			}
			if (repository_.CopyFileToRepo(path, elem.payLoad().value()))
			{
				if (repository_.db().addRecord(key, elem))
				{
					repository_.versions().UpdateVersion(key1);
					return SUCCESS;
				}
			}
			else
			{
				return NOFILE;
			}
		}
		return FAILED;
	}
	
}
#endif

