using System;
using System.Collections.Generic;
using System.Text;

namespace SharedLib
{
    public static class SharedFunctions
    {
        public static long[] DecompressLongs(byte[] compressedData)
        {
            List<long> toReturn = new List<long>();

            int pointer = 0;
            while (pointer < compressedData.Length)
            {
                long toAdd = 0;
                int numberLength = compressedData[pointer];
                pointer++;
                for (int i = 0; i < numberLength; i++)
                {
                    toAdd += (compressedData[pointer] << (i * 8));
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
                long[] decompressedArray = DecompressLongs(advertisement.DataSections[2]);
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
    }
}
