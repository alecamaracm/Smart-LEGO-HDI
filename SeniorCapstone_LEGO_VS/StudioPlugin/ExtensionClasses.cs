using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Text;

namespace StudioPlugin
{
    public static class ExtensionClasses
    {
        public static void WriteString(this Stream stream,string toWrite)
        {
            PluginLoader.Log(toWrite);
           byte[] bytes = UTF8Encoding.UTF8.GetBytes(toWrite);
            stream.Write(bytes, 0, bytes.Length);
        }
    }
}
