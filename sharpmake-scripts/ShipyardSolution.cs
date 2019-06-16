using Sharpmake;

namespace ShipyardSharpmake
{
    class BaseShipyardSolution : BaseSolution
    {
        public BaseShipyardSolution(string solutionName, ShipyardTarget shipyardTarget)
            : base(solutionName, shipyardTarget)
        {
        }
    }

    [Generate]
    class ShipyardDllSolution : BaseShipyardSolution
    {
        public ShipyardDllSolution()
            : base("shipyard.dll", ShipyardUtils.DefaultShipyardTargetDll)
        {
        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.AddProject<ShipyardSystemDllProject>(target);
            configuration.AddProject<ShipyardMathDllProject>(target);
            configuration.AddProject<ShipyardGraphicsDllProject>(target);
        }
    }

    [Generate]
    class ShipyardSolution : BaseShipyardSolution
    {
        public ShipyardSolution()
            : base("shipyard", ShipyardUtils.DefaultShipyardTargetLib)
        {
        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.AddProject<ShipyardSystemProject>(target);
            configuration.AddProject<ShipyardMathProject>(target);
            configuration.AddProject<ShipyardGraphicsProject>(target);
        }
    }
}
