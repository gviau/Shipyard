using System;
using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    public class SharpmakeProject : BaseCSharpProject
    {
        public SharpmakeProject()
            : base("shipyard.sharpmakescripts", "", new Target(Platform.anycpu, ShipyardUtils.DefaultShipyardDevEnv, Optimization.Debug))
        {
            AssemblyName = "ShipyardSharpmake";
        }

        public override void ConfigureAll(Configuration configuration, Target target)
        {
            base.ConfigureAll(configuration, target);

            configuration.ReferencesByPath.Add(@"[project.SharpmakeCsPath]\sharpmake-binaries\Sharpmake.dll");
            configuration.ReferencesByPath.Add(@"[project.SharpmakeCsPath]\sharpmake-binaries\Sharpmake.CommonPlatforms.dll");
            configuration.ReferencesByPath.Add(@"[project.SharpmakeCsPath]\sharpmake-binaries\Sharpmake.Generators.dll");
        }

        protected override string GetTargetOutputPath()
        {
            return @"\tmp\lib\";
        }
    }
}
