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
    /// FileItem.xaml 的交互逻辑
    /// </summary>
    public partial class FileItem : UserControl
    {
        public FileItem()
        {
            InitializeComponent();
        }

        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {
            string fileName = FileName.Text;

            if(fileName == "..")
            {
                return;
            }

            MenuItem menuItem = sender as MenuItem;
            string header = menuItem.Header.ToString();

            switch (header)
            {
                case ("重命名"):
                    ReNameBox.Visibility = Visibility.Visible;
                    ReNameBox.Text = FileName.Text;
                    ReNameBox.Select(ReNameBox.Text.Length, 0);
                    ReNameBox.Focus();
                    break;
                case ("复制"):
                    break;
                case ("剪切"):
                    break;
                case ("粘贴"):
                    break;
                case ("删除"):
                    break;
                default:
                    break;
            }
        }

        private void ReNameBox_KeyUp(object sender, KeyEventArgs e)
        {
            if(e.Key == Key.Enter)
            {
                ReNameBox.Visibility = Visibility.Hidden;
                FileName.Text = ReNameBox.Text;
                // System Call to rename the file
            }
        }

        private void ReNameBox_LostFocus(object sender, RoutedEventArgs e)
        {
            ReNameBox.Visibility = Visibility.Hidden;
            FileName.Text = ReNameBox.Text;
            // System Call to rename the file
        }
    }
}
