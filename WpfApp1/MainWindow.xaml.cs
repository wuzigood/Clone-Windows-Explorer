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
        private static extern int Sum(int a, ref int b);

        int count = 0;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void menuItem_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("menuItem Click");
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {            
            //MessageBox.Show(String.Format("{0} x {1}", MyGrid.ActualWidth, MyGrid.ActualHeight));
            //ListBoxItem listBoxItem = new ListBoxItem();
            ////listBox1.

            //int b = 20;
            //int a = Sum(10, ref b);
            //label1.Content = b.ToString();
            //count++;
            //if (count > 4)
            //{
            //    MyGrid.RowDefinitions.Add(new RowDefinition());
            //}
            //Button button = new Button
            //{
            //    Name = "button" + count.ToString(),
            //    Visibility = Visibility.Visible,
            //    Content = count.ToString(),
            //    Width = 200,
            //    Height = 200
            //};
            //button.SetValue(Grid.ColumnProperty, 0);
            //button.SetValue(Grid.RowProperty, 0);
            //MyGrid.Children.Add(button);
        }

        private void ListBox_DoubleClick(object sender, SelectionChangedEventArgs e)
        {
            //label1.Content = "double click";
        }

        private void CommomClick(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Click");
        }
    }
}
