using SharedLib;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;

namespace LEGO_Builder
{
    public class Session
    {
        /// <summary>
        /// Currently placed somewhere, the key is the brick id
        /// </summary>
        public Dictionary<long, SessionBrick> bricks = new();
       
        ulong firstBrickCacheID = ulong.MaxValue;

        public Dictionary<ulong,OldUpdateDataClass> dataFromLastUIUpdate=new();

        public Session(bool forceClean=false)
        {
            if (forceClean) CADAPIs.RemoveAll();
        }

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
                            Environment.CurrentDirectory + @"\..\..\..\BrickDefinitions\" + brickDelta.remoteBrickType + ".bdf", 
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
                        if (bricks.ContainsKey((long)brickDelta.brickData.brickID)){
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
                        }
                        break;

                    }

                case DeltaType.RotatingConnected:
                    break;
                default:
                    break;
            }
        }

        internal void ForceRecalculate()
        {
            SubmitChanges();
        }

        public void SubmitChanges()
        {
            Console.WriteLine("Recalculating bricks...");
            RecalculatePositions();
            Console.WriteLine("Uploading bricks to CAD...");
            SendStateToCAD();
        }


        private void RecalculatePositions()
        {
            foreach (var brick in bricks) brick.Value.positionCalculated = false;

            var brickList = bricks.Values.ToList();
            var firstBrick = FindFirstBrick(brickList);
            if(firstBrick==null)
            {
                return;
            }

            firstBrick.position = Vector3.Zero;
            firstBrick.rotation = 0;
            firstBrick.positionCalculated = true;

            FixPositionAndRotationRecursively(firstBrick);
        }

        private void FixPositionAndRotationRecursively(SessionBrick currentBrick)
        {
            foreach(var studsGoingToBrick in currentBrick.rxStuds.Where(p=>p.connectedTo!=null).Where(p=>!p.connectedTo.brick.positionCalculated).GroupBy(p=>p.connectedTo?.brick.brickData.brickID,p=>p))
            {
                var brickID = studsGoingToBrick.Key;
                var studs = studsGoingToBrick.ToList();
                if (studs.Count >= 2) //We are ingoring single stud connections
                {
                    var otherBrick = studs[0].connectedTo.brick;

                    var stud0Current = studs[0];
                    var stud1Current = studs[1];

                    bool foundSolution = false;
                    for (int remoteRotation = 0; remoteRotation < 360; remoteRotation += 90)
                    {
                        otherBrick.position = stud0Current.GetWorldPosition() - Vector3.Transform(stud0Current.connectedTo.brickOffset, Matrix4x4.CreateFromYawPitchRoll(0, 0, remoteRotation * Program.DEG2RAD));
                        otherBrick.rotation = remoteRotation;
                        var guessedStud1Other = stud1Current.connectedTo.GetWorldPosition();
                        if (Math.Abs((guessedStud1Other - stud1Current.GetWorldPosition()).Length())<0.1)
                        {
                            foundSolution = true;
                            otherBrick.positionCalculated = true;
                            break;
                        }
                    }

                    if (!foundSolution)
                    {
                        Console.WriteLine("[ERROR] Could not find a solution for the other brick rotation!");
                    }
                    else
                    {
                        FixPositionAndRotationRecursively(otherBrick);
                    }
                }
                else
                {
                    Console.WriteLine("[WARN] Not enough stud connections going from brick "+currentBrick.brickData.brickID +" to brick " + brickID);
                }                    
            }

            foreach (var studsGoingToBrick in currentBrick.txStuds.Where(p => p.connectedTo != null).Where(p => !p.connectedTo.brick.positionCalculated).GroupBy(p => p.connectedTo?.brick.brickData.brickID, p => p))
            {
                var brickID = studsGoingToBrick.Key;
                var studs = studsGoingToBrick.ToList();
                if (studs.Count >= 2) //We are ingoring single stud connections
                {
                    var otherBrick = studs[0].connectedTo.brick;

                    var stud0Current = studs[0];
                    var stud1Current = studs[1];

                    bool foundSolution = false;
                    for (int remoteRotation = 0; remoteRotation < 360; remoteRotation += 90)
                    {
                        otherBrick.position = stud0Current.GetWorldPosition() - Vector3.Transform(stud0Current.connectedTo.brickOffset, Matrix4x4.CreateFromYawPitchRoll(0, 0, remoteRotation * Program.DEG2RAD));
                        otherBrick.position.X = (float)Math.Round(otherBrick.position.X);
                        otherBrick.position.Y = (float)Math.Round(otherBrick.position.Y);
                        otherBrick.position.Z = (float)Math.Round(otherBrick.position.Z);

                        otherBrick.rotation = remoteRotation;
                        var guessedStud1Other = stud1Current.connectedTo.GetWorldPosition();
                        if (Math.Abs((guessedStud1Other - stud1Current.GetWorldPosition()).Length()) < 0.1)
                        {
                            foundSolution = true;
                            otherBrick.positionCalculated = true;
                            break;
                        }
                    }

                    if (!foundSolution)
                    {
                        Console.WriteLine("[ERROR] Could not find a solution for the other brick rotation!");
                    }
                    else
                    {
                        FixPositionAndRotationRecursively(otherBrick);
                    }
                }
                else
                {
                    Console.WriteLine("[WARN] Not enough stud connections going from brick " + currentBrick.brickData.brickID + " to brick " + brickID);
                }
            }
        }

        private void SendStateToCAD()
        {
            foreach (var oldData in dataFromLastUIUpdate) oldData.Value.usedInTheCurrentState = false;

            foreach(var brick in bricks)
            {
                if (brick.Value.positionCalculated)
                {
                    bool brickJustCreated = false;
                    if (!dataFromLastUIUpdate.ContainsKey((ulong)brick.Key))
                    {
                        brickJustCreated = true;
                        dataFromLastUIUpdate.Add((ulong)brick.Key, new OldUpdateDataClass());
                    }

                    var oldBrickData=dataFromLastUIUpdate[(ulong)brick.Key];

                    if (!brick.Value.position.Equals(oldBrickData.position) || brick.Value.rotation!=oldBrickData.rotation || brickJustCreated)
                    {
                        oldBrickData.position = brick.Value.position;
                        oldBrickData.rotation = brick.Value.rotation;                        
                        CADAPIs.AddBlock((int)brick.Value.brickData.brickType, (long)brick.Value.brickData.brickID, color: BrickDataProvider.GetColorForBrick(brick.Key), new Vector3(brick.Value.position.X, brick.Value.position.Z, brick.Value.position.Y), new Vector3(0, brick.Value.rotation, 0));
                    }

                    oldBrickData.usedInTheCurrentState = true;
                }
            }

            foreach(var blockNotAnymore in dataFromLastUIUpdate.Where(p => !p.Value.usedInTheCurrentState))
            {
                dataFromLastUIUpdate.Remove(blockNotAnymore.Key);
                CADAPIs.RemoveBlock((long)blockNotAnymore.Key);
            }
        }

        private SessionBrick FindFirstBrick(List<SessionBrick> brickList)
        {
            if (brickList.Count == 0) return null;

            if(firstBrickCacheID == ulong.MaxValue || !brickList.Any(p => p.brickData.brickID == firstBrickCacheID))
            {
                var brickSelected = brickList.FirstOrDefault(brick => brick.rxStuds.Where(p => p.connectedTo != null).Where(p => !p.connectedTo.brick.positionCalculated).GroupBy(p => p.connectedTo?.brick.brickData.brickID, p => p).Any(l => l.Count() >= 2)
                        || brick.txStuds.Where(p => p.connectedTo != null).Where(p => !p.connectedTo.brick.positionCalculated).GroupBy(p => p.connectedTo?.brick.brickData.brickID, p => p).Any(l => l.Count() >= 2));
                if(brickSelected == null)
                {
                    return null;
                }
                firstBrickCacheID= brickSelected.brickData.brickID;
            }

            return brickList.Find(p => p.brickData.brickID == firstBrickCacheID);
        }
    }

    public class OldUpdateDataClass
    {
        public Vector3 position=Vector3.Zero;
        public float rotation=0;
        public bool usedInTheCurrentState=false;
    }
}
