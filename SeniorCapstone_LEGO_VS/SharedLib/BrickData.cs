using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SharedLib
{
    public class BrickData
    {
        public ulong brickID;
        public ulong brickType;

        public override string ToString()
        {
            return $"brickID={brickID}, brickType={brickType}";
        }
    }
}
