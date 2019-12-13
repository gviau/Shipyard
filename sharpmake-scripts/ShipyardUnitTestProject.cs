using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardUnitTestProject : BaseExecutableProject
    {
        public ShipyardUnitTestProject()
            : base("shipyard.unittests", @"..\shipyard-unit-test\", ShipyardUtils.DefaultShipyardTargetLib)
        {
        }
        
        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.ForcedIncludes.Add("shipyardunittestprecomp.h");
            configuration.PrecompHeader = "shipyardunittestprecomp.h";
            configuration.PrecompSource = "shipyardunittestprecomp.cpp";
        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardSystemProject>(target, ShipyardUtils.DefaultDependencySettings);
            configuration.AddPublicDependency<ShipyardMathProject>(target, ShipyardUtils.DefaultDependencySettings);
            configuration.AddPublicDependency<ShipyardGraphicsProject>(target, ShipyardUtils.DefaultDependencySettings);
        }

        protected override void ConfigureIncludePaths(Configuration configuration)
        {
            base.ConfigureIncludePaths(configuration);

            configuration.IncludePrivatePaths.Add(SourceRootPath);
        }
    }
}
