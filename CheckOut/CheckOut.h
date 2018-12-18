#pragma once

///////////////////////////////////////////////////////////////////////
// Checkout.h - application defined checkput files                   //
// ver 2.0                                                           //
// Implemented by Congyou Liu                                        //
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
*  -------------------
*  CheckOutOne: To checkout one file in the repository
*  CheckOutWithChildren: To checkout one file with its children in the repository
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

#ifndef CHECKOUT_H
#define CHECKOUT_H

#include "../ServerPrototype/ServerPrototype.h"

namespace CheckOutSpace
{
	using namespace Repository;

	template <typename T>
	class CheckOut
	{
	public:
		CheckOut(Server& repository) :repository_(repository) {};
		bool CheckOutOne(std::string path, std::string key);
		int CheckOutWithChildren(std::string path, std::string key);
		static void identity() { std::cout << "\n  \"" << __FILE__ << "\""; };
		Server& repository() { return repository_; };
		Server repository() const { return repository_; };
	private:
		Server& repository_;
	};

	//----< checkout one file >----------------------
	template <typename T>
	bool CheckOut<T>::CheckOutOne(std::string path, std::string key)
	{
		std::string filepath = (repository_.db())[key].payLoad().value();
		std::string name = Repository::Server::KeyToPath(key);
		std::string path1 = path + "/" + VersionSpace::Version::RemoveVersionInfo(name);
		//std::cout << filepath << std::endl;
		//std::cout << name << std::endl;
		//std::cout << path1 << std::endl;
		if (repository_.CopyFileToRepo(filepath, path1))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	//----< checkout file with children >----------------------
	template <typename T>
	int CheckOut<T>::CheckOutWithChildren(std::string path, std::string key)
	{
		std::vector<std::string> children = repository_.GetChildren(key);
		int count = 0;
		for (auto child : children)
		{
			if (CheckOutOne(path, child))
				++count;
		}
		return count;
	}
}
#endif
