using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardViewerProject : BaseShipyardExecutableProject
    {
        public ShipyardViewerProject()
            : base("ShipyardViewer", "../shipyard-viewer")
        {

        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.Options.Add(Sharpmake.Options.Vc.Linker.SubSystem.Application);
        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardProject>(target, ShipyardUtils.DefaultDependencySettings);
        }
    }
}
