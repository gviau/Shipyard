using Sharpmake;
using System;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardToolsProject : BaseShipyardProject
    {
        public ShipyardToolsProject()
            : base("shipyard.tools", @"..\shipyard\tools\", ShipyardUtils.DefaultShipyardTargetDll)
        {
            
        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardGraphicsDllProject>(target, ShipyardUtils.DefaultDependencySettings);
            configuration.AddPublicDependency<ShipyardMathDllProject>(target, ShipyardUtils.DefaultDependencySettings);
            configuration.AddPublicDependency<ShipyardSystemDllProject>(target, ShipyardUtils.DefaultDependencySettings);
        }

        protected override void ConfigureIncludePaths(Configuration configuration)
        {
            base.ConfigureIncludePaths(configuration);

            configuration.IncludePaths.Add(SourceRootPath + @"..\extern\assimp\");
        }

        protected override void ConfigurePlatform(Configuration configuration, Platform platform)
        {
            base.ConfigurePlatform(configuration, platform);

            switch (platform)
            {
                case Platform.win32:
                case Platform.win64:
                    ConfigureMswinPaths(configuration, ((platform == Platform.win64) ? "x64" : "x86"));
                    break;
            }
        }

        private void ConfigureMswinPaths(Configuration configuration, string mswinPlatformString)
        {
            configuration.LibraryPaths.Add(SourceRootPath + @"..\extern\assimp\bin\");

            configuration.LibraryFiles.Add(@"assimp-vc141-mt.lib");
            configuration.LibraryFiles.Add(@"zlib.lib");

            configuration.TargetCopyFiles.Add(SourceRootPath + @"..\extern\assimp\bin\assimp-vc141-mt.dll");
            configuration.TargetCopyFiles.Add(SourceRootPath + @"..\extern\assimp\bin\zlib.dll");
        }
    }
}
