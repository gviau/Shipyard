using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardSolution : BaseShipyardSolution
    {
        public ShipyardSolution()
            : base("Shipyard", ShipyardUtils.DefaultShipyardTargetLib)
        {
        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, Target target)
        {
            base.ConfigureAll(configuration, target);

            configuration.AddProject<ShipyardProject>(target);
        }
    }
}
