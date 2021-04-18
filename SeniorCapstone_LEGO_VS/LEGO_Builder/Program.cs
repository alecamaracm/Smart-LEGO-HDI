using SharedLib;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading.Tasks;
using WebSocketSharp.Server;

namespace LEGO_Builder
{
    class Program
    {
        public const float DEG2RAD = (float)Math.PI / 180;
        public const float RAD2DEG = 180 / (float)Math.PI;


        //Bricks actually in game (We should get their updates)
        public static Dictionary<long, Brick> inGameBricks = new Dictionary<long, Brick>();

        //Bricks that have been detected in the last X seconds and that are not initialized
        public static Dictionary<long, Brick> emptyBricks = new Dictionary<long, Brick>();

        public static bool scanning = false;

        public static Session mainSession;

        public static WebSocketServer wss;

        static void Main(string[] args)
        {
            mainSession = new Session();
               

            wss = new WebSocketServer(SharedFunctions.BRIDGE_WS_PORT);
            wss.AddWebSocketService<WSBridgeBehavior>("/Bridge");
            wss.Start();

            Console.WriteLine("Web socket started in port " + SharedFunctions.BRIDGE_WS_PORT);

            while (true)
            {
                ProcessCommand(Console.ReadLine());
            }           
        }


        static void ProcessCommand(string command)
        {
            if (command.Equals("recalculate"))
            {
                mainSession.SubmitChanges();
            }
            if (command.Equals("startScan"))
            {
                StartScan();
            }else if (command.Equals("superTest"))
            {
                Random random = new Random();
                int count =10;
                for(int x = 0; x < count;x ++)
                {
                    for (int y = 0; y < count; y ++)
                    {
                        for (int z = 0; z < count; z ++)
                        {

                                CADAPIs.AddBlock(3001, x * 100 + y * 10 + z, "%23" + random.Next(254, 255).ToString("X") + random.Next(254, 255).ToString("X") + random.Next(254, 255).ToString("X"), new System.Numerics.Vector3(x * 80, y * 24, z * 40), new System.Numerics.Vector3(0, 0, 0));

                          //   CADAPIs.RemoveBlock(x * 100 + y * 10 + z);
                        }
                    }
                }
                CADAPIs.RemoveAll();
            }
            else if (command.Equals("stopScan"))
            {
                StopScan();
            } else if (command.Equals("test1"))
            {
                DoTest1();
            }
            else if (command.Equals("-test1"))
            {
                DoTestMinus1();
                
            } else if (command.Equals("stress"))
            {
                Stopwatch watch = new();
                long oldMemory = GC.GetTotalMemory(true);
                watch.Start();
                for(int i = 0; i < 100000; i++)
                {
                    DoTest1();
                    DoTestMinus1();
                }
                watch.Stop();

                long newMemory = GC.GetTotalMemory(true);

                Console.WriteLine($"100000 iterations done in {watch.ElapsedMilliseconds}. Memory delta: {newMemory-oldMemory}");

            }
        }

        private static void DoTestMinus1()
        {
            //Basic test where a brick is directly on top of another
            var stream = new AdvertisementTransferClass();
            stream.brickData = new BrickData() { brickID = 1, brickType = 3001 };
            stream.brickDeltas.Add(new BrickDelta(stream.brickData)
            {
                brickData = stream.brickData,
                //destinationMAC = 2, //Not sure if we are sending this
                deltaType = DeltaType.BasicDisconnected,
                localStud = 0,
                //remoteBrickType = 3001,  //Not sure if we are sending this
                //remoteStud = 0  //Not sure if we are sending this
            });
            stream.brickDeltas.Add(new BrickDelta(stream.brickData)
            {
                brickData = stream.brickData,
                //destinationMAC = 2,  //Not sure if we are sending this
                deltaType = DeltaType.BasicDisconnected,
                localStud = 1,
                //remoteBrickType = 3001,  //Not sure if we are sending this
                //remoteStud = 1  //Not sure if we are sending this
            });
            foreach (var delta in stream.brickDeltas)
            {
                mainSession.InputNewDelta(delta);
            }
            mainSession.SubmitChanges();
        }

        private static void DoTest1()
        {
            //Basic test where a brick is directly on top of another
            var stream = new AdvertisementTransferClass();
            stream.brickData = new BrickData() { brickID = 1, brickType = 3001 };
            stream.brickDeltas.Add(new BrickDelta(stream.brickData)
            {
                brickData = stream.brickData,
                destinationMAC = 2,
                deltaType = DeltaType.BasicConnected,
                localStud = 0,
                remoteBrickType = 3001,
                remoteStud = 4
            });
            stream.brickDeltas.Add(new BrickDelta(stream.brickData)
            {
                brickData = stream.brickData,
                destinationMAC = 2,
                deltaType = DeltaType.BasicConnected,
                localStud = 4,
                remoteBrickType = 3001,
                remoteStud = 5
            });
            foreach (var delta in stream.brickDeltas)
            {
                mainSession.InputNewDelta(delta);
            }
            mainSession.SubmitChanges();
        }

        static void StopScan()
        {
            scanning = false;
        }

        static void StartScan()
        {
            scanning = true;
        }
    }
}
