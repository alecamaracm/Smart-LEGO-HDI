
using Android.Widget;
using LEGO_Bridge_PHONE.ViewModels;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using nexus.core;
using nexus.protocols.ble;
using nexus.protocols.ble.scan;
using nexus.protocols.ble.scan.advertisement;
using SharedLib;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using WebSocket4Net;

namespace LEGO_Bridge_PHONE
{
    public static class BLEThings
    {
        public static IBluetoothLowEnergyAdapter ble;
        static CancellationTokenSource cts;

        public static ConcurrentDictionary<ulong, bool> workingWithMACs = new ConcurrentDictionary<ulong, bool>();

        public static object initializeBrickLock = new object();
        public static bool initializeBrick = true;
        public static ulong intializeBrickWithID = 8;

        static WebSocket webSocket;

        public static InformationViewModel lastViewModel;

        public static async Task ToggleAction(InformationViewModel aboutViewModel)
        {
            lastViewModel = aboutViewModel;
            if(cts==null || cts.IsCancellationRequested==true)
            {
                aboutViewModel.ToggleText = "Stop work";
                await StartWork(aboutViewModel);
            }
            else
            {
                CancelWork();
                aboutViewModel.ToggleText = "Start work" ;
            }
        }

        public async static Task StartWork(InformationViewModel aboutViewModel)
        {
            aboutViewModel.StatusText = "Opening WS...";
            
            webSocket = new WebSocket("ws://"+aboutViewModel.ip+":"+SharedFunctions.BRIDGE_WS_PORT+"/Bridge");
         //   webSocket = new WebSocket("wss://echo.websocket.org");
            webSocket.Opened += WebSocket_Opened;
            webSocket.Closed += WebSocket_Closed;
            webSocket.MessageReceived += WebSocket_MessageReceived;
            webSocket.Error += WebSocket_Error;
            webSocket.Open();

            if (ble.AdapterCanBeEnabled && ble.CurrentState.IsDisabledOrDisabling())
            {
                await ble.EnableAdapter();
            }


            cts = new CancellationTokenSource();

            await  ble.ScanForBroadcasts(
                    new ScanSettings()
                    {                           
                        Mode = ScanMode.HighPower,                       
                        IgnoreRepeatBroadcasts = false
                    },

                    async (IBlePeripheral peripheral) =>
                    {
                        // read the advertising data
                        var adv = peripheral.Advertisement;

                        var bit8MAC = new List<byte> { 0x00, 0x00 };
                        bit8MAC.AddRange(peripheral.Address);

                        if (IsAdvertisementFromABrick(adv))
                        {
                            await DoWorkWithAdvertisement(adv, BitConverter.ToUInt64(bit8MAC.ToArray(), 0), peripheral);
                        }

                                  
                    },
                    
                    cts.Token
                );
        }



        private async static Task DoWorkWithAdvertisement(IBleAdvertisement adv, ulong address,IBlePeripheral peripheral)
        {
            //Even though we are filtering by our magic numbers on the watcher, check again
            if (SharedFunctions.ParseBrickDataFromAdvertisement(adv.ToMyData(address), out BrickData brickData))
            {
                //We are already working with another advertisement of this brick, just ingore this one

                lock (workingWithMACs)
                {
                    Console.WriteLine("Found brick!");
                    if (workingWithMACs.ContainsKey(address)) return;
                    workingWithMACs.AddOrUpdate(address, true, (a, b) => true);
                }

                try
                {
                    if (brickData.brickID == 0)
                    { //Uninitialized brick

                        if (initializeBrick)
                        { //Let's initialize a new brick!
                            initializeBrick = false;

                            byte[] array=new byte[8];
                            for(int i=0;i<8;i++)
                            {
                                array[i] = (byte)(intializeBrickWithID >> (8 * i));
                            }

                            var connection = await ble.ConnectToDevice(peripheral, TimeSpan.FromSeconds(5), progress => Debug.WriteLine(progress));

                            var data = await connection.GattServer.WriteCharacteristicValue(SharedFunctions.MiscServiceGUID, SharedFunctions.MiscServiceIDChar, array);
                            Console.WriteLine("Brick initialized!");

                        }
                        else
                        {
                            //BridgeNetwork.GotUninitializedBrick(brickData);
                        }
                    }
                    else
                    {
                        try
                        {
                            var connection = await ble.ConnectToDevice(peripheral, TimeSpan.FromSeconds(5), progress => Debug.WriteLine(progress));
                            var data = await connection.GattServer.ReadCharacteristicValue(SharedFunctions.StreamerServiceGUID, SharedFunctions.StreamerServiceGUID);

                            if(data.Length>0)
                            {
                                var toSend = new AdvertisementTransferClass()
                                {
                                    brickData = brickData,
                                    dataStream = data,
                                };

                                string jsonToSend=JsonConvert.SerializeObject(toSend);
                                if (webSocket != null && webSocket.State == WebSocketState.Open) webSocket.Send(SharedFunctions.BRIDGE_2_BUILDER_DATASTREAM+"|" + jsonToSend);
                                Console.WriteLine("Got streamed data for brick: "+brickData.ToString());
                            }
                            else
                            {
                                Console.WriteLine("!!! Got empty streamed data for brick: " + brickData.ToString());
                            }
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("!!! Failed to get stream data. Error: " + ex.Message);
                            throw;
                        }                     
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine("An error has ocurred when receiving advertisement for MAC address: " + address + ". Error: " + ex.Message);
                }
                finally
                {
                    lock (workingWithMACs)
                    {
                        workingWithMACs.TryRemove(address, out bool a);
                    }
                }
            }
        }

        public static AdvertisementData ToMyData(this IBleAdvertisement lEAdvertisement, ulong address)
        {
            return new AdvertisementData()
            {
                address = address,
                DataSections = lEAdvertisement.RawData.Select(p => p.Data).ToArray()
            };
        }

        private static void WebSocket_Error(object sender, SuperSocket.ClientEngine.ErrorEventArgs e)
        {
            Console.WriteLine("Error!");
            lastViewModel.StatusText = "WS error!";
        }

        private static void WebSocket_MessageReceived(object sender, MessageReceivedEventArgs e)
        {
            if(e.Message=="pong")
            {
                lastViewModel.StatusText = "Connected";
            }
            Console.WriteLine("Got message: "+e.Message);
        }

        private static void WebSocket_Closed(object sender, EventArgs e)
        {
            lastViewModel.StatusText = "WS closed";
            Console.WriteLine("Socket closed!");
        }

        private static void WebSocket_Opened(object sender, EventArgs e)
        {
            webSocket.Send("ping");
            Console.WriteLine("Socket opened!");
        }

        public static void CancelWork()
        {
            if (cts != null) cts.Cancel();
            if (webSocket != null && webSocket.State == WebSocketState.Open) webSocket.Close();
        }

        public static bool IsAdvertisementFromABrick(IBleAdvertisement advertisement)
        {
            //Most useless advertisements will be dropped here
            var identificators=advertisement.RawData.Where(predicate => (int)predicate.Type == 254);
            if (identificators.Any()==false) return false;
       //     if (advertisement.RawData.Count() < 2 || (byte)advertisement.RawData.ElementAt(1).Type != 254) return false;

            //Check for our magic constants
            var data = identificators.First().Data;
            if (data[0] != SharedFunctions.identificationBuffer[0]
                || data[1] != SharedFunctions.identificationBuffer[1]
                || data[2] != SharedFunctions.identificationBuffer[2]) return false;

            return true;
        }

        public static void Init(IBluetoothLowEnergyAdapter bluetoothLowEnergyAdapter)
        {
            ble = bluetoothLowEnergyAdapter;
        }
    }

}
