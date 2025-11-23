using System;
using ImageSource;

namespace ImageClientCs
{
    internal class Program
    {
        static void AccessImageData(IHandleMgr mgr)
        {
            IImageHandle handle = mgr.GetImageHandle();

            Image2d frame = handle.GetData();

            Console.WriteLine($"Frame time={frame.time}");
            Console.WriteLine($"Frame format={frame.format}");
            Console.WriteLine($"Frame dims=({frame.dims[0]}, {frame.dims[1]})");
            Console.WriteLine($"Frame size={frame.data.Length}");
            Console.WriteLine("Frame data=");
            for (int i = 0; (i < frame.data.Length) && (i < 128); i++)
            {
                var elm = frame.data.GetValue(i);
                Console.Write(elm + ", ");
            }
            Console.WriteLine();
        }


        static void Main(string[] args)
        {
            // instantiate COM server
            // IHandleMgr mgr = new ImageSource.HandleMgr();
            var guid = Guid.Parse("{C8767698-8297-4BE1-904E-FC25034851D2}"); // HandleMgr CLSID
            var mgr = (IHandleMgr)Activator.CreateInstance(Type.GetTypeFromCLSID(guid));

            for (int it = 0; it < 10; it++)
            {
                Console.WriteLine($"Iteration {it}...");
                AccessImageData(mgr);
                Console.WriteLine();
            }

            // call GC to release all COM server references before exiting
            mgr = null;
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }
    }
}
