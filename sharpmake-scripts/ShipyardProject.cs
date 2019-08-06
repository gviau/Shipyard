using Sharpmake;

namespace ShipyardSharpmake
{
    class BaseShipyardProject : BaseLibProject
    {
        public BaseShipyardProject(string projectName, string relativeProjectPath, ShipyardTarget shipyardTarget)
            : base(projectName, relativeProjectPath, shipyardTarget)
        {

        }

        protected override void ConfigureIncludePaths(Configuration configuration)
        {
            base.ConfigureIncludePaths(configuration);

            configuration.IncludePaths.Add(SourceRootPath + @"..\");
        }
    }

    class BaseShipyardSystemProject : BaseShipyardProject
    {
        public BaseShipyardSystemProject(ShipyardTarget shipyardTarget)
            : base("shipyard.system", @"..\shipyard\system\", shipyardTarget)
        {

        }
    }

    class BaseShipyardMathProject : BaseShipyardProject
    {
        public BaseShipyardMathProject(ShipyardTarget shipyardTarget)
            : base("shipyard.math", @"..\shipyard\math\", shipyardTarget)
        {

        }

        protected override void ConfigureIncludePaths(Configuration configuration)
        {
            base.ConfigureIncludePaths(configuration);

            configuration.IncludePaths.Add(SourceRootPath + @"..\extern\glm\");
        }
    }

    class BaseShipyardGraphicsProject : BaseShipyardProject
    {
        public BaseShipyardGraphicsProject(ShipyardTarget shipyardTarget)
            : base("shipyard.graphics", @"..\shipyard\graphics\", shipyardTarget)
        {
            AdditionalSourceRootPaths.Add(SourceRootPath + @"..\extern\imgui\");
        }

        protected override void ConfigureIncludePaths(Configuration configuration)
        {
            base.ConfigureIncludePaths(configuration);

            configuration.IncludePaths.Add(SourceRootPath + @"..\extern\directx\");
            configuration.IncludePaths.Add(SourceRootPath + @"..\extern\imgui\");
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

    [Generate]
    class ShipyardSystemDllProject : BaseShipyardSystemProject
    {
        public ShipyardSystemDllProject()
            : base(ShipyardUtils.DefaultShipyardTargetDll)
        {

        }
    }

    [Generate]
    class ShipyardSystemProject : BaseShipyardSystemProject
    {
        public ShipyardSystemProject()
            : base(ShipyardUtils.DefaultShipyardTargetLib)
        {

        }
    }

    [Generate]
    class ShipyardMathDllProject : BaseShipyardMathProject
    {
        public ShipyardMathDllProject()
            : base(ShipyardUtils.DefaultShipyardTargetDll)
        {

        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardSystemDllProject>(target, ShipyardUtils.DefaultDependencySettings);
        }
    }

    [Generate]
    class ShipyardMathProject : BaseShipyardMathProject
    {
        public ShipyardMathProject()
            : base(ShipyardUtils.DefaultShipyardTargetLib)
        {

        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardSystemProject>(target, ShipyardUtils.DefaultDependencySettings);
        }
    }

    [Generate]
    class ShipyardGraphicsDllProject : BaseShipyardGraphicsProject
    {
        public ShipyardGraphicsDllProject()
            : base(ShipyardUtils.DefaultShipyardTargetDll)
        {

        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardSystemDllProject>(target, ShipyardUtils.DefaultDependencySettings);
            configuration.AddPublicDependency<ShipyardMathDllProject>(target, ShipyardUtils.DefaultDependencySettings);
        }
    }

    [Generate]
    class ShipyardGraphicsProject : BaseShipyardGraphicsProject
    {
        public ShipyardGraphicsProject()
            : base(ShipyardUtils.DefaultShipyardTargetLib)
        {

        }

        protected override void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardSystemProject>(target, ShipyardUtils.DefaultDependencySettings);
            configuration.AddPublicDependency<ShipyardMathProject>(target, ShipyardUtils.DefaultDependencySettings);
        }
    }
}
