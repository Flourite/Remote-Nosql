#pragma once

///////////////////////////////////////////////////////////////////////
// Browse.h - application defined browsing file metadata             //
// ver 1.0                                                           //
// Updated by Congyou Liu                                            //
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
*  -------------------
*  This package provides a single class, Browse:
*  It provide output the file in the repository and its metadata, payload
*  ShowResult: Output the result of the browse query
*  ShowChildren: Show children of one file
*  ShowKeys: Show all the keys according to the query
*  OpenFile: open a file to see its content
*  GetPath: get the file path in the repository
*
*  Required Files:
*  ---------------
*    ServerPrototype.h, Process.h
*
*  Maintenance History:
*  --------------------
*  
*  Ver 1.0 : 10 Feb 2018
*  - first release
*
*/

#ifndef BROWSE_H
#define BROWSE_H

#include "../ServerPrototype/ServerPrototype.h"
#include "../Process/Process/Process.h"

namespace BrowseSpace
{
	using namespace Repository;
	using namespace NoSqlDb;

	template <typename T>
	class Browse
	{
	public:
		Browse(Server& repository) :repository_(repository), query_(Query<T>(repository_.db())) {};
		void ShowResult();
		void ShowChildren(std::vector<std::string>& children);
		void ShowKeys();
		bool OpenFile(const std::string key, const std::string path);
		static void identity() { std::cout << "\n  \"" << __FILE__ << "\""; };
		std::string GetPath(const std::string key);
		Server& repository() { return repository_; };
		Server repository() const { return repository_; };
		Query<T>& query() { return query_; };
		Query<T> query() const { return query_; };
	private:
		Server& repository_;
		Query<T> query_;
	};

	//----< show children of the key >----------------------
	template <typename T>
	void Browse<T>::ShowChildren(std::vector<std::string>& children)
	{
		for (auto child : children)
		{
			std::cout << "Key: " + child + "   " << std::endl;
			std::cout << "Name: " + (repository_.db())[child].name() << std::endl;
			std::cout << "Description: " + (repository_.db())[child].descrip() << std::endl;
		}
	}

	//----< show query result >----------------------
	template <typename T>
	void Browse<T>::ShowResult()
	{
		if (query_.keys().size() == 0)
		{
			std::cout << "No record found" << std::endl;
		}
		else
		{
			for (auto key : query_.keys())
			{
				
				if (repository_.contains(key))
				{
					std::cout << "Key: " + key + "   " << std::endl;
					std::cout << "Name: " + (repository_.db())[key].name() << std::endl;
					std::cout << "Description: " + (repository_.db())[key].descrip() << std::endl;
					if ((repository_.db())[key].children().size() > 0)
					{
						std::cout << "Children: " << std::endl;
						ShowChildren((repository_.db())[key].children());
					}
				}
				
			}
		}
	}

	//----< show keys of the query result >----------------------
	template <typename T>
	void Browse<T>::ShowKeys()
	{
		if (query_.keys().size() == 0)
		{
			std::cout << "Not found" << std::endl;
		}
		else
		{
			std::cout << "Keys are:" << std::endl;
			for (auto key : query_.keys())
			{
				std::cout << key << " ";
			}
		}
		std::cout << std::endl;
	}

	//----< get value of payload data >----------------------
	template <typename T>
	std::string Browse<T>::GetPath(const std::string key)
	{
		if (repository_.contains(key))
		{
			return (repository_.db())[key].payLoad().value();
		}
		else
		{
			return "";
		}
	}
	//----< open a file >----------------------
	template <typename T>
	bool Browse<T>::OpenFile(const std::string key, const std::string path)
	{
		Process process;
		process.title(key);
		std::string notepad = "c:/windows/system32/notepad.exe";
		process.application(notepad);
		std::string cmdLine = "/A " + path;
		process.commandLine(cmdLine);
#ifdef _DEBUG
		std::cout << "\n  starting process: \"" << notepad << "\"";
		std::cout << "\n  with this cmdlne: \"" << cmdLine << "\"";
#endif
		process.create();

#ifdef _DEBUG
		CBP callback = []() { std::cout << "\n  --- child process exited with this message ---"; };
		process.setCallBackProcessing(callback);
		process.registerCallback();

		std::cout << "\n  after OnExit";
		std::cout.flush();
		//char ch;
		//std::cin.read(&ch, 1);
#endif
		return true;
	}


}
#endif
