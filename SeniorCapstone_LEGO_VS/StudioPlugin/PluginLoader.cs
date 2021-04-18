using System;
using System.Collections.Generic;
using System.Text;
using UnityEngine;
using Unity;
using System.IO;
using UnityEngine.SceneManagement;

namespace StudioPlugin
{
    public class PluginLoader
    {
        private static GameObject persistentGameObject;
        public static void Load()
        {         
            Log("Loading plugin...",false);
 
            persistentGameObject = new GameObject();
            persistentGameObject.name = "AABB_PLUGIN";
            persistentGameObject.AddComponent<MainPluginComponent>();
            persistentGameObject.transform.parent = null;
            GameObject.DontDestroyOnLoad(persistentGameObject);
        }

        public static void Unload()
        {
           _Unload();
        }

        private static void _Unload()
        {
            try
            {
            }
            catch (Exception ex)
            {
                
            }  
        }

        public static void Log(string str,bool append=true)
        {
            try
            {
                using (var logWriter = new StreamWriter(Environment.GetFolderPath(Environment.SpecialFolder.Desktop) + "/log.txt", append))
                {
                    logWriter.WriteLine(str);
                }
            }
            catch (Exception)
            {
            }    
        }
    }

    public class ConsoleTextWriter : TextWriter
    {
        public override void Write(string value="")
        {
            PluginLoader.Log(value+"");
        }

        public override void WriteLine(string value="")
        {
            PluginLoader.Log(value + NewLine);
        }

        public override Encoding Encoding
        {
            get { return Encoding.Default; }
        }
    }
}
