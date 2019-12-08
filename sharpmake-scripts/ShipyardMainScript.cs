using Sharpmake;

[module: Sharpmake.Include("BaseCSharpProject.cs")]
[module: Sharpmake.Include("BaseCSharpSolution.cs")]
[module: Sharpmake.Include("BaseShipyardExecutableProject.cs")]
[module: Sharpmake.Include("BaseShipyardLibProject.cs")]
[module: Sharpmake.Include("BaseShipyardProject.cs")]
[module: Sharpmake.Include("BaseShipyardSolution.cs")]
[module: Sharpmake.Include("ShipyardUtils.cs")]
[module: Sharpmake.Include("SharpmakeProject.cs")]
[module: Sharpmake.Include("SharpmakeSolution.cs")]
[module: Sharpmake.Include("ShipyardProject.cs")]
[module: Sharpmake.Include("ShipyardSolution.cs")]
[module: Sharpmake.Include("ShipyardTarget.cs")]
[module: Sharpmake.Include("ShipyardToolsProject.cs")]
[module: Sharpmake.Include("ShipyardUnitTestProject.cs")]
[module: Sharpmake.Include("ShipyardUnitTestSolution.cs")]
[module: Sharpmake.Include("ShipyardViewerProject.cs")]
[module: Sharpmake.Include("ShipyardViewerSolution.cs")]

namespace ShipyardSharpmake
{
    class ShipyardMainScript
    {
        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            arguments.Generate<ShipyardSolution>();
            arguments.Generate<ShipyardViewerSolution>();

            arguments.Generate<ShipyardUnitTestSolution>();

            arguments.Generate<SharpmakeSolution>();
        }   
    }
}
