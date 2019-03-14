using Sharpmake;

[module: Sharpmake.Include("BaseShipyardExecutableProject.cs")]
[module: Sharpmake.Include("BaseShipyardLibProject.cs")]
[module: Sharpmake.Include("BaseShipyardProject.cs")]
[module: Sharpmake.Include("BaseShipyardSolution.cs")]
[module: Sharpmake.Include("ShipyardProject.cs")]
[module: Sharpmake.Include("ShipyardSolution.cs")]
[module: Sharpmake.Include("ShipyardTarget.cs")]
[module: Sharpmake.Include("ShipyardUnitTestProject.cs")]
[module: Sharpmake.Include("ShipyardUnitTestSolution.cs")]
[module: Sharpmake.Include("ShipyardUtils.cs")]
[module: Sharpmake.Include("ShipyardViewerProject.cs")]
[module: Sharpmake.Include("ShipyardViewerSolution.cs")]

namespace ShipyardSharpmake
{
    class Program
    {
        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            arguments.Generate<ShipyardSolution>();
            arguments.Generate<ShipyardViewerSolution>();

            arguments.Generate<ShipyardUnitTestSolution>();
        }   
    }
}
