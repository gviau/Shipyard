using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardProject : BaseShipyardProject
    {
        public ShipyardProject()
            : base("Shipyard", "../shipyard", ShipyardUtils.DefaultShipyardTargetLib)
        {

        }

        [Configure]
        public override void ConfigureAll(Configuration configuration, Target target)
        {
            base.ConfigureAll(configuration, target);
        }

        protected override void ConfigureIncludePaths(Configuration configuration)
        {
            base.ConfigureIncludePaths(configuration);

            configuration.IncludePaths.Add(SourceRootPath);
            configuration.IncludePaths.Add(SourceRootPath + @"extern\");
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

        protected override void ConfigureDefines(Configuration configuration, Platform platform)
        {
            base.ConfigureDefines(configuration, platform);

            configuration.Defines.Add("SHIPYARD_NONCLIENT_BUILD");
        }

        private void ConfigureMswinPaths(Configuration configuration, string mswinPlatformString)
        {
            configuration.IncludePrivatePaths.Add(@"$(DXSDK_DIR)\Include\");

            configuration.LibraryPaths.Add(@"$(DXSDK_DIR)\Lib\" + mswinPlatformString + @"\");

            configuration.LibraryFiles.Add("d3d11.lib");
            configuration.LibraryFiles.Add("d3dx11.lib");
            configuration.LibraryFiles.Add("dxgi.lib");
            configuration.LibraryFiles.Add("DxErr.lib");
            configuration.LibraryFiles.Add("dxguid.lib");
            configuration.LibraryFiles.Add("d3dcompiler.lib");
        }
    }
}
