using System;
using System.Collections.Generic;

namespace LEGO_Builder
{
    internal class BrickDataProvider
    {
        public static Dictionary<long, string> generatedColors = new Dictionary<long, string>();

        public static List<string> colorsThatCanBeUsed = new() { "FF0000","00FF00","0000FF","FFFF00","FFFFFF","00FFFF","FF00FF"};
        public static int lastColorIndex = -1;

        public static string GetColorForBrick(long brickID)
        {
            if (generatedColors.ContainsKey(brickID)) return generatedColors[brickID];
            generatedColors.Add(brickID, colorsThatCanBeUsed[(++lastColorIndex) % colorsThatCanBeUsed.Count]);
            return generatedColors[brickID];
        }
    }
}