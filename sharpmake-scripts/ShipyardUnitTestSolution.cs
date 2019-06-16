using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardUnitTestSolution : BaseSolution
    {
        public ShipyardUnitTestSolution()
            : base("shipyard.unittests", ShipyardUtils.DefaultShipyardTargetLib)
        {
        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.AddProject<ShipyardSystemProject>(target);
            configuration.AddProject<ShipyardUnitTestProject>(target);
        }
    }
}
