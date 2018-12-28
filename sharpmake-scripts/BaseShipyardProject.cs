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

            configuration.ProjectFileName  = platformString + @"[target.DevEnv]\[project.Name]";
            configuration.ProjectPath      = @"[project.SharpmakeCsPath]\..\generated-projects\";
            configuration.IntermediatePath = configuration.ProjectPath + @"intermediate\[target.DevEnv]\[project.Name]\";

            string targetOutputPath = GetTargetOutputPath();

            string outputPath = @"..\" + targetOutputPath + @"\" + platformString + @"\[target.Optimization]\";

            configuration.TargetLibraryPath = outputPath;
            configuration.TargetPath = outputPath;

            configuration.Output = (target.OutputType == OutputType.Lib) ? Configuration.OutputType.Lib : Configuration.OutputType.Dll;

            ConfigureIncludePaths(configuration);
            ConfigurePlatform(configuration, target.Platform);
            ConfigureDefines(configuration, target.Platform);

            ConfigureProjectDependencies(configuration, target);

            ConfigureDisabledWarnings(configuration, target);

            configuration.Options.Add(Options.Vc.General.TreatWarningsAsErrors.Enable);
        }

        protected virtual void ConfigureIncludePaths(Configuration configuration)
        {

        }

        protected virtual void ConfigurePlatform(Configuration configuration, Platform platform)
        {
            configuration.Options.Add(Options.Vc.Compiler.Exceptions.Disable);
            configuration.Options.Add(Options.Vc.Compiler.MinimalRebuild.Disable);
        }

        protected virtual void ConfigureDefines(Configuration configuration, Platform platform)
        {

        }

        protected virtual void ConfigureProjectDependencies(Configuration configuration, Target target)
        {

        }

        protected virtual void ConfigureDisabledWarnings(Configuration configuration, Target target)
        {
            const string disableUnreferencedFormalParameterWarning = "4100";
            const string disableNamelessStructUnionWarning = "4201";
            const string disableUnwindSemanticsNotEnabledWarning = "4530";
            const string disableNoExceptWithNoExceptionModeWarning = "4577";

            configuration.Options.Add(new Options.Vc.Compiler.DisableSpecificWarnings(
                    disableUnreferencedFormalParameterWarning,
                    disableNamelessStructUnionWarning,
                    disableUnwindSemanticsNotEnabledWarning,
                    disableNoExceptWithNoExceptionModeWarning));
        }

        protected virtual string GetTargetOutputPath()
        {
            return @"lib\";
        }
    }
}
