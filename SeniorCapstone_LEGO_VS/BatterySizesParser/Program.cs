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

            List<(string part, float capacity, float thickness, float width, float height)> batteryParsedData = new List<(string part, float capacity, float thickness, float width, float height)>();

            foreach (var file in Directory.GetFiles(Environment.CurrentDirectory + "/Data"))
            {
                string data = File.ReadAllText(file);
                var match = Regex.Match(data, "<tr class=\"row-(?:.[^><]*)\" role=\"row\"><td class=\"column-1\">(?:.[^><]*)</td><td class=\"column-2\">(?:.[^><]*)</td><td class=\"column-3\">(.[^><]*)</td><td class=\"column-4\">(?:.[^><]*)</td><td class=\"column-5\">(.[^><]*)</td><td class=\"column-6\">(.[^><]*)</td><td class=\"column-7\">(.[^><]*)</td><td class=\"column-8\">(.[^><]*)</td></tr>");

                do
                {
                    try
                    {
                        batteryParsedData.Add((match.Groups[1].Captures[0].Value, float.Parse(match.Groups[2].Captures[0].Value), float.Parse(match.Groups[3].Captures[0].Value.Split('~').Last()), float.Parse(match.Groups[4].Captures[0].Value), float.Parse(match.Groups[5].Captures[0].Value)));
                    }
                    catch (Exception)
                    {

                    }
                     match = match.NextMatch();
                } while (match.Success);
            }

            var thatFit4x2Duplo = batteryParsedData.Where(p => FitsRectangle(p, 48.5, 28.6) && p.thickness <= 4 && p.capacity>250).OrderByDescending(p=>p.capacity).ToList();
            var thatFit2x2Duplo = batteryParsedData.Where(p => FitsRectangle(p, 16.6, 28.6) && p.thickness <= 4  && p.capacity > 50).OrderByDescending(p => p.capacity).ToList();

            var thatFit4x2Normal = batteryParsedData.Where(p => FitsRectangle(p, 28, 12) && p.thickness <= 3  && p.capacity > 40).OrderByDescending(p => p.capacity).ToList();
            var thatFit2x2Normal = batteryParsedData.Where(p => FitsRectangle(p, 12, 13.5) && p.thickness <= 3 && p.capacity > 15).OrderByDescending(p => p.capacity).ToList();
        }

        private static bool FitsRectangle((string part, float capacity, float thickness, float width, float height) part, double side1, double side2)
        {
            return (part.width<=side1 && part.height<=side2) || (part.width <= side2 && part.height <= side1);
        }
    }
}
