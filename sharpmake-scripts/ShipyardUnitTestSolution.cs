using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardUnitTestSolution : BaseShipyardSolution
    {
        public ShipyardUnitTestSolution()
            : base("Shipyard-Unit-Test", ShipyardUtils.DefaultShipyardTargetLib)
        {
        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.AddProject<ShipyardProject>(target);
            configuration.AddProject<ShipyardUnitTestProject>(target);
        }
    }
}
