using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardViewerLibProject : BaseLibProject
    {
        public ShipyardViewerLibProject()
            : base("shipyard.viewer", @"..\shipyard-viewer\src\framework\", ShipyardUtils.DefaultShipyardTargetDll)
        {

        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardSystemDllProject>(target, ShipyardUtils.DefaultDependencySettings);
            configuration.AddPublicDependency<ShipyardMathDllProject>(target, ShipyardUtils.DefaultDependencySettings);
            configuration.AddPublicDependency<ShipyardGraphicsDllProject>(target, ShipyardUtils.DefaultDependencySettings);

            configuration.AddPublicDependency<ShipyardToolsProject>(target, ShipyardUtils.DefaultDependencySettings);
        }
    }

    [Generate]
    class ShipyardViewerProject : BaseExecutableProject
    {
        public ShipyardViewerProject()
            : base("shipyard.viewer.app", @"..\shipyard-viewer\src\", ShipyardUtils.DefaultShipyardTargetDll)
        {

        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.Options.Add(Sharpmake.Options.Vc.Linker.SubSystem.Application);

            Configuration.VcxprojUserFileSettings projectUserFileSettings = new Configuration.VcxprojUserFileSettings();
            projectUserFileSettings.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]\..\shipyard-viewer\approot\";
            projectUserFileSettings.OverwriteExistingFile = true;

            configuration.VcxprojUserFile = projectUserFileSettings;
        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardViewerLibProject>(target, ShipyardUtils.DefaultDependencySettings);
        }
    }
}
