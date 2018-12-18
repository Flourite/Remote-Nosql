///////////////////////////////////////////////////////////////////////
// QueryControl.xaml.cs - Control GUI for query request              //
// ver 1.0                                                           //
// Implemented by Congyou Liu                                        //
///////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * This package provides a WPF-based control GUI for Project4.  It's 
 * responsibilities are to:
 * - show query result of file and file metadata information
 * - Provide a display of directory contents of the query request.
 *   
 * Required Files:
 * ---------------
 * QueryControl.xaml, QueryControl.xaml.cs
 * 
 * Maintenance History:
 * --------------------
 * ver 1.0 : 30 Apr 2018
 * - first release
 */
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
using MsgPassingCommunication;

namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for QueryControl.xaml
    /// </summary>
  public partial class QueryControl : UserControl
  {
		internal CsEndPoint navEndPoint_;
		internal Stack<string> pathStack_ = new Stack<string>();
		public QueryControl()
    {
			InitializeComponent();
    }

		//----<click request button to send query>-------
		private void Request_Click(object sender, RoutedEventArgs e)
		{
			MainWindow win = (MainWindow)Window.GetWindow(this);
			CsEndPoint serverEndPoint = new CsEndPoint();
			serverEndPoint.machineAddress = "localhost";
			serverEndPoint.port = 8080;
			CsMessage msg = new CsMessage();
			msg.add("to", CsEndPoint.toString(serverEndPoint));
			msg.add("from", CsEndPoint.toString(navEndPoint_));
			msg.add("command", "queryrequest");
			msg.add("hasName", hasname.Text);
			msg.add("hasCategory", hascategory.Text);
			//msg.add("path", pathStack_.Peek());
			win.translater.postMessage(msg);
			
		}


		//----< function dispatched by child thread to main thread >-------

		internal void clearFiles()
		{
			FileList.Items.Clear();
		}
		//----< function dispatched by child thread to main thread >-------

		internal void addFile(string file)
		{
			FileList.Items.Add(file);
		}
		//----< Display file metadata information >-------
		private void FileList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
		{
			MainWindow win = (MainWindow)Window.GetWindow(this);
			string fileName = (string)FileList.SelectedItem;
			CsEndPoint serverEndPoint = new CsEndPoint();
			serverEndPoint.machineAddress = "localhost";
			serverEndPoint.port = 8080;
			CsMessage msg = new CsMessage();
			msg.add("to", CsEndPoint.toString(serverEndPoint));
			msg.add("from", CsEndPoint.toString(navEndPoint_));
			msg.add("command", "display");
			msg.add("path", pathStack_.Peek());
			msg.add("fileName", fileName);
			win.translater.postMessage(msg);
		}


	}
}
