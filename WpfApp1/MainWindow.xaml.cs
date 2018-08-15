using System;
using System.Collections.Generic;
using System.ComponentModel;
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

namespace WpfApp1
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        private List<FileInfo> fileInfos;

        BackgroundWorker backgroundWorker = new BackgroundWorker();

        SearchWindow searchWindow = new SearchWindow();

        public MainWindow()
        {
            InitializeComponent();

            string currentPath = FileManager.GetCurrentDirectory();
            //AddressBar.Text = currentPath;
            this.Title = currentPath;
            //FileManager.currentPath = currentPath = "C:";
            //MessageBox.Show(currentPath);

            FileManager.CSetPath(currentPath);

            fileInfos = FileManager.GetFileInfos();
            FileItems.ItemsSource = fileInfos;

            backgroundWorker.DoWork += DoWork_Handler;
            backgroundWorker.RunWorkerCompleted += RunWorkerCompleted_Handler;
            backgroundWorker.RunWorkerAsync();

            searchWindow.Title = "正在初始化...尚未能搜索";
        }

        private void FileItems_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if(FileItems.SelectedItems.Count == 1)
            {
                FileInfo fileInfo = FileItems.SelectedItems[0] as FileInfo;

                string Name = fileInfo.Name;
                if (Name == ".")
                {
                    // Do Nothing
                }
                else if (Name == "..")
                {
                    //Name = FileManager.currentPath.TrimEnd("\\");
                    FileManager.currentPath = FileManager.currentPath.Remove(FileManager.currentPath.LastIndexOf("\\") + 1);
                    FileManager.currentPath = FileManager.currentPath.TrimEnd('\\');
                    // Update 
                    FileManager.SetPath(FileManager.currentPath);
                    //AddressBar.Text = FileManager.currentPath;
                    this.Title = FileManager.currentPath;
                    fileInfos = FileManager.GetFileInfos();
                    FileItems.ItemsSource = fileInfos;
                }
                else if(fileInfo.IsFolder)
                {
                    FileManager.SetPath(FileManager.currentPath + "\\" + Name);
                    //AddressBar.Text = FileManager.currentPath;
                    this.Title = FileManager.currentPath;
                    fileInfos = FileManager.GetFileInfos();
                    FileItems.ItemsSource = fileInfos;
                }
                else
                {
                    // It is a Common File, use C# to open the default application
                    try
                    {
                        System.Diagnostics.Process.Start(FileManager.currentPath + "\\" + Name);
                    }
                    catch { }
                }
            }
        }

        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("blank");
        }

        private void FileItems_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if(FileItems.SelectedItems.Count > 0)
            {
                FileItems.SelectedItem = null;
                FileItems.Focus();
            }
        }

        private void Search_Click(object sender, RoutedEventArgs e)
        {
            searchWindow.Show();
        }

        private void DoWork_Handler(object sender, DoWorkEventArgs args)
        {
            FileManager.InitSearchFile();
        }

        private void RunWorkerCompleted_Handler(object sender, RunWorkerCompletedEventArgs args)
        {
            searchWindow.Title = "初始化完成";
        }
    }
}
