using Newtonsoft.Json;
using SharedLib;
using System;
using System.Collections.Generic;
using System.Text;
using WebSocketSharp;
using WebSocketSharp.Server;

namespace LEGO_Builder
{
    class WSBridgeBehavior : WebSocketBehavior
    {
        protected override void OnMessage(MessageEventArgs e)
        {
            if (e.IsText == false) return;          

            if(e.Data.Equals("ping"))
            {
                Send("pong");
                return;
            }

            if (e.Data.Contains("|") == false) return;

            switch (e.Data.Split('|')[0])
            {
                case SharedFunctions.BRIDGE_2_BUILDER_DATASTREAM:
                    var receivedData = JsonConvert.DeserializeObject<AdvertisementTransferClass>(e.Data.Substring(e.Data.IndexOf("|")+1));
                    receivedData.ParsePacket();
                    Console.WriteLine("Received stream data from bick: " + receivedData);
                    break;
                default:
                    Console.WriteLine("Found unknown BRIDGE_2_BUILDER data type!");
                    break;
            }
        }
    }
}
