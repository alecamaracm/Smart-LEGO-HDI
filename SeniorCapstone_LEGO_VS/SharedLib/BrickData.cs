using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SharedLib
{
    public class BrickData
    {
        public long brickID;
        public long brickType;

        public override string ToString()
        {
            return $"brickID={brickID}, brickType={brickType}";
        }
    }
}
