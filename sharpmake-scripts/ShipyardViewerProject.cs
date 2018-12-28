using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardViewerProject : BaseShipyardProject
    {
        public ShipyardViewerProject()
            : base("ShipyardViewer", "../shipyard-viewer", ShipyardUtils.DefaultShipyardTargetLib)
        {

        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, Target target)
        {
            base.ConfigureAll(configuration, target);

            // configuration.Output = Configuration.OutputType.Exe;
        }

        protected override void ConfigureProjectDependencies(Configuration configuration, Target target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardProject>(target, ShipyardUtils.DefaultDependencySettings);
        }
    }
}
