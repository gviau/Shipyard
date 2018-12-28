using Sharpmake;

namespace ShipyardSharpmake
{
    class BaseShipyardProject : Project
    {
        public BaseShipyardProject(string projectName, string baseSourcePath, ITarget target)
        {
            IsFileNameToLower = false;

            Name = projectName;

            AddTargets(target);

            SourceRootPath = @"[project.SharpmakeCsPath]\" + baseSourcePath + @"\";
        }

        public virtual void ConfigureAll(Configuration configuration, Target target)
        {
            bool isMswinPlatform = (target.Platform == Platform.win32 || target.Platform == Platform.win64);

            string platformString = @"[target.Platform]\";
            if (isMswinPlatform)
            {
                platformString = @"Mswin\";
            }

            configuration.ProjectFileName = platformString + @"[target.DevEnv]\[project.Name]";
            configuration.ProjectPath = @"[project.SharpmakeCsPath]\..\generated-projects\";
            configuration.IntermediatePath = configuration.ProjectPath + @"intermediate\";

            configuration.Output = (target.OutputType == OutputType.Lib) ? Configuration.OutputType.Lib : Configuration.OutputType.Dll;

            ConfigureIncludePaths(configuration);
            ConfigurePlatform(configuration, target.Platform);
            ConfigureDefines(configuration, target.Platform);

            ConfigureProjectDependencies(configuration, target);
        }

        protected virtual void ConfigureIncludePaths(Configuration configuration)
        {

        }

        protected virtual void ConfigurePlatform(Configuration configuration, Platform platform)
        {

        }

        protected virtual void ConfigureDefines(Configuration configuration, Platform platform)
        {

        }

        protected virtual void ConfigureProjectDependencies(Configuration configuration, Target target)
        {

        }
    }
}
