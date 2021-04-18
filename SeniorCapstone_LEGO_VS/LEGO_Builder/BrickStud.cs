using System.Numerics;

namespace LEGO_Builder
{
    public class BrickStud
    {
        public BrickStud(int _studId, Vector3 _brickOffset, SessionBrick _ownerBrick)
        {
            studID = _studId;
            brickOffset = _brickOffset;
            brick = _ownerBrick;
        }

        public int studID;
        public SessionBrick brick;

        public Vector3 brickOffset;
        
        //Null if not connected
        public BrickStud connectedTo;

        public bool IsConnectedToABrick()
        {
            return connectedTo != null;
        }

        public Vector3 GetWorldPosition()
        {
            return brick.position + Vector3.Transform(brickOffset, Matrix4x4.CreateFromYawPitchRoll(0, 0, brick.rotation * Program.DEG2RAD));
        }

    }
}