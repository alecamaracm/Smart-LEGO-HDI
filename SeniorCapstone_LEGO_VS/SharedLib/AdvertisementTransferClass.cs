using System;
using System.Collections.Generic;
using System.Text;

namespace SharedLib
{
    public class AdvertisementTransferClass
    {
        public BrickData brickData;
        public byte[] dataStream;

        public List<BrickDelta> brickDeltas;

        public bool ParsePacket()
        {
            if (dataStream.Length==0)
            {
                Console.WriteLine("dataStream to parse is empty!");
                return false;
            }

            brickDeltas=new List<BrickDelta>();

            List<byte> dataStreamList = new List<byte>(dataStream.Length);
            foreach (byte bytee in dataStream) dataStreamList.Add(bytee);

            while(dataStreamList.Count>0)
            {
                BrickDelta delta = new BrickDelta();
                delta.deltaType = dataStreamList[0];

                int deltaLength = 0;

                switch(delta.deltaType)
                {
                    case 0: //Stud chance (Connected or replaced)
                        var macBytes = dataStreamList.GetRange(1, 6);
                        macBytes.AddRange(new byte[] { 0x00, 0x00 }); //Padd the 6 bytes into the 8 for the long                         
                        delta.destinationMAC = BitConverter.ToInt64(macBytes.ToArray(), 0);

                        var studBytes = dataStreamList.GetRange(7, 3);
                        studBytes.InsertRange(0, new byte[] { 0x00 }); //Padd the 3 bytes into the 4 for the int 
                        studBytes.Reverse(); //We are sending MSByte first but this system is little endian
                        delta.localStud = BitConverter.ToInt32(studBytes.ToArray(), 0);

                        studBytes = dataStreamList.GetRange(10, 3);
                        studBytes.InsertRange(0, new byte[] { 0x00 }); //Padd the 3 bytes into the 4 for the int 
                        studBytes.Reverse(); //We are sending MSByte first but this system is little endian
                        delta.remoteStud = BitConverter.ToInt32(studBytes.ToArray(), 0);

                        deltaLength = 13;
                        break;
                    case 1: //Stud disconnected
                        var studBytes2 = dataStreamList.GetRange(1, 3);
                        studBytes2.InsertRange(0, new byte[] { 0x00 }); //Padd the 3 bytes into the 4 for the int 
                        studBytes2.Reverse(); //We are sending MSByte first but this system is little endian
                        delta.localStud = BitConverter.ToInt32(studBytes2.ToArray(), 0);

                        deltaLength = 4;
                        break;
                    case 2:  //Rotating stud change or add                       
                        throw new NotImplementedException("Rotation deltas not implemented yet!");
                    default:
                        throw new NotImplementedException("Unknown delta type!");
                }

                dataStreamList.RemoveRange(0, deltaLength);

                brickDeltas.Add(delta);
            }

            return true;
        }

        public override string ToString()
        {
            string toReturn = "";
            toReturn += "Stream packet from: " + brickData?.ToString() + Environment.NewLine;
            brickDeltas?.ForEach(p => toReturn += p.ToString() + Environment.NewLine);

            foreach(var bytee in dataStream)
            {
                toReturn += bytee + "\t";
            }
            toReturn += Environment.NewLine;

            return toReturn;
        }
    }



    public class BrickDelta
    {
        public byte deltaType=0xFF;
        public long destinationMAC;
        public int localStud;
        public int remoteStud;

        public float[] rotations;

        public override string ToString()
        {
            switch (deltaType)
            {
                case 0: //Basic stud connection
                    return $"New basic stud connected. LocalStud={localStud}, RemoteMAC={destinationMAC.ToString("X6")}, RemoteStud={remoteStud}";
                case 1: //Basic stud connection
                    return $"Stud disconnected. LocalStud={localStud}";
                case 2: //Rotating stud connection
                    return $"Rotating stud connected.";
                default:
                    return "Unknown brick delta!";
            }
        }
    }
}
