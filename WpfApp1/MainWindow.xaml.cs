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
using System.Runtime.InteropServices;

namespace WpfApp1
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        [DllImport("C:\\Users\\zzk\\source\\repos\\WpfApp1\\Debug\\core.dll", EntryPoint ="Sum", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Sum(int a, int b);

        [DllImport("C:\\Users\\zzk\\source\\repos\\WpfApp1\\Debug\\core.dll", EntryPoint = "GetCurrentDir", CharSet =CharSet.Unicode,CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GetCurrentDir();

        private int width = 100;
        private int height = 100;

        public MainWindow()
        {
            InitializeComponent();

            IntPtr intPtr = GetCurrentDir();
            string currentDir = Marshal.PtrToStringUni(intPtr);
            AddressBar.Text = currentDir;
        }

        private void menuItem_Click(object sender, RoutedEventArgs e)
        {
            MenuItem menuItem = (MenuItem)sender;
            string val = menuItem.Header as string;
            switch(val)
            {
                case "新建文件夹":
                    Add_Folder();
                    break;
                case "新建文件":
                    Add_File();
                    break;
                default:
                    break;
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
        }

        private void Mouse_Enter()
        {

        }

        private void Add_Folder()
        {
            Button button = new Button();
            button.Width = width;
            button.Height = height;
            button.Visibility = Visibility.Visible;
            ImageBrush imageBrush = new ImageBrush();
            imageBrush.ImageSource =
                new BitmapImage(
                    new Uri("C:\\Users\\zzk\\source\\repos\\WpfApp1\\WpfApp1\\bin\\Debug\\folder.png")
                );
            button.Background = imageBrush;
            wrapPanel.Children.Add(button);
        }

        private void Add_File()
        {
            Button button = new Button();
            button.Width = width;
            button.Height = height;
            button.Visibility = Visibility.Visible;
            ImageBrush imageBrush = new ImageBrush();
            imageBrush.ImageSource =
                new BitmapImage(
                    new Uri("C:\\Users\\zzk\\source\\repos\\WpfApp1\\WpfApp1\\bin\\Debug\\document.ico")
                );
            button.Background = imageBrush;
            wrapPanel.Children.Add(button);
        }
    }
}
