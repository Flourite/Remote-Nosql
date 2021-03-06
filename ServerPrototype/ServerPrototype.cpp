/////////////////////////////////////////////////////////////////////////
// ServerPrototype.cpp - Console App that processes incoming messages  //
// ver 2.0                                                             //
// Updated by Congyou Liu                                              //
/////////////////////////////////////////////////////////////////////////

#include "ServerPrototype.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../Process/Process/Process.h"
#include "../CheckIn/CheckIn.h"
#include "../CheckOut/CheckOut.h"
#include "../Browse/Browse.h"
#include "../CppNoSqlDb/PayLoad/PayLoad.h"
#include <chrono>

namespace MsgPassComm = MsgPassingCommunication;

using namespace Repository;
using namespace FileSystem;
using namespace CheckInSpace;
using namespace CheckOutSpace;
using namespace BrowseSpace;
using namespace NoSqlDb;
using Msg = MsgPassingCommunication::Message;

//----< return name of every file on path >----------------------------

Files Server::getFiles(const Repository::SearchPath& path)
{
  return Directory::getFiles(path);
}
//----< return name of every subdirectory on path >--------------------

Dirs Server::getDirs(const Repository::SearchPath& path)
{
  return Directory::getDirectories(path);
}

namespace MsgPassingCommunication
{
  // These paths, global to MsgPassingCommunication, are needed by 
  // several of the ServerProcs, below.
  // - should make them const and make copies for ServerProc usage

  std::string sendFilePath;
  std::string saveFilePath;

  //----< show message contents >--------------------------------------

  template<typename T>
  void show(const T& t, const std::string& msg)
  {
    std::cout << "\n  " << msg.c_str();
    for (auto item : t)
    {
      std::cout << "\n    " << item.c_str();
    }
  }
  //----< test ServerProc simply echos message back to sender >--------

  std::function<Msg(Msg)> echo = [](Msg msg) {
    Msg reply = msg;
    reply.to(msg.from());
    reply.from(msg.to());
    return reply;
  };
  //----< getFiles ServerProc returns list of files on path >----------

  
  //----< getDirs ServerProc returns list of directories on path >-----

  std::function<Msg(Msg)> getDirs = [](Msg msg) {
    Msg reply;
    reply.to(msg.from());
    reply.from(msg.to());
    reply.command("getDirs");
    std::string path = msg.value("path");
    if (path != "")
    {
      std::string searchPath = storageRoot;
      if (path != ".")
        searchPath = searchPath + "\\" + path;
      Files dirs = Server::getDirs(searchPath);
      size_t count = 0;
      for (auto item : dirs)
      {
        if (item != ".." && item != ".")
        {
          std::string countStr = Utilities::Converter<size_t>::toString(++count);
          reply.attribute("dir" + countStr, item);
        }
      }
    }
    else
    {
      std::cout << "\n  getDirs message did not define a path attribute";
    }
    return reply;
  };

  
	

  //----< analyze code on current server path >--------------------------
  /*
  *  - Creates process to run CodeAnalyzer on specified path
  *  - Won't return until analysis is done and logfile.txt
  *    is copied to sendFiles directory
  */
  std::function<Msg(Msg)> codeAnalyze = [](Msg msg) {
    Msg reply;
    reply.to(msg.from());
    reply.from(msg.to());
    reply.command("sendFile");
    reply.attribute("sendingFile", "logfile.txt");
    reply.attribute("fileName", "logfile.txt");
    reply.attribute("verbose", "blah blah");
    std::string path = msg.value("path");
    if (path != "")
    {
      std::string searchPath = storageRoot;
      if (path != "." && path != searchPath)
        searchPath = searchPath + "\\" + path;
      if (!FileSystem::Directory::exists(searchPath))
      {
        std::cout << "\n  file source path does not exist";
        return reply;
      }
      // run Analyzer using Process class

      Process p;
      p.title("test application");
      //std::string appPath = "c:/su/temp/project4sample/debug/CodeAnalyzer.exe";
      std::string appPath = "CodeAnalyzer.exe";
      p.application(appPath);

      //std::string cmdLine = "c:/su/temp/project4Sample/debug/CodeAnalyzer.exe ";
      std::string cmdLine = "CodeAnalyzer.exe ";
      cmdLine += searchPath + " ";
      cmdLine += "*.h *.cpp /m /r /f";
      //std::string cmdLine = "c:/su/temp/project4sample/debug/CodeAnalyzer.exe ../Storage/path *.h *.cpp /m /r /f";
      p.commandLine(cmdLine);

      std::cout << "\n  starting process: \"" << appPath << "\"";
      std::cout << "\n  with this cmdlne: \"" << cmdLine << "\"";

      CBP callback = []() { std::cout << "\n  --- child process exited ---"; };
      p.setCallBackProcessing(callback);

      if (!p.create())
      {
        std::cout << "\n  can't start process";
      }
      p.registerCallback();

      std::string filePath = searchPath + "\\" + /*msg.value("codeAnalysis")*/ "logfile.txt";
      std::string fullSrcPath = FileSystem::Path::getFullFileSpec(filePath);
      std::string fullDstPath = sendFilePath;
      if (!FileSystem::Directory::exists(fullDstPath))
      {
        std::cout << "\n  file destination path does not exist";
        return reply;
      }
      fullDstPath += std::string("\\") + /*msg.value("codeAnalysis")*/ "logfile.txt";
      FileSystem::File::copy(fullSrcPath, fullDstPath);
    }
    else
    {
      std::cout << "\n  getDirs message did not define a path attribute";
    }
    return reply;
  };
}

//----< Getfile proc >----------------
ServerProc Server::GetFile()
{
	return [this](Msg msg) {
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("getFiles");
		std::string path = msg.value("path");
		if (path != "")
		{
			std::string searchPath = storageRoot;
			if (path != ".")
				searchPath = searchPath + "\\" + path;
			Files files = Server::getFiles(searchPath);
			size_t count = 0;
			for (auto item : files)
			{
				std::string countStr = Utilities::Converter<size_t>::toString(++count);
				reply.attribute("file" + countStr, item);
				
			}
		}
		else
		{
			std::cout << "\n  getFiles message did not define a path attribute";
		}
		return reply;
	};
}

//----< sendFile ServerProc sends file to requester >----------------
ServerProc Server::SendFile()
{
	return [this](Msg msg) {
		std::string sendFilePath;
		std::string saveFilePath;
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("sendFile");
		reply.attribute("sendingFile", msg.value("fileName"));
		reply.attribute("fileName", msg.value("fileName"));
		reply.attribute("verbose", "blah blah");
		std::string path = msg.value("path");
		if (path != "")
		{
			std::string searchPath = storageRoot;
			if (path != "." && path != searchPath)
				searchPath = searchPath + "\\" + path;
			if (!FileSystem::Directory::exists(searchPath))
			{
				std::cout << "\n  file source path does not exist";
				return reply;
			}
			std::string filePath = searchPath + "/" + msg.value("fileName");
			std::string fullSrcPath = FileSystem::Path::getFullFileSpec(filePath);
			std::string fullDstPath = sendFilePath;
			if (!FileSystem::Directory::exists(fullDstPath))
			{
				std::cout << "\n  file destination path does not exist";
				return reply;
			}
			fullDstPath += "/" + msg.value("fileName");
			FileSystem::File::copy(fullSrcPath, fullDstPath);
			
			
		}
		else
		{
			std::cout << "\n  getDirs message did not define a path attribute";
		}
		return reply;
	};

}
//----< send checkin status message to client >----------------
ServerProc Server::CheckInFile()
{
	return [this](Msg msg)
	{
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("checkin");
		CheckIn<PayLoad> checkin(*this);
		DbElement<PayLoad> elem;
		elem.name(msg.value("fileName"));
		elem.descrip(msg.value("Description"));
		if (msg.value("Children") != "")
		{
			std::vector<std::string> s;
			s.push_back(msg.value("Children"));
			elem.children() = s;
		}
		
		PayLoad p;
		std::vector<std::string> s1;
		s1.push_back(msg.value("Category"));
		p.categories() = s1;
		p.owner() = msg.value("Category");
		p.status() = "open";
		p.value() = "../LocalStorage/" + elem.name();
		elem.payLoad(p);
		int a = checkin.DoCheckIn("../LocalStorage/" + elem.name(), elem.name(), msg.value("Category"), elem);
		std::string sta;
		NoSqlDb::PayLoad::showDb(db_);
		
		if (a == 0) sta = "CHECKIN FAILED";
		if (a == -1) sta = "ERROR: OWNER";
		if (a == -2) sta = "ERROR: CHILDREN";
		if (a == -3) sta = "ERROR: NOKEY";
		if (a == -4) sta = "ERROR: NOFILE";
		if (a == 1)
		{
			sta = "SUCCESS";
			//int ver = checkin.repository().versions().GetLatestVersion(msg.value("fileName"));
			
			reply.attribute("fileName", msg.value("fileName"));
			
		}
		reply.attribute("status", sta);
		return reply;
	};
}

//----< send status of closing checkin to requester >----------------
ServerProc Server::CloseCheckIn()
{
	return [this](Msg msg)
	{
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("closecheckin");
		CheckIn<PayLoad> checkin(*this);
		std::string s = msg.value("fileName");
		std::string owner = "";
		int i = 0;
		while (s[i] != '_')
		{
			owner = owner + s[i];
			++i;
		}
		std::string name = "";
		for (int j = i + 1; j < (int)s.length(); j++)
		{
			name = name + s[j];
		}
		int a = checkin.DoClose(owner + "::" + name, owner);
		std::string sta;
		NoSqlDb::PayLoad::showDb(db_);
		if (a == 1) sta = "SUCCESS";
		if (a == 0) sta = "CHECKIN FAILED";
		if (a == -1) sta = "ERROR: OWNER";
		if (a == -2) sta = "ERROR: CHILDREN";
		if (a == -3) sta = "ERROR: NOKEY";
		if (a == -4) sta = "ERROR: NOFILE";
		reply.attribute("status", sta);
		return reply;
	};
}
//----< sends checkout file to requester >----------------
ServerProc Server::CheckOutFile(){
	return [this](Msg msg){
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		std::string w = msg.value("withchildren");
		std::string s = msg.value("fileName");
		CheckOut<PayLoad> checkout(*this);
		std::string owner = "";
		int i = 0;
		while (s[i] != '_'){owner = owner + s[i];++i;}
		std::string name = "";
		for (int j = i + 1; j < (int)s.length(); j++){name = name + s[j];}
		bool a = false;
		int count = 0;
		if (w == "Y"){a = checkout.CheckOutOne("../CheckedOutFiles", owner + "::" + name);}
		else{count = checkout.CheckOutWithChildren("../CheckedOutFiles", owner + "::" + name);}
		std::string sta;
		if ((a == false)&&(count == 0)){sta = "FAILED";}
		else{
			sta = "SUCCESS";
			reply.command("sendFile");
			reply.attribute("sendingFile", msg.value("fileName"));
			reply.attribute("fileName", msg.value("fileName"));
			reply.attribute("verbose", "You can see the blocks");
			std::string path = msg.value("path");
			if (path != ""){
				std::string searchPath = storageRoot;
				if (path != "." && path != searchPath)
					searchPath = searchPath + "\\" + path;
				if (!FileSystem::Directory::exists(searchPath)){
					std::cout << "\n  file source path does not exist";
					return reply;
				}
				std::string filePath = searchPath + "/" + msg.value("fileName");
				std::string fullSrcPath = FileSystem::Path::getFullFileSpec(filePath);
				std::string fullDstPath = "./SendFiles";
				if (!FileSystem::Directory::exists(fullDstPath)){
					std::cout << "\n  file destination path does not exist";
					return reply;
				}
				fullDstPath += "/" + msg.value("fileName");
				FileSystem::File::copy(fullSrcPath, fullDstPath);
			}
			else{std::cout << "\n  getDirs message did not define a path attribute";}
		}
		reply.attribute("status", sta);
		return reply;
	};
}

//----< sends query result to requester >----------------
ServerProc Server::QueryRequest()
{
	return [this](Msg msg){
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		reply.command("queryrequest");
		Browse<PayLoad> browse(*this);
		std::string hasname = msg.value("hasName");
		std::string cat = msg.value("hasCategory");
		Conditions<PayLoad> cond1;
		if (hasname != ""){
			cond1.name(hasname);
			browse.query().select(cond1);
		}
		if (cat != ""){
			browse.query().select([=](auto elem) {return elem.payLoad().hasCategory(cat); });
		}
		std::string match = "";
		for (auto key : browse.query().keys()){
			if (browse.repository().contains(key)){
				std::string s = key;
				std::string owner = "";
				int i = 0;
				while (s[i] != ':'){
					owner = owner + s[i];
					++i;
				}
				std::string name = "";
				for (int j = i + 2; j < (int)s.length(); j++){
					name = name + s[j];
				}
				match = match + owner + "_" + name + "+";
			}
		}
		reply.attribute("hasname", match);
		return reply;
	};
}
//----< sends file and its metadata informationto requester >----------------
ServerProc Server::Display(){
	return [this](Msg msg){
		Msg reply;
		reply.to(msg.from());
		reply.from(msg.to());
		std::string s = msg.value("fileName");
		std::string owner = "";
		int i = 0;
		while (s[i] != '_'){
			owner = owner + s[i];
			++i;
		}
		std::string name = "";
		for (int j = i + 1; j < (int)s.length(); j++){name = name + s[j];}
		int j = s.length() - 1;
		std::string ver = "";
		while (s[j] != '.'){ver = s[j] + ver; j--;}
		std::string file = owner + "::" + name;
		reply.command("sendFile");
		reply.attribute("sendingFile", msg.value("fileName"));
		reply.attribute("fileName", msg.value("fileName"));
		reply.attribute("verbose", "You can see the blocks");
		std::string path = msg.value("path");
		if (path != ""){
			std::string searchPath = storageRoot;
			if (path != "." && path != searchPath)
				searchPath = searchPath + "\\" + path;
			if (!FileSystem::Directory::exists(searchPath)){
				std::cout << "\n  file source path does not exist";
				return reply;
			}
			std::string filePath = searchPath + "/" + msg.value("fileName");
			std::string fullSrcPath = FileSystem::Path::getFullFileSpec(filePath);
			std::string fullDstPath = "./SendFiles";
			if (!FileSystem::Directory::exists(fullDstPath)){
				std::cout << "\n  file destination path does not exist";
				return reply;
			}
			fullDstPath += "/" + msg.value("fileName");
			FileSystem::File::copy(fullSrcPath, fullDstPath);
		}
		Browse<PayLoad> browse(*this);
		reply.attribute("name", (browse.repository().db())[file].name());
		reply.attribute("category", (browse.repository().db())[file].payLoad().categories()[0]);
		reply.attribute("children", (browse.repository().db())[file].children()[0]);
		reply.attribute("version", ver);
		return reply;
	};
}
using namespace MsgPassingCommunication;

int main()
{
  SetConsoleTitleA("Project4Sample Server Console");

  std::cout << "\n  Testing Server Prototype";
  std::cout << "\n ==========================";
  std::cout << "\n";

  //StaticLogger<1>::attach(&std::cout);
  //StaticLogger<1>::start();

  sendFilePath = FileSystem::Directory::createOnPath("./SendFiles");
  saveFilePath = FileSystem::Directory::createOnPath("./SaveFiles");

  Server server(serverEndPoint, "ServerPrototype");

  // may decide to remove Context
  MsgPassingCommunication::Context* pCtx = server.getContext();
  pCtx->saveFilePath = saveFilePath;
  pCtx->sendFilePath = sendFilePath;

  server.start();

  std::cout << "\n  testing getFiles and getDirs methods";
  std::cout << "\n --------------------------------------";
  Files files = server.getFiles();
  show(files, "Files:");
  Dirs dirs = server.getDirs();
  show(dirs, "Dirs:");
  std::cout << "\n";

  std::cout << "\n  testing message processing";
  std::cout << "\n ----------------------------";
  server.addMsgProc("echo", echo);
  server.addMsgProc("getFiles", server.GetFile());
  server.addMsgProc("getDirs", getDirs);
  server.addMsgProc("sendFile", server.SendFile());
  server.addMsgProc("codeAnalyze", codeAnalyze);
  server.addMsgProc("serverQuit", echo);
	server.addMsgProc("checkin", server.CheckInFile());
	server.addMsgProc("closecheckin", server.CloseCheckIn());
	server.addMsgProc("checkout", server.CheckOutFile());
	server.addMsgProc("queryrequest", server.QueryRequest());
	server.addMsgProc("display", server.Display());

  server.processMessages();
  
  Msg msg(serverEndPoint, serverEndPoint);  // send to self
  msg.name("msgToSelf");

  /////////////////////////////////////////////////////////////////////
  // Additional tests here, used during development
  //
  //msg.command("echo");
  //msg.attribute("verbose", "show me");
  //server.postMessage(msg);
  //std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  //msg.command("getFiles");
  //msg.remove("verbose");
  //msg.attributes()["path"] = storageRoot;
  //server.postMessage(msg);
  //std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  //msg.command("getDirs");
  //msg.attributes()["path"] = storageRoot;
  //server.postMessage(msg);
  //std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  std::cout << "\n  press enter to exit\n";
  std::cin.get();
  std::cout << "\n";

  msg.command("serverQuit");
  server.postMessage(msg);
  server.stop();
  return 0;
}

