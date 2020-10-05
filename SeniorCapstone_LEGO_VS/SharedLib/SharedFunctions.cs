using System;
using System.Collections.Generic;
using System.Text;

namespace SharedLib
{
    public static class SharedFunctions
    {
        public static readonly byte[] identificationBuffer = { 0x20, 0x69, 0x21 };
        public static readonly Guid StreamerServiceGUID = new Guid("{f0006969-0451-4000-b000-000000000000}");
        public static readonly Guid MiscServiceGUID = new Guid("00006968-0000-1000-8000-00805f9b34fb");
        public static readonly Guid MiscServiceIDChar = new Guid("f0002626-0451-4000-b000-000000000000");

        public static int BRIDGE_WS_PORT= 4259;

        public const string BRIDGE_2_BUILDER_DATASTREAM="dataStream";

        public static ulong[] DecompressLongs(byte[] compressedData)
        {
            List<ulong> toReturn = new List<ulong>();

            int pointer = 0;
            while (pointer < compressedData.Length)
            {
                ulong toAdd = 0;
                int numberLength = compressedData[pointer];
                pointer++;
                for (int i = 0; i < numberLength; i++)
                {
                    toAdd += (ulong)((ulong)compressedData[pointer] << (i * 8));
                    pointer++;
                }
                toReturn.Add(toAdd);
            }
            return toReturn.ToArray();
        }

        public static bool ParseBrickDataFromAdvertisement(AdvertisementData advertisement, out BrickData brickData)
        {
            brickData = new BrickData();

            try
            {
                ulong[] decompressedArray = DecompressLongs(advertisement.DataSections[2]);
                if (decompressedArray.Length != 2) return false;

                brickData.brickID = decompressedArray[0];
                brickData.brickType = decompressedArray[1];

                return true;
            }
            catch (Exception ex)
            {
                Console.Write("Failed to parse brick data! Error: " + ex.Message);
                return false;
            }
        }

        public static bool ParseBrickDataFromAdvertisement(object p, out BrickData brickData)
        {
            throw new NotImplementedException();
        }
    }
}
