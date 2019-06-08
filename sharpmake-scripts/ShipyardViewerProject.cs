using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardViewerProject : BaseExecutableProject
    {
        public ShipyardViewerProject()
            : base("ShipyardViewer", "../shipyard-viewer", ShipyardUtils.DefaultShipyardTargetDll)
        {

        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.Options.Add(Sharpmake.Options.Vc.Linker.SubSystem.Application);
            
            Configuration.VcxprojUserFileSettings projectUserFileSettings = new Configuration.VcxprojUserFileSettings();
            projectUserFileSettings.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]\..\shipyard-viewer\approot";
            projectUserFileSettings.OverwriteExistingFile = true;

            configuration.VcxprojUserFile = projectUserFileSettings;
        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardDllProject>(target, ShipyardUtils.DefaultDependencySettings);
        }
    }
}
