using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace LEGO_Builder
{
    public static class CADAPIs
    {
        public const string address = "127.0.0.1";
        public const int port = 6971;

        public static void AddBlock(int brickTypeID, long systemID,string color,Vector3 position,Vector3 rotation,bool async=true)
        {
            if (color.StartsWith("%23") == false) color = "%23" + color;

            WebClient webClient = new WebClient();
            webClient.Proxy = null;
            string response=webClient.UploadString("http://"+address+":"+port+ "/addBrick?brickTypeID="+brickTypeID+ "&systemID=" + systemID+"&color="+color+
                "&x="+position.X+ "&y=" + position.Y +"&z=" + position.Z + "&rx=" + rotation.X + "&ry=" + rotation.Y + "&rz=" + rotation.Z, "");
        }

        public static void RemoveBlock(long systemID)
        {
            WebClient webClient = new WebClient();
            string response = webClient.UploadString("http://" + address + ":" + port + "/removeBrick?systemID=" + systemID, "");
        }

        public static void RemoveAll()
        {
            WebClient webClient = new WebClient();
            string response = webClient.UploadString("http://" + address + ":" + port + "/removeAll", "");
        }
    }
}
