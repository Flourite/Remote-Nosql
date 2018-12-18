#pragma once

///////////////////////////////////////////////////////////////////////
// Version.h - application defined version control and record        //
// ver 1.0                                                           //
// Implemented by Congyou Liu                                        //
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
*  -------------------
*  SetVersion: set a version
*  GetLatestVersion: Get the latest version of a file in the repository
*  RemoveFile: delete a file
*  ShowVersion: Output the version of a file
*  UpdateVersion: Update the file's version of there sre some new check-in
*  RemoveVersionInfo:Remove the version information
*

*  Maintenance History:
*  --------------------
*  Ver 1.0 : 30 Apr 2018
*  - first release
*
*/
#ifndef VERSION_H
#define VERSION_H
#include <unordered_map>
#include <iostream>
#include <string>

namespace VersionSpace
{
	class Version
	{
	public:
		std::unordered_map<std::string, int>& versions() { return versions_; };
		std::unordered_map<std::string, int> versions() const { return versions_; };
		void SetVersion(std::string FileName, int version);
		int GetLatestVersion(const std::string FileName);
		void RemoveFile(std::string FileName);
		void ShowVersion(std::string FileName);
		static void identity() { std::cout << "\n  \"" << __FILE__ << "\""; };
		std::string UpdateVersion(std::string FileName);
		std::string NewVersion(std::string FileName);
		static std::string RemoveVersionInfo(std::string FileName);
		std::unordered_map<std::string, int>::iterator begin() { return versions_.begin(); };
		std::unordered_map<std::string, int>::iterator end() { return versions_.end(); };
	private:
		std::unordered_map<std::string, int> versions_;
	};

	void Version::SetVersion(std::string FileName, int version)
	{
		versions_[FileName] = version;
	}

	void Version::RemoveFile(std::string FileName)
	{
		versions_.erase(FileName);
	}

	int Version::GetLatestVersion(const std::string FileName)
	{
		auto target = versions_.find(FileName);
		if (target != versions_.end())
			return target->second;
		else
			return -1;
	}

	void Version::ShowVersion(std::string FileName)
	{
		int version = GetLatestVersion(FileName);
		if (version != -1)
		{
			std::cout << "The latest version of ";
			std::cout << FileName << " is " << version << std::endl;
		}
		else
			std::cout << "File not found!" << std::endl;
	}

	std::string Version::NewVersion(std::string FileName)
	{
		auto target = versions_.find(FileName);
		if (target != versions_.end())
		{
			return FileName + "." + std::to_string(target->second + 1);
		}
		else
		{
			return FileName + ".1";
		}
	}

	std::string Version::UpdateVersion(std::string FileName)
	{
		auto target = versions_.find(FileName);
		if (target != versions_.end())
		{
			int a = target->second + 1;
			
			SetVersion(FileName, a);
			
			return FileName + "." + std::to_string(a);
		}
		else
		{
			SetVersion(FileName, 1);
			return FileName + ".1";
		}
	}

	std::string Version::RemoveVersionInfo(std::string FileName)
	{
		auto target = FileName.rbegin();
		while ((*target != '.') && (target != FileName.rend()))
			target++;
		if (target != FileName.rend())
			FileName.erase(target.base() - 1, FileName.end());
		return FileName;
	}
}

#endif

