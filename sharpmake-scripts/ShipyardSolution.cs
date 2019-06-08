using Sharpmake;

namespace ShipyardSharpmake
{
    class BaseShipyardSolution : BaseSolution
    {
        public BaseShipyardSolution(ShipyardTarget shipyardTarget)
            : base("Shipyard", shipyardTarget)
        {
        }
    }

    [Generate]
    class ShipyardDllSolution : BaseShipyardSolution
    {
        public ShipyardDllSolution()
            : base(ShipyardUtils.DefaultShipyardTargetDll)
        {
        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.AddProject<ShipyardDllProject>(target);
        }
    }

    [Generate]
    class ShipyardSolution : BaseShipyardSolution
    {
        public ShipyardSolution()
            : base(ShipyardUtils.DefaultShipyardTargetLib)
        {
        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.AddProject<ShipyardProject>(target);
        }
    }
}
