using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardUnitTestProject : BaseExecutableProject
    {
        public ShipyardUnitTestProject()
            : base("ShipyardUnitTest", "../shipyard-unit-test", ShipyardUtils.DefaultShipyardTargetLib)
        {
        }
        
        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);
        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardProject>(target, ShipyardUtils.DefaultDependencySettings);
        }
    }
}
