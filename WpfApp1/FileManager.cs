using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Windows.Media.Imaging;
using System.Windows;

namespace WpfApp1
{
    class FileManager
    {
        /*
         * 这个函数用于测试DLL是否正常可用
         */ 
        [DllImport("..\\..\\..\\Debug\\core.dll", EntryPoint = "Sum", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Sum(int a, int b);

        /*
         * 返回的是一个地址，将这个指针传给Marshal类调用PtrToString可以获取以这个地址为首的一个字符串。
         * 字符串的内容是当前目录地址。
         */
        [DllImport("..\\..\\..\\Debug\\core.dll", EntryPoint = "GetCurrentDir", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GetCurrentDir();

        /*
         * 这个结构体等同于dll中的结构体定义
         * CharSet设置为Unicode，因为dll中结构体的声明的是wchar类型，是Unicode的，这样就可以等同了
         */ 
        [StructLayout(LayoutKind.Sequential, CharSet= CharSet.Unicode)]
        public struct CFileInfo
        {
            public bool IsFolder;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
            public string Name;
        };
        /*
         * 初始化，第一个文件夹一般是当前文件夹 " . " 
         */ 
        [DllImport("..\\..\\..\\Debug\\core.dll", EntryPoint = "FirstFind", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool FirstFind(ref CFileInfo cfileInfo);

        /*
         * 获取下一个。其实这里的调用相当于直接调用windows API。逻辑等同。
         * 这个和上一个函数的作用是传递内容。
         */ 
        [DllImport("..\\..\\..\\Debug\\core.dll", EntryPoint = "NextFind", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool NextFind(ref CFileInfo cfileInfo);

        /*
         * 设置路径, public 方法
         */ 
        [DllImport("..\\..\\..\\Debug\\core.dll", EntryPoint = "SetPath",  CharSet =CharSet.Unicode,CallingConvention = CallingConvention.Cdecl)]
        public static extern void CSetPath(string val);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 1)]
        public struct SearchFileResult
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
            public string Name;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
            public string Path;
        };

        [DllImport("..\\..\\..\\Debug\\SearchFile.dll", EntryPoint = "InitSearchFile", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool InitSearchFile();

        [DllImport("..\\..\\..\\Debug\\SearchFile.dll", EntryPoint = "InitFindFile", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void InitFindFile(string val);

        [DllImport("..\\..\\..\\Debug\\SearchFile.dll", EntryPoint = "GetFindFile", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool GetFindFile(ref SearchFileResult searchFileResult);

        public static string currentPath;

        public static List<FileInfo> fileInfos; // = new List<FileInfo>();

        public static string GetCurrentDirectory()
        {
            IntPtr path = GetCurrentDir();
            currentPath = Marshal.PtrToStringUni(path);
            return currentPath;
        }

        public static List<FileInfo> GetFileInfos()
        {
            fileInfos = new List<FileInfo>();
            fileInfos.Clear();
            GetFirstFile();
            CFileInfo cfileInfo = new CFileInfo();
            while (NextFind(ref cfileInfo))
            {
                FileInfo fileInfo = new FileInfo
                {
                    Name = cfileInfo.Name,
                    IsFolder = cfileInfo.IsFolder
                };
                if (fileInfo.IsFolder)
                {
                    fileInfo.ImagePath = new BitmapImage(new Uri("Image\\folder1.ico", System.UriKind.Relative));
                    //fileInfo.ImagePath = new BitmapImage();
                }
                else
                {
                    fileInfo.ImagePath = new BitmapImage(new Uri("Image\\document-icon.png", System.UriKind.Relative));
                }
                fileInfos.Add(fileInfo);
            }
            return fileInfos;
        }

        private static FileInfo GetFirstFile()
        {
            CFileInfo cfileInfo = new CFileInfo();
            FileInfo fileInfo = new FileInfo();
            if (FirstFind(ref cfileInfo))
            {
                fileInfo.Name = cfileInfo.Name;
                fileInfo.IsFolder = cfileInfo.IsFolder;
                if(fileInfo.IsFolder)
                {
                    fileInfo.ImagePath = new BitmapImage(new Uri("Image\\folder1.ico", System.UriKind.Relative));
                }
            }
            return fileInfo;
        }

        public static void SetPath(string path)
        {
            //MessageBox.Show(path);
            currentPath = path;
            CSetPath(currentPath);
        }

        public static List<FileInfoItem> SearchFile(string pattern, out int Count)
        {
            InitFindFile(pattern);

            List<FileInfoItem> fileInfoItems = new List<FileInfoItem>();
            SearchFileResult searchFileResult = new SearchFileResult();
            int i = 0;
            while(GetFindFile(ref searchFileResult))
            {
                i++;
                FileInfoItem fileInfoItem = new FileInfoItem();
                fileInfoItem.Name = searchFileResult.Name;
                fileInfoItem.Path = searchFileResult.Path;
                fileInfoItems.Add(fileInfoItem);
            }
            Count = i;
            return fileInfoItems;
        }
    }
}
