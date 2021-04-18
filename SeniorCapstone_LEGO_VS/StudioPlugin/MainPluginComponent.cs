using System;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace StudioPlugin
{
    public class MainPluginComponent:MonoBehaviour
    {
        Thread backgroundThread;    

        HttpListener listener;
        int port = 6971;

        Dictionary<LDrawPart, CustomBrickDataComponent> customBrickData = new Dictionary<LDrawPart, CustomBrickDataComponent>();

        public void Start()
        {
            backgroundThread = new Thread(DoWork);
            backgroundThread.IsBackground = true;
            backgroundThread.Start();
        }
        public void Update()
        {
       

        }
        public void OnGUI()
        {        
          
        }

        public void OnDestroy()
        {
           // tcpListener.Stop();
        }

        void DoWork()
        {
            try
            {
                listener = new HttpListener();
                listener.Prefixes.Add("http://localhost:"+port+"/");
                listener.Prefixes.Add("http://127.0.0.1:" + port + "/");
                listener.Start();
            }
            catch (System.Exception)
            {
                PluginLoader.Log("An error has ocurrend when initializing the HTTPListener!");
                return;
            }
            BLStudio.Instance.onRemovePrimitivie += primitiveRemoved;
            BLStudio.Instance.onModifiedPart += PartModified;

            while (true)
            {
                HttpListenerContext ctx = listener.GetContext();
                // ListenOnce(ctx);
                //StartCoroutine("ScketchyFix", ctx);
                DoWorkWithListener(ctx);
                Thread.Sleep(25);
            }
        }

        IEnumerator ScketchyFix(object arg)
        {
            HttpListenerContext ctx = (HttpListenerContext)arg;
            DoWorkWithListener(ctx);
            yield return null;
        }

        private void DoWorkWithListener(HttpListenerContext ctx)
        {
            try
            {                

                HttpListenerRequest req = ctx.Request;
                HttpListenerResponse resp = ctx.Response;

                using (StreamWriter writer = new StreamWriter(resp.OutputStream))
                {
                    switch (req.HttpMethod)
                    {
                        case "GET":
                            switch (req.Url.AbsolutePath)
                            {
                                case "/getBricks":
                                    ProcessGetBricksRequest(req, resp);
                                    break;
                                default:
                                    resp.StatusCode = 404;
                                    break;
                            }
                            break;
                        case "POST":
                            switch (req.Url.AbsolutePath)
                            {
                                case "/addBrick":
                                    ProcessAddBrickRequest(req, resp);
                                    break;
                                case "/removeBrick":
                                    ProcessRemoveBrickRequest(req, resp);
                                    break;
                                case "/removeAll":
                                    ProcessRemoveAllRequest(req, resp);
                                    break;
                                default:
                                    resp.StatusCode = 404;
                                    break;
                            }
                            break;
                        default:
                            resp.StatusCode = 404;
                            break;
                    }


                    resp.Close();
                }


            }
            catch (System.Exception ex)
            {
                try
                {
                    PluginLoader.Log("An error has ocurrend in the HTTPListener while loop! Error: " + ex.Message);
                }
                catch (System.Exception) { }
            }
        }

        private void ProcessRemoveAllRequest(HttpListenerRequest req, HttpListenerResponse resp)
        {
            PluginLoader.Log("Processing remove brick request...");     

            int removals = 0;

            foreach (var brick in customBrickData.Select(p => p.Key).ToList())
            {
                if (customBrickData.ContainsKey(brick)) customBrickData.Remove(brick);
                BLStudio.Instance.removeBrick(brick);
                removals++;
            }

            resp.StatusCode = 200;
            resp.OutputStream.WriteString("Removed " + removals + " bricks.");
        }

        private void primitiveRemoved(LDrawStep step, LDrawPrimitive part)
        {
            if (!(part is LDrawPart)) return;

            if(customBrickData.ContainsKey(part as LDrawPart))
            {
                PluginLoader.Log("Detected a part removal from the UI. Removing also our extra data reference.");
                customBrickData.Remove(part as LDrawPart);
            }
        }

        private void ProcessGetBricksRequest(HttpListenerRequest req, HttpListenerResponse resp)
        {
            PluginLoader.Log("Processing brick GET request...");

            resp.StatusCode = 200;

            foreach (var brick in customBrickData)
            {
                resp.OutputStream.WriteString($"{brick.Key.PartName}-[{brick.Value.myID}]"+Environment.NewLine);
            }



            
        }

        private void PartModified(LDrawPrimitive part)
        {
            PluginLoader.Log("part modified!");
            if (!(part is LDrawPart)) return;

            LDrawPart modifiedPart = part as LDrawPart;

            if (customBrickData.ContainsKey(modifiedPart) == false) return;

            PluginLoader.Log("Part modified! " + customBrickData[modifiedPart].myID);
        }

        private bool CheckForQueryParamExistance(string param, HttpListenerRequest req, HttpListenerResponse resp)
        {
            if(req.QueryString[param]==null)
            {
                resp.StatusCode = 500;
                resp.OutputStream.WriteString("Missing " + param + "!");
                
                return false;
            }
            else
            {
                return true;
            }

        }

        private void ProcessRemoveBrickRequest(HttpListenerRequest req, HttpListenerResponse resp)
        {
            PluginLoader.Log("Processing remove brick request...");

            if (CheckForQueryParamExistance("systemID", req, resp) == false) return;

            long systemID = 0;
            if (long.TryParse(req.QueryString["systemID"], out systemID) == false)
            {
                resp.StatusCode = 500;
                resp.OutputStream.WriteString("systemID is not a long!");

                return;
            }

            var bricksWithID = customBrickData.Where(p => p.Value.myID == systemID).Select(p=>p.Key);
            int removals = 0;

            foreach (var brick in bricksWithID.ToList())
            {
                if (customBrickData.ContainsKey(brick)) customBrickData.Remove(brick);
                BLStudio.Instance.removeBrick(brick);
                removals++;
            }
        
            resp.StatusCode = 200;
            resp.OutputStream.WriteString("Removed " +removals+ " bricks.");
        }

        private void ProcessAddBrickRequest(HttpListenerRequest req, HttpListenerResponse resp)
        {
            resp.StatusCode = 200;

            PluginLoader.Log("Processing add brick request...");

            if (CheckForQueryParamExistance("brickTypeID", req, resp) == false) return;
            if (CheckForQueryParamExistance("systemID", req, resp) == false) return;
            if (CheckForQueryParamExistance("color", req, resp) == false) return;
            if (CheckForQueryParamExistance("x", req, resp) == false) return;
            if (CheckForQueryParamExistance("y", req, resp) == false) return;
            if (CheckForQueryParamExistance("z", req, resp) == false) return;
            if (CheckForQueryParamExistance("rx", req, resp) == false) return;
            if (CheckForQueryParamExistance("ry", req, resp) == false) return;
            if (CheckForQueryParamExistance("rz", req, resp) == false) return;

            long systemID = 0;
            if (long.TryParse(req.QueryString["systemID"], out systemID) == false)
            {
                resp.StatusCode = 500;
                resp.OutputStream.WriteString("systemID is not a long!");

                return;
            }

            var exisitngPartsWithTheID = customBrickData.Where(p => p.Value.myID == systemID);
            if(exisitngPartsWithTheID.Any())
            {
                int removals = 0;
                foreach (var brick in exisitngPartsWithTheID.Select(p => p.Key).ToList())
                {
                    if (customBrickData.ContainsKey(brick)) customBrickData.Remove(brick);
                    BLStudio.Instance.removeBrick(brick);
                    removals++;
                }
                resp.OutputStream.WriteString("Removed " + removals + " bricks on brick add");
            }

            var partToAdd = new LDrawPart(req.QueryString["brickTypeID"] + ".dat");
            if (partToAdd == null)
            {
                resp.StatusCode = 500;
                resp.OutputStream.WriteString("brickTypeID not found!");
                
                return;
            }               

            Color newColor;
            if (ColorUtility.TryParseHtmlString(req.QueryString["color"], out newColor) == false)
            {
                resp.StatusCode = 500;
                resp.OutputStream.WriteString("color is not a color!");
            
                return;
            }

            Vector3 newBrickPosition = new Vector3(
                float.Parse(req.QueryString["x"]),
                float.Parse(req.QueryString["y"]),
                float.Parse(req.QueryString["z"]));

            Vector3 newBrickRotation = new Vector3(
               float.Parse(req.QueryString["rx"]),
               float.Parse(req.QueryString["ry"]),
               float.Parse(req.QueryString["rz"]));

            partToAdd.TransformMatrix = Matrix4x4.TRS(newBrickPosition, Quaternion.Euler(newBrickRotation), Vector3.one);        
            BLStudio.Instance.changeBrickColor(partToAdd, ColorLibrary.Instance.getSimilarColorCodeForColor_RGB(newColor));
            BLStudio.Instance.addBrick(partToAdd);

            customBrickData.Add(partToAdd, new CustomBrickDataComponent() { myID=systemID });  
        }
    }
}