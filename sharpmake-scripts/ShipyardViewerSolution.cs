using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardViewerSolution : BaseSolution
    {
        public ShipyardViewerSolution()
            : base("shipyard.viewer", ShipyardUtils.DefaultShipyardTargetDll)
        {

        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.AddProject<ShipyardViewerProject>(target);
        }
    }
}
