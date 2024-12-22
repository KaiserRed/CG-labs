using OpenTK.Windowing.Desktop;

namespace BezierCurve
{
    class Program
    {
        static void Main(string[] args)
        {
            using (Game game = new Game(800, 800, "Quadratic Bézier Curve"))
            {
                game.Run();
            }
        }
    }
}