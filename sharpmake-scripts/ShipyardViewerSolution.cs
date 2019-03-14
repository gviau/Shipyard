using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardViewerSolution : BaseShipyardSolution
    {
        public ShipyardViewerSolution()
            : base("Shipyard-Viewer", ShipyardUtils.DefaultShipyardTargetLib)
        {

        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.AddProject<ShipyardProject>(target);
            configuration.AddProject<ShipyardViewerProject>(target);
        }
    }
}
