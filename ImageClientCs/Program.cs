using System;
using ImageSource;

namespace ImageClientCs
{
    internal class Program
    {
        static void AccessImageData(IHandleMgr mgr, bool verbose)
        {
            IImageHandle handle = mgr.GetImageHandle();

            Image2d frame = handle.GetData();

            if (verbose)
            {
                Console.WriteLine($"Frame time={frame.time}");
                Console.WriteLine($"Frame format={frame.format}");
                Console.WriteLine($"Frame dims=({frame.dims[0]}, {frame.dims[1]})");
                Console.WriteLine($"Frame size={frame.data.Length}");
                Console.Write("Frame data (first 128bytes): ");
                for (int i = 0; (i < frame.data.Length) && (i < 128); i++)
                {
                    var elm = frame.data.GetValue(i);
                    Console.Write(elm + ", ");
                }
                Console.WriteLine();
            }
        }


        static void Main(string[] args)
        {
            // instantiate COM server (in separate ImageSource process)
            IHandleMgr mgr = new ImageSource.HandleMgr();

            // access image-data 100 times with console logging every 10th iteration
            for (int it = 0; it < 100; it++)
            {
                bool verbose = (it % 10) == 0;
                if (verbose)
                    Console.WriteLine($"Iteration {it}...");
                AccessImageData(mgr, verbose);
                if (verbose)
                    Console.WriteLine();
            }

            // call GC to release all COM server references before exiting
            mgr = null;
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }
    }
}
