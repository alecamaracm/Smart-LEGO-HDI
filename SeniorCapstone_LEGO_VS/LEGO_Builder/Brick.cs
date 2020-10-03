using SharedLib;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LEGO_Builder
{
    class Brick
    {
        BrickData brickData;

        public Brick(BrickData brickData)
        {
            this.brickData = brickData;
        }

        internal void ProcessAdvertisementUpdate(AdvertisementData advertisement)
        {
            throw new NotImplementedException();
        }
    }
}
