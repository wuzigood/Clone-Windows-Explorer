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
using System.Windows.Shapes;

/*
目前主要存在的问题是C#和C++之前传递结构体。我选择一个一个传结构体，而不是传一个数组什么的。
如果搜索到的条目很大，比如搜索"a"，dll只需要1.5s左右即可完成，但是一个一个传浪费了很多时间。
*/ 

namespace WpfApp1
{
    public class FileInfoItem
    {
        public string Name { set; get; }
        public string Path { set; get; }
    }

    /// <summary>
    /// SearchWindow.xaml 的交互逻辑
    /// </summary>
    public partial class SearchWindow : Window
    {
        public SearchWindow()
        {
            InitializeComponent();
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            this.Hide();
            e.Cancel = true;
        }

        //List<FileInfo> TempFileInfos;
        //int FileCount = 0;

        //private void AddSearchResult(List<FileInfoItem> fileInfos)
        //{
        //    for(uint i=0; i<fileInfos.Count; i++)
        //    {
        //        TempFileInfos.Add(fileInfos[i]);
        //    }
        //    FileInfoList.ItemsSource = TempFileInfos;
        //    FileCount += fileInfos.Count;
        //    CountStatus.Text = "一共有" + FileCount.ToString() + "个条目";
        //}

        private void SetSearchResult(List<FileInfoItem> fileInfos)
        {
            if (fileInfos.Count == 0)
            {
                MessageBox.Show("找不到这个关键字的内容");
            }
            FileInfoList.ItemsSource = fileInfos;
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {   
            if(this.Title == "正在初始化...尚未能搜索")
            {
                MessageBox.Show("正在初始化，请稍后。");
                return;
            }
            if (Pattern.Text.Length == 0)
            {
                MessageBox.Show("不能为空");
                return;
            }
            int Count = 0;
            SetSearchResult(FileManager.SearchFile(Pattern.Text, out Count));
            //FileCount = Count;
            CountStatus.Text = "一共有" + Count.ToString() + "个条目";
        }
    }
}
