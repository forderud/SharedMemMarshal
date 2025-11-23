using System;
using ImageSource;

namespace ImageClientCs
{
    internal class Program
    {
        static void Main(string[] args)
        {
            // instantiate COM server
            IHandleMgr mgr = new ImageSource.HandleMgr();
            // equivalent to Activator.CreateInstance(Type.GetTypeFromCLSID(typeof(ImageSource.HandleMgr).GUID));

            IImageHandle handle = mgr.GetImageHandle();

            Image2d frame = handle.GetData();
        }
    }
}
