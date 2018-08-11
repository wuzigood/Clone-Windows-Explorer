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

namespace WpfApp1
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        private List<FileInfo> fileInfos;

        public MainWindow()
        {
            InitializeComponent();

            string currentPath = FileManager.GetCurrentDirectory();
            AddressBar.Text = currentPath;
            //FileManager.currentPath = currentPath = "C:";
            //MessageBox.Show(currentPath);

            FileManager.CSetPath(currentPath);

            fileInfos = FileManager.GetFileInfos();
            FileItems.ItemsSource = fileInfos;
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
                    AddressBar.Text = FileManager.currentPath;
                    fileInfos = FileManager.GetFileInfos();
                    FileItems.ItemsSource = fileInfos;
                }
                else if(fileInfo.IsFolder)
                {
                    FileManager.SetPath(FileManager.currentPath + "\\" + Name);
                    AddressBar.Text = FileManager.currentPath;
                    fileInfos = FileManager.GetFileInfos();
                    FileItems.ItemsSource = fileInfos;
                }
            }
        }

        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {

        }
    }
}
