using Memory;
using System.Reflection.PortableExecutable;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.RegularExpressions;

Mem mem = new Mem();

string path;
string procName = "Cemu.exe";
string baseAddress;
string ScreenOrientationOffset = "0x1012667D";

HookCemu();
void HookCemu()
{
    Console.WriteLine("Waiting for Cemu...");

    if(mem.OpenProcess(procName))
    {
        Console.WriteLine("Cemu hooked");
        mem.OpenProcess(procName);
        path = mem.mProc.MainModule.FileName.Replace(procName, "log.txt");
        GetBaseAddress();
    }
    else
    {
        Console.Clear();
        HookCemu();
    }
}

void GetBaseAddress()
{
    //Read the log file
    using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
    using (var sr = new StreamReader(fs, Encoding.Default))
    {
        string pattern = @"base:\s*0x([0-9a-fA-F]+)";
        Match match = Regex.Match(sr.ReadToEnd(), pattern);
        baseAddress = "0x" + match.Groups[1].Value.TrimStart('0');
    }

    Console.WriteLine("Base address: " + baseAddress);

    ChangeScreenOrientation();
}

void ChangeScreenOrientation()
{
    long fullAddress = Convert.ToInt64(baseAddress.Trim(), 16) + Convert.ToInt64(ScreenOrientationOffset.Trim(), 16);
    string hexfullAddress = "0x" + fullAddress.ToString("X");

    Console.WriteLine("Screen Orientation address: " + hexfullAddress);
    mem.WriteMemory(hexfullAddress, "byte", "0");

    Console.WriteLine("Screen Orientation successfully changed to Wii mode" + "\n");
    Console.WriteLine("Click any button to close this windows");
    Console.ReadKey();
    Environment.Exit(0);
}