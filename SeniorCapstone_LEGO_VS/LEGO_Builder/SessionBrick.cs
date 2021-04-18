using SharedLib;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Numerics;

namespace LEGO_Builder
{
    public class SessionBrick
    {
        public Session Session;

        public BrickData brickData;

        //Top studs
        public List<BrickStud> txStuds = new();

        //Bottom sutds
        public List<BrickStud> rxStuds = new();

        public Vector3 position = Vector3.Zero;
        public float rotation = 0;
        public bool positionCalculated = false;


        public SessionBrick(BrickData _brickData, string brickDefinitionFile, Session session)
        {
            Session = session;
            brickData = _brickData;

            CultureInfo.CurrentCulture = CultureInfo.InvariantCulture;

            using (StreamReader reader=new StreamReader(brickDefinitionFile))
            {
                while (reader.ReadLine() != "txStuds") { }

                while(true)
                { //Read all the txStuds
                    var line = reader.ReadLine();
                    if (line == "rxStuds") break;
                    txStuds.Add(parseBrickStud(line));
                }

                while (!reader.EndOfStream)
                { //Read all the txStuds
                    rxStuds.Add(parseBrickStud(reader.ReadLine()));
                }
            }    
        }

        public BrickStud parseBrickStud(string line)
        {
            int studId = int.Parse(line.Split(':')[0]);
            List<float> studCoordinates = line.Split(':')[1].Split(' ').Select(p => float.Parse(p)).ToList();
            return new BrickStud(studId, new System.Numerics.Vector3(studCoordinates[0], studCoordinates[1], studCoordinates[2]), this);
        }

       

        /// <summary>
        /// Removes itself from the session if not connected to anything
        /// </summary>
        public void PerformCleanupIfNeccessary()
        {
            if (!IsConnectedToSomething()) Session.bricks.Remove((long)brickData.brickID); 
        }


        /// <summary>
        /// Returns whether this brick is connected to at least another brick
        /// </summary>
        /// <returns></returns>
        private bool IsConnectedToSomething()
        {
            if (txStuds.Any(stud => stud.connectedTo != null)) return true;
            if (rxStuds.Any(stud => stud.connectedTo != null)) return true;
            return false;
        }
    }

    public class BrickDefinitionFile
    {

    }
}