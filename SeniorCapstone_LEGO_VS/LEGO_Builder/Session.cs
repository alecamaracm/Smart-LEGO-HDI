using SharedLib;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace LEGO_Builder
{
    public class Session
    {
        /// <summary>
        /// Currently placed somewhere, the key is the brick id
        /// </summary>
        public Dictionary<long, SessionBrick> bricks = new();

        /// <summary>
        /// Process a delta in the session
        /// </summary>
        /// <param name="brickDelta"></param>
        public void InputNewDelta(BrickDelta brickDelta)
        {
            switch (brickDelta.deltaType)
            {
                case DeltaType.BasicConnected:
                    {
                        //Makes sure that the brick exists in the current sessions. Otherwise it adds it
                        if (!bricks.ContainsKey((long)brickDelta.brickData.brickID)) bricks.Add((long)brickDelta.brickData.brickID, 
                            new SessionBrick(brickDelta.brickData, 
                            Environment.CurrentDirectory + @"\..\..\..\BrickDefinitions\" + brickDelta.brickData.brickType + ".bdf", 
                            this));

                        //Makes sure that the remote brick exists in the current sessions. Otherwise it adds it
                        if (!bricks.ContainsKey(brickDelta.destinationMAC)) bricks.Add(brickDelta.destinationMAC, 
                            new SessionBrick(new BrickData() { brickID = (ulong)brickDelta.destinationMAC, brickType = (ulong)brickDelta.remoteBrickType }, 
                            Environment.CurrentDirectory + @"\..\..\..\BrickDefinitions\" + brickDelta.brickData.brickType + ".bdf", 
                            this));

                        var rxBrick = bricks[(long)brickDelta.brickData.brickID];
                        var txBrick = bricks[brickDelta.destinationMAC];

                        var rxStud = rxBrick.rxStuds.First(stud => stud.studID == brickDelta.localStud);
                        var txStud = txBrick.txStuds.First(stud => stud.studID == brickDelta.remoteStud);

                        //Cleanup the connection the other way too
                        if (rxStud.connectedTo != null)
                        {
                            rxStud.connectedTo.connectedTo = null;   //Remove old coonection if it was connected
                                                                     //Clean the brick if it is not used anymore
                            rxStud.connectedTo.brick.PerformCleanupIfNeccessary();
                        }
                        rxStud.connectedTo = txStud; //Set the new connection

                        //Cleanup the connection the other way too
                        if (txStud.connectedTo != null)
                        {
                            txStud.connectedTo.connectedTo = null;   //Remove old coonection if it was connected  
                                                                     //Clean the brick if it is not used anymore
                            txStud.connectedTo.brick.PerformCleanupIfNeccessary();
                        }
                        txStud.connectedTo = rxStud; //Set the new connection
                        break;
                    }

                case DeltaType.BasicDisconnected:
                    {
                        var rxBrick = bricks[(long)brickDelta.brickData.brickID];
                        var rxStud = rxBrick.rxStuds.First(stud => stud.studID == brickDelta.localStud);

                        //Cleanup the connection the other way too
                        if (rxStud.connectedTo != null)
                        {
                            rxStud.connectedTo.connectedTo = null;
                            rxStud.connectedTo.brick.PerformCleanupIfNeccessary();
                        }
                        rxStud.connectedTo = null;
                        rxStud.brick.PerformCleanupIfNeccessary();
                        break;
                    }

                case DeltaType.RotatingConnected:
                    break;
                default:
                    break;
            }
        }

    }


}
