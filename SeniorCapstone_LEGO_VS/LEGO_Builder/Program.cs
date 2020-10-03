using SharedLib;
using System;
using System.Collections.Generic;
using System.ComponentModel;

namespace LEGO_Builder
{
    class Program
    {
        //Bricks actually in game (We should get their updates)
        public static Dictionary<long, Brick> inGameBricks = new Dictionary<long, Brick>();

        //Bricks that have been detected in the last X seconds and that are not initialized
        public static Dictionary<long, Brick> emptyBricks = new Dictionary<long, Brick>();

        public static bool scanning = false;


        static void Main(string[] args)
        {
            while (true)
            {
                ProcessCommand(Console.ReadLine());
            }
        }

        public void DoWork()
        {
            //Even though we are filtering by our magic numbers on the watcher, check again
           /* if (HelperFunctions.IsAdvertisementFromABrick(args.Advertisement) && HelperFunctions.ParseBrickDataFromAdvertisement(args.Advertisement, out BrickData brickData))
            {
                if (brickData.brickID == 0) //Uninitialized brick
                {
                    HandleUninitializedBrick(brickData);
                }
                if (inGameBricks.ContainsKey(brickData.brickID))
                {
                    inGameBricks[brickData.brickID].ProcessAdvertisementUpdate(args.Advertisement);
                }
                else
                {
                    if (scanning)
                    { //It is not in game but we are scanning for new ones, so add it
                        Brick newBrick = new Brick(brickData);
                        inGameBricks.Add(brickData.brickID, newBrick);
                    }
                    else
                    { // It is not in game and we are not scanning, so we can forget about this brick
                    }
                }

                Console.WriteLine($"Got brick advertisment: " + brickData.ToString());
            }*/
        }

        static void HandleUninitializedBrick(BrickData brickData)
        {
            throw new NotImplementedException();
        }

        static void ProcessCommand(string command)
        {
            if (command.Equals("startScan"))
            {
                StartScan();
            }
            else if (command.Equals("stopScan"))
            {
                StopScan();
            }
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
