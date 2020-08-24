using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace BatterySizesParser
{
    class Program
    {
        static void Main(string[] args)
        {
            System.Globalization.CultureInfo.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;
            string data = File.ReadAllText(Environment.CurrentDirectory + "/Data/11-15mm.txt");

         
            
            var match = Regex.Match(data, "<tr class=\"row-(?:.[^><]*)\" role=\"row\"><td class=\"column-1\">(?:.[^><]*)</td><td class=\"column-2\">(?:.[^><]*)</td><td class=\"column-3\">(.[^><]*)</td><td class=\"column-4\">(?:.[^><]*)</td><td class=\"column-5\">(.[^><]*)</td><td class=\"column-6\">(.[^><]*)</td><td class=\"column-7\">(.[^><]*)</td><td class=\"column-8\">(.[^><]*)</td></tr>");

            List<(string part, float capacity, float thickness, float width, float height)> batteryParsedData=new List<(string part, float capacity, float thickness, float width, float height)>();

            do
            {
                batteryParsedData.Add((match.Groups[1].Captures[0].Value, float.Parse(match.Groups[2].Captures[0].Value), float.Parse(match.Groups[3].Captures[0].Value), float.Parse(match.Groups[4].Captures[0].Value), float.Parse(match.Groups[5].Captures[0].Value)));
                match = match.NextMatch();
            } while (match.Success);
        
        }
    }
}
