///////////////////////////////////////////////////////////////////////
// MainWindow.xaml.cs - GUI for Project 4                            //
//                                                                   //
// author:Congyou Liu. May.1st, 2018.                                //
///////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * This package provides a WPF-based GUI for  Project 4.  It's 
 * responsibilities are to:
 * - Provide connect tab to connect the remote repository
 * - Provide checkin, closecheckin, checkout tab
 * - Provide query tab to match the file user want
 * - Provide a display of directory contents of a remote ServerPrototype.
 * - It provides a subdirectory list and a filelist for the selected directory.
 * - You can navigate into subdirectories by double-clicking on subdirectory
 *   or the parent directory, indicated by the name "..".
 *   
 * Required Files:
 * ---------------
 * Mainwindow.xaml, MainWindow.xaml.cs
 * Translater.dll
 * 
 * Maintenance History:
 * --------------------
 * ver 3.0 : 30 Apr 2018
 * - add checkin control
 * - add close checkin control
 * - add query control
 * - add checkout control
 * ver 2.0 : 22 Apr 2018
 * - added tabbed display
 * - moved remote file view to RemoteNavControl
 * - migrated some methods from MainWindow to RemoteNavControl
 * - added local file view
 * - added NoSqlDb with very small demo as server starts up
 * ver 1.0 : 30 Mar 2018
 * - first release
 * - Several early prototypes were discussed in class. Those are all superceded
 *   by this package.
 */

// Translater has to be statically linked with CommLibWrapper
// - loader can't find Translater.dll dependent CommLibWrapper.dll
// - that can be fixed with a load failure event handler
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using System.IO;
using MsgPassingCommunication;

namespace WpfApp1
{
  public partial class MainWindow : Window
  {
    public MainWindow()
    {
      InitializeComponent();
      Console.Title = "Project4Demo GUI Console";
    }

    private Stack<string> pathStack_ = new Stack<string>();
    internal Translater translater;
    internal CsEndPoint endPoint_;
    private Thread rcvThrd = null;
    private Dictionary<string, Action<CsMessage>> dispatcher_ 
      = new Dictionary<string, Action<CsMessage>>();
    internal string saveFilesPath;
    internal string sendFilesPath;

    //----< process incoming messages on child thread >----------------

    private void processMessages()
    {
      ThreadStart thrdProc = () => {
        while (true)
        {
          CsMessage msg = translater.getMessage();
          try
          {
            string msgId = msg.value("command");
            Console.Write("\n  client getting message \"{0}\"", msgId);
            if (dispatcher_.ContainsKey(msgId))
              dispatcher_[msgId].Invoke(msg);
          }
          catch(Exception ex)
          {
            Console.Write("\n  {0}", ex.Message);
            msg.show();
          }
        }
      };
      rcvThrd = new Thread(thrdProc);
      rcvThrd.IsBackground = true;
      rcvThrd.Start();
    }
    //----< add client processing for message with key >---------------

    private void addClientProc(string key, Action<CsMessage> clientProc)
    {
      dispatcher_[key] = clientProc;
    }
    ////----< load getDirs processing into dispatcher dictionary >-------

    private void DispatcherLoadGetDirs()
    {
      Action<CsMessage> getDirs = (CsMessage rcvMsg) =>
      {
        Action clrDirs = () =>
        {
          //NavLocal.clearDirs();
          NavRemote.clearDirs();
					CloseCheckin.clearDirs();
        };
        Dispatcher.Invoke(clrDirs, new Object[] { });
        var enumer = rcvMsg.attributes.GetEnumerator();
        while (enumer.MoveNext())
        {
          string key = enumer.Current.Key;
          if (key.Contains("dir"))
          {
            Action<string> doDir = (string dir) =>
            {
              NavRemote.addDir(dir);
							CloseCheckin.addDir(dir);
            };
            Dispatcher.Invoke(doDir, new Object[] { enumer.Current.Value });
          }
        }
        Action insertUp = () =>
        {
          NavRemote.insertParent();
					CloseCheckin.insertParent();
        };
        Dispatcher.Invoke(insertUp, new Object[] { });
      };
      addClientProc("getDirs", getDirs);
    }
    //----< load getFiles processing into dispatcher dictionary >------

    private void DispatcherLoadGetFiles()
    {
      Action<CsMessage> getFiles = (CsMessage rcvMsg) =>
      {
        Action clrFiles = () =>
        {
          NavRemote.clearFiles();
					CloseCheckin.clearFiles();
        };
        Dispatcher.Invoke(clrFiles, new Object[] { });
        var enumer = rcvMsg.attributes.GetEnumerator();
        while (enumer.MoveNext())
        {
          string key = enumer.Current.Key;
          if (key.Contains("file"))
          {
            Action<string> doFile = (string file) =>
            {
              NavRemote.addFile(file);
							CloseCheckin.addFile(file);
            };
            Dispatcher.Invoke(doFile, new Object[] { enumer.Current.Value });
          }
        }
      };
      addClientProc("getFiles", getFiles);
    }
    //----< load sendFiles processing into dispatcher dictionary >------

    private void DispatcherLoadSendFile()
    {
      Action<CsMessage> sendFile = (CsMessage rcvMsg) => {
        Console.Write("\n  processing incoming file");
        string fileName = "";
        var enumer = rcvMsg.attributes.GetEnumerator();
        while (enumer.MoveNext()) {
          string key = enumer.Current.Key;
          if (key.Contains("sendingFile")) {
            fileName = enumer.Current.Value;
          }
					if (key.Contains("status")){
						Action<string> writestatus = (string sta) =>{
							connectstatus.Text = "CheckOut status: " + sta + ", see CheckedOutFiles folder";
						};
						Dispatcher.Invoke(writestatus, new Object[] { enumer.Current.Value });
					}
					if (key.Contains("name")){
						Action<string> writename = (string name) =>{
							QueryControl.name.Text = name;
						};
						Dispatcher.Invoke(writename, new Object[] { enumer.Current.Value });
					}
					if (key.Contains("category")){
						Action<string> writecategory = (string category) =>{
							QueryControl.category.Text = category;
						};
						Dispatcher.Invoke(writecategory, new Object[] { enumer.Current.Value });
					}
					if (key.Contains("children")){
						Action<string> writechildren = (string children) =>
						{
							QueryControl.children.Text = children;
						};
						Dispatcher.Invoke(writechildren, new Object[] { enumer.Current.Value });
					}
					if (key.Contains("version")){
						Action<string> writeversion = (string version) =>{
							QueryControl.version.Text = version;
						};
						Dispatcher.Invoke(writeversion, new Object[] { enumer.Current.Value });
					}
				}
        if (fileName.Length > 0){
          Action<string> act = (string fileNm) => { showFile(fileNm); };
          Dispatcher.Invoke(act, new object[] { fileName });
        }
      };
      addClientProc("sendFile", sendFile);
    }

		//----< load checkin processing into dispatcher dictionary >------
		private void DispatcherLoadcheckin()
		{
			Action<CsMessage> checkin = (CsMessage rcvMsg) =>
			{
				var enumer = rcvMsg.attributes.GetEnumerator();
				while (enumer.MoveNext())
				{
					string key = enumer.Current.Key;
					if (key.Contains("status"))
					{
						Action<string> writestatus = (string sta) =>
						{
							connectstatus.Text = "CheckIn status: " + sta;
						};
						Dispatcher.Invoke(writestatus, new Object[] { enumer.Current.Value });
					}
					if (key.Contains("fileName"))
					{
						Action<string> doFile = (string file) =>
						{
							NavRemote.addFile(file);
						};
						Dispatcher.Invoke(doFile, new Object[] { enumer.Current.Value });
					}
				}
			};
			addClientProc("checkin", checkin);
		}

		//----< load closecheckin processing into dispatcher dictionary >------
		private void DispatcherLoadCloseCheckIn()
		{
			Action<CsMessage> closecheckin = (CsMessage rcvMsg) =>
			{
				var enumer = rcvMsg.attributes.GetEnumerator();
				while (enumer.MoveNext())
				{
					string key = enumer.Current.Key;
					if (key.Contains("status"))
					{
						Action<string> writestatus = (string sta) =>
						{
							connectstatus.Text = "CloseCheckIn status: " + sta;
						};

						Dispatcher.Invoke(writestatus, new Object[] { enumer.Current.Value });
					}
					
				}
			};
			addClientProc("closecheckin", closecheckin);
		}

		//----< load queryrequest processing into dispatcher dictionary >------
		private void DispatcherLoadQueryRequest()
		{
			Action<CsMessage> queryrequest = (CsMessage rcvMsg) =>
			{
				Action clearFiles = () =>
				{
					QueryControl.clearFiles();
				};
				Dispatcher.Invoke(clearFiles, new Object[] { });
				var enumer = rcvMsg.attributes.GetEnumerator();
				while (enumer.MoveNext())
				{
					string key = enumer.Current.Key;
					if (key.Contains("hasname"))
					{
						Action<string> process = (string name) =>
						{
							int i = 0;
							while (i < name.Length)
							{
								string s = "";
								while (name[i] != '+')
								{
									s = s + name[i];
									i++;
								}
								if (s != "")
								{
									QueryControl.addFile(s);
								}
								i++;
							}
						};
						Dispatcher.Invoke(process, new Object[] { enumer.Current.Value });
					}
				}
			};
			addClientProc("queryrequest", queryrequest);
		}
    //----< load all dispatcher processing >---------------------------

    private void loadDispatcher()
    {
      DispatcherLoadGetDirs();
      DispatcherLoadGetFiles();
      DispatcherLoadSendFile();
			DispatcherLoadcheckin();
			DispatcherLoadCloseCheckIn();
			DispatcherLoadQueryRequest();

		}
    //----< start Comm, fill window display with dirs and files >------

    private void Window_Loaded(object sender, RoutedEventArgs e)
    {
      // start Comm
      /*endPoint_ = new CsEndPoint();
      endPoint_.machineAddress = "localhost";
      endPoint_.port = 8082;
      NavRemote.navEndPoint_ = endPoint_;

      translater = new Translater();
      translater.listen(endPoint_);

      // start processing messages
      processMessages();

      // load dispatcher
      loadDispatcher();

      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = "localhost";
      serverEndPoint.port = 8080;
      pathStack_.Push("../Storage");

      NavRemote.PathTextBlock.Text = "Storage";
      NavRemote.pathStack_.Push("../Storage");

      NavLocal.PathTextBlock.Text = "LocalStorage";
      NavLocal.pathStack_.Push("");
      NavLocal.localStorageRoot_ = "../../../../LocalStorage";
      saveFilesPath = translater.setSaveFilePath("../../../SaveFiles");
      sendFilesPath = translater.setSendFilePath("../../../SendFiles");

      NavLocal.refreshDisplay();
      NavRemote.refreshDisplay();*/
      test();
    }
    //----< strip off name of first part of path >---------------------

    public string removeFirstDir(string path)
    {
      string modifiedPath = path;
      int pos = path.IndexOf("/");
      modifiedPath = path.Substring(pos + 1, path.Length - pos - 1);
      return modifiedPath;
    }
    //----< show file text >-------------------------------------------

    private void showFile(string fileName)
    {
      Paragraph paragraph = new Paragraph();
      string fileSpec = saveFilesPath + "\\" + fileName;
      string fileText = File.ReadAllText(fileSpec);
      paragraph.Inlines.Add(new Run(fileText));
      CodePopupWindow popUp = new CodePopupWindow();
      popUp.codeView.Blocks.Clear();
      popUp.codeView.Blocks.Add(paragraph);
      popUp.Show();
    }

		private void txtMsg1_TextChanged(object sender, TextChangedEventArgs e)
		{

		}

		private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{

		}
		//----< click connect button in connect tab >------
		private void Button_Click_Connect(object sender, RoutedEventArgs e)
		{
			// start Comm
			endPoint_ = new CsEndPoint();
      endPoint_.machineAddress = "localhost";
      endPoint_.port = Convert.ToInt32(txtMsg1.Text);
      NavRemote.navEndPoint_ = endPoint_;
			CloseCheckin.navEndPoint_ = endPoint_;
			QueryControl.navEndPoint_ = endPoint_;

      translater = new Translater();
      translater.listen(endPoint_);

      // start processing messages
      processMessages();

      // load dispatcher
      loadDispatcher();

      CsEndPoint serverEndPoint = new CsEndPoint();
      serverEndPoint.machineAddress = "localhost";
      serverEndPoint.port = 8080;
      pathStack_.Push("../Storage");

      NavRemote.PathTextBlock.Text = "Storage";
      NavRemote.pathStack_.Push("../Storage");
			CloseCheckin.PathTextBlock.Text = "Storage";
			CloseCheckin.pathStack_.Push("../Storage");
			NavLocal.PathTextBlock.Text = "LocalStorage";
      NavLocal.pathStack_.Push("");
      NavLocal.localStorageRoot_ = "../../../../LocalStorage";
			QueryControl.pathStack_.Push("../Storage");
			saveFilesPath = translater.setSaveFilePath("../../../SaveFiles");
      sendFilesPath = translater.setSendFilePath("../../../SendFiles");

      NavLocal.refreshDisplay();
			CloseCheckin.refreshDisplay();
      NavRemote.refreshDisplay();
			connectstatus.Text = "Connected! Port number is " + txtMsg1.Text;
		}
		//----<tests >---------------------------------

		void test()
		{
			//MouseButtonEventArgs e = new MouseButtonEventArgs(null, 0, 0);
			//ConnectButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
			//DirList.SelectedIndex = 1;
			//DirList_MouseDoubleClick(this, e);
			Task test = test1()
				.ContinueWith(t => test2()).Unwrap()
				.ContinueWith(t => test3()).Unwrap()
				.ContinueWith(t => test4()).Unwrap()
				.ContinueWith(t => test5()).Unwrap()
				.ContinueWith(t => test6()).Unwrap()
				.ContinueWith(t => test7()).Unwrap()
				.ContinueWith(t => test8()).Unwrap()
				.ContinueWith(t => test9()).Unwrap()
				.ContinueWith(t => test10()).Unwrap()
				.ContinueWith(t => test11()).Unwrap();
		}

		Task test1()
		{
			Task test1 = Task.Factory.StartNew(() =>
			{
				Console.WriteLine("\n-----Now beginning testing. -----\n");
				Console.WriteLine("\n-----Firstly demonstrate requirement 4. -----\n");
				Console.WriteLine("\n-----In Connect tab we can input local machine port. We suppose it be 8088.-----\n");
				Console.WriteLine("\n-----Then click connect button.----- \n");
			}).ContinueWith((t) =>
			{
				Dispatcher.Invoke((Action)(() =>
				{
					ConnectButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
				}));
			});
			return test1;
		}

		Task test2()
		{
			Task test2 = Task.Factory.StartNew(() =>
			{
				Thread.Sleep(2000);
				Console.WriteLine("\n-----We start check-in. It meet requirement 5 because it sends checkin message.-----\n");
				Console.WriteLine("\n-----Also meet requirement 3: upload file-----\n");
				Console.WriteLine("\n-----You can input file description, children file name, category in GUI.-----\n");
				Console.WriteLine("\n-----Then doubleclick the file in the filelist.-----\n");
				Console.WriteLine("\n-----Checkin status will show at the bottom. If necessary do refresh.----- \n");
			}).ContinueWith((t) =>
			{
				Dispatcher.Invoke((Action)(() =>
				{
					Console.WriteLine("\n-----We first check in file4.h with no children and category C.-----\n");
					NavLocal.Des.Text = "file4.h";
					NavLocal.children.Text = "";
					NavLocal.cate.Text = "C";
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					NavLocal.FileList.SelectedIndex = 2;
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					MouseButtonEventArgs mouseEve = new MouseButtonEventArgs(Mouse.PrimaryDevice, Environment.TickCount, MouseButton.Left);
					mouseEve.RoutedEvent = ListBox.MouseDoubleClickEvent;
					NavLocal.FileList.RaiseEvent(mouseEve);
				}));
			});
			return test2;
		}

		Task test3()
		{
			Task test3 = Task.Factory.StartNew(() =>
			{
				Thread.Sleep(2000);
				Console.WriteLine("\n-----We then check in file3.h which have children file4 and category B-----\n");
				
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					NavLocal.Des.Text = "file3.h";
					NavLocal.children.Text = "C::file4.h.1";
					NavLocal.cate.Text = "B";
					NavLocal.FileList.SelectedIndex = 1;
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					MouseButtonEventArgs mouseEve = new MouseButtonEventArgs(Mouse.PrimaryDevice, Environment.TickCount, MouseButton.Left);
					mouseEve.RoutedEvent = ListBox.MouseDoubleClickEvent;
					NavLocal.FileList.RaiseEvent(mouseEve);
				}));
			});
			return test3;
		}

		Task test4()
		{
			Task test4 = Task.Factory.StartNew(() =>
			{
				Thread.Sleep(2000);
				Console.WriteLine("\n-----We then check in file1.h which have children file3 and category B-----\n");
				
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					NavLocal.Des.Text = "file1.h";
					NavLocal.children.Text = "B::file3.h.1";
					NavLocal.cate.Text = "B";
					NavLocal.FileList.SelectedIndex = 0;
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					MouseButtonEventArgs mouseEve = new MouseButtonEventArgs(Mouse.PrimaryDevice, Environment.TickCount, MouseButton.Left);
					mouseEve.RoutedEvent = ListBox.MouseDoubleClickEvent;
					NavLocal.FileList.RaiseEvent(mouseEve);
				}));
			});
			return test4;
		}

		Task test5()
		{
			Task test5 = Task.Factory.StartNew(() =>
			{
				Thread.Sleep(2000);
				Console.WriteLine("\n-----Now we do close checkin.-----\n");
				Console.WriteLine("\n-----You can doubleclick the file in the filelist to do closing.------\n");
				Console.WriteLine("\n-----We firstly close file3, since its children isn't closed yet it will fail.-----\n");
			}).ContinueWith((t) =>
			{
				Dispatcher.Invoke((Action)(() =>
				{
					CloseCheckin.refreshDisplay();
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					CloseCheckin.FileList.SelectedIndex = 1;
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					MouseButtonEventArgs mouseEve = new MouseButtonEventArgs(Mouse.PrimaryDevice, Environment.TickCount, MouseButton.Left);
					mouseEve.RoutedEvent = ListBox.MouseDoubleClickEvent;
					CloseCheckin.FileList.RaiseEvent(mouseEve);
				}));
			});
			return test5;
		}

		Task test6()
		{
			Task test6 = Task.Factory.StartNew(() =>
			{
				Thread.Sleep(2000);
				Console.WriteLine("\n-----We close file4.-----\n");
			}).ContinueWith((t) =>
			{
				Dispatcher.Invoke((Action)(() =>
				{
					CloseCheckin.refreshDisplay();
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					CloseCheckin.FileList.SelectedIndex = 2;
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					MouseButtonEventArgs mouseEve = new MouseButtonEventArgs(Mouse.PrimaryDevice, Environment.TickCount, MouseButton.Left);
					mouseEve.RoutedEvent = ListBox.MouseDoubleClickEvent;
					CloseCheckin.FileList.RaiseEvent(mouseEve);
				}));
			});
			return test6;
		}

		Task test7()
		{
			Task test7 = Task.Factory.StartNew(() =>
			{
				Thread.Sleep(2000);
				Console.WriteLine("\n-----We close file1.-----\n");
			}).ContinueWith((t) =>
			{
				Dispatcher.Invoke((Action)(() =>
				{
					CloseCheckin.refreshDisplay();
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					CloseCheckin.FileList.SelectedIndex = 0;
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					MouseButtonEventArgs mouseEve = new MouseButtonEventArgs(Mouse.PrimaryDevice, Environment.TickCount, MouseButton.Left);
					mouseEve.RoutedEvent = ListBox.MouseDoubleClickEvent;
					CloseCheckin.FileList.RaiseEvent(mouseEve);
				}));
			});
			return test7;
		}

		Task test8()
		{
			Task test8 = Task.Factory.StartNew(() =>
			{
				Thread.Sleep(2000);
				Console.WriteLine("\n-----Now we do checkout.-----\n");
				Console.WriteLine("\n-----Choose if you want to check out with children.-----\n");
				Console.WriteLine("\n-----You can doubleclick the file in the filelist to do checkout. It will also open the file you selected.-----\n");
				Console.WriteLine("\n-----After checkout look at the status bar and check CheckedOutFile folder.------\n");
				Console.WriteLine("\n-----This meet requirement 3 and 6.-----\n");
				Console.WriteLine("\n-----We firstly checkout only file1.-----\n");
			}).ContinueWith((t) =>
			{
				Dispatcher.Invoke((Action)(() =>
				{
					NavRemote.refreshDisplay();
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					NavRemote.FileList.SelectedIndex = 0;
					NavRemote.withchildren.IsChecked = false;
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					MouseButtonEventArgs mouseEve = new MouseButtonEventArgs(Mouse.PrimaryDevice, Environment.TickCount, MouseButton.Left);
					mouseEve.RoutedEvent = ListBox.MouseDoubleClickEvent;
					NavRemote.FileList.RaiseEvent(mouseEve);
				}));
			});
			return test8;
		}

		Task test9()
		{
			Task test9 = Task.Factory.StartNew(() =>
			{
				Thread.Sleep(2000);
				Console.WriteLine("\n-----We then checkout file3 and its children.-----\n");
			}).ContinueWith((t) =>
			{
				Dispatcher.Invoke((Action)(() =>
				{
					NavRemote.refreshDisplay();
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					NavRemote.FileList.SelectedIndex = 1;
					NavRemote.withchildren.IsChecked = true;
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					MouseButtonEventArgs mouseEve = new MouseButtonEventArgs(Mouse.PrimaryDevice, Environment.TickCount, MouseButton.Left);
					mouseEve.RoutedEvent = ListBox.MouseDoubleClickEvent;
					NavRemote.FileList.RaiseEvent(mouseEve);
				}));
			});
			return test9;
		}

		Task test10()
		{
			Task test10 = Task.Factory.StartNew(() =>
			{
				Thread.Sleep(2000);
				Console.WriteLine("\n-----We do some query request, which means finding the matching file according to the name(by regex is fine) and category we input.-----\n");
				Console.WriteLine("\n-----Input the regex in hasname and the whole category name in hascategory textbox then click request.-----\n");
				Console.WriteLine("\n-----Now we input file*, category B to match file name and category.The result should be two files-----\n");
				Console.WriteLine("\n-----They will be shown in the listbox below.-----\n");
			}).ContinueWith((t) =>
			{
				Dispatcher.Invoke((Action)(() =>
				{
					QueryControl.hasname.Text = "file*";
					QueryControl.hascategory.Text = "B";
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					QueryControl.Request.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
				}));
			});
			return test10;
		}

		Task test11()
		{
			Task test11 = Task.Factory.StartNew(() =>
			{
				Thread.Sleep(2000);
				Console.WriteLine("\n-----We double click the first file listed in the listbox.-----\n");
				Console.WriteLine("\n-----It will display the metadata(name, children, category, description) of this file in the text box.------\n");
				Console.WriteLine("\n-----And also open the file to see the text.-----\n");
			}).ContinueWith((t) =>
			{
				Dispatcher.Invoke((Action)(() =>
				{
					QueryControl.FileList.SelectedIndex = 0;
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Dispatcher.Invoke((Action)(() =>
				{
					MouseButtonEventArgs mouseEve = new MouseButtonEventArgs(Mouse.PrimaryDevice, Environment.TickCount, MouseButton.Left);
					mouseEve.RoutedEvent = ListBox.MouseDoubleClickEvent;
					QueryControl.FileList.RaiseEvent(mouseEve);
				}));
			}).ContinueWith((t) =>
			{
				Thread.Sleep(2000);
				Console.WriteLine("\n-----Now requirement 2 is all demonstrated.-----\n");
				Console.WriteLine("\n-----It is your turn to do anything with this GUI. Have fun!-----\n");
			});
			return test11;
		}
	}
}
