using SharedLib;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Data.Xml.Dom;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;
using Windows.System.Profile;
using Windows.System.RemoteSystems;
using Windows.UI.Notifications;
using Windows.UI.Shell;
using Windows.UI.Xaml.Controls;

namespace LEGO_Bridge_WIN
{
    public static class Program
    {
        public static readonly byte[] identificationBuffer = { 0x20, 0x69, 0x21 };
        public static readonly Guid StreamerServiceGUID = new Guid("{f0006969-0451-4000-b000-000000000000}");

        static BluetoothLEAdvertisementWatcher watcher = new BluetoothLEAdvertisementWatcher();

        public static  ConcurrentDictionary<ulong,bool> workingWithMACs = new ConcurrentDictionary<ulong,bool>();

        static void Main(string[] args)
        {          
            watcher.Received += OnAdvertisementReceived;

            watcher.AdvertisementFilter.Advertisement.DataSections.Add(item: 
                       new BluetoothLEAdvertisementDataSection() { DataType=0xFF, Data = identificationBuffer.AsBuffer() });
            watcher.ScanningMode = BluetoothLEScanningMode.Passive;
            watcher.Start();

            while (true)
            {
                ProcessCommand(Console.ReadLine());                
            }
        }


        private static async void OnAdvertisementReceived(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs args)
        {
            //if (args.BluetoothAddress != 0xC464E3ACD8E6) return;
            
            //Even though we are filtering by our magic numbers on the watcher, check again
            if(IsAdvertisementFromABrick(args.Advertisement) && SharedFunctions.ParseBrickDataFromAdvertisement(args.Advertisement.ToMyData(args.BluetoothAddress), out BrickData brickData))
            {
                //We are already working with another advertisement of this brick, just ingore this one

                try
                {
                    lock (workingWithMACs)
                    {
                        if (workingWithMACs.ContainsKey(args.BluetoothAddress)) return;
                        workingWithMACs.AddOrUpdate(args.BluetoothAddress, true, (a, b) => true);
                    }

                    var device = await BluetoothLEDevice.FromBluetoothAddressAsync(args.BluetoothAddress);
                 
                    var streamerService = await device.GetGattServicesForUuidAsync(StreamerServiceGUID);
                    if (streamerService.Status == GattCommunicationStatus.Success && streamerService.Services.Count > 0)
                    {
                        var streamChar = await streamerService.Services[0].GetCharacteristicsForUuidAsync(StreamerServiceGUID);
                        if (streamChar.Status == GattCommunicationStatus.Success && streamChar.Characteristics.Count > 0)
                        {
                            var data = await streamChar.Characteristics[0].ReadValueAsync();
                            if (data.Status == GattCommunicationStatus.Success)
                            {
                                var streamedData = data.Value.ToArray();
                                Console.WriteLine("Got streamed data!");
                            }
                        }                        
                    }
                    var aaa = device.ConnectionStatus;
                    device.Dispose();
                }
                catch (Exception ex)
                {                   
                    Console.WriteLine("An error has ocurred when receiving advertisement for MAC address: " + args.BluetoothAddress+ ". Error: "+ex.Message);
                }
                finally
                {
                    lock (workingWithMACs)
                    {
                        workingWithMACs.TryRemove(args.BluetoothAddress, out bool a);
                    }
                }                
            }
            
           
        }

        public static AdvertisementData ToMyData(this BluetoothLEAdvertisement lEAdvertisement,ulong address)
        {
            return new AdvertisementData() {
                address = address,
                DataSections = lEAdvertisement.DataSections.Select(p=>p.Data.ToArray()).ToArray()
            };
        }

        public static bool IsAdvertisementFromABrick(BluetoothLEAdvertisement advertisement)
        {
            //Most useless advertisements will be dropped here
            if (advertisement.DataSections.Count != 3 || advertisement.DataSections[1]?.DataType != 255) return false;

            //Check for our magic constants
            var data = advertisement.DataSections[1].Data.ToArray();
            if (data[0] != identificationBuffer[0]
                || data[1] != identificationBuffer[1]
                || data[2] != identificationBuffer[2]) return false;

            return true;
        }




        private static void ProcessCommand(string command)
        {
            if (command.Equals("start"))
            {
                watcher.Start();
            }
            else if (command.Equals("stop"))
            {
                watcher.Stop();
            }
        }
    }
    
}
