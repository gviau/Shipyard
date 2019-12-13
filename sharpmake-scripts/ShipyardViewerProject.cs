using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardViewerLibProject : BaseLibProject
    {
        public ShipyardViewerLibProject()
            : base("shipyard.viewer", @"..\shipyard-viewer\framework\", ShipyardUtils.DefaultShipyardTargetDll)
        {

        }

        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.ForcedIncludes.Add("shipyardviewerlibprecomp.h");
            configuration.PrecompHeader = "shipyardviewerlibprecomp.h";
            configuration.PrecompSource = "shipyardviewerlibprecomp.cpp";
        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardSystemDllProject>(target, ShipyardUtils.DefaultDependencySettings);
            configuration.AddPublicDependency<ShipyardMathDllProject>(target, ShipyardUtils.DefaultDependencySettings);
            configuration.AddPublicDependency<ShipyardGraphicsDllProject>(target, ShipyardUtils.DefaultDependencySettings);

            configuration.AddPublicDependency<ShipyardToolsProject>(target, ShipyardUtils.DefaultDependencySettings);
        }

        protected override void ConfigureDefines(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureDefines(configuration, target);

            configuration.Defines.Add("SHIPYARD_VIEWER_LIB_DLL");
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

            configuration.ForcedIncludes.Add("shipyardviewerprecomp.h");
            configuration.PrecompHeader = "shipyardviewerprecomp.h";
            configuration.PrecompSource = "shipyardviewerprecomp.cpp";

            Configuration.VcxprojUserFileSettings projectUserFileSettings = new Configuration.VcxprojUserFileSettings();
            projectUserFileSettings.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]\..\shipyard-viewer\approot\";
            projectUserFileSettings.OverwriteExistingFile = true;

            configuration.VcxprojUserFile = projectUserFileSettings;
        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardViewerLibProject>(target, ShipyardUtils.DefaultDependencySettings);

            configuration.IncludePaths.Add(SourceRootPath + @"..\framework\");
        }
    }
}
