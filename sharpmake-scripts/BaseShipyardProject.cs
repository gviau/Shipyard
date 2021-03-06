﻿using Sharpmake;

namespace ShipyardSharpmake
{
    abstract class BaseProject : Project
    {
        public BaseProject(string projectName, string baseSourcePath, ShipyardTarget target)
            : base(typeof(ShipyardTarget))
        {
            IsFileNameToLower = false;

            Name = projectName;

            AddTargets(target);

            SourceRootPath = @"[project.SharpmakeCsPath]\" + baseSourcePath + @"\";

            ResourceFilesExtensions.Add(".fx");
            ResourceFilesExtensions.Add(".hlsl");

            NatvisFiles.Add(@"[project.SharpmakeCsPath]\..\tools\visualizers\shipyard.natvis");
        }

        [Configure]
        public virtual void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            bool isMswinPlatform = (target.Platform == Platform.win32 || target.Platform == Platform.win64);

            string platformString = @"[target.Platform]\";
            if (isMswinPlatform)
            {
                platformString = @"Mswin\";
            }

            string outputTypeString = (target.OutputType == OutputType.Lib) ? "lib" : "dll";

            configuration.ProjectFileName   = @"[project.Name]";
            configuration.ProjectPath       = @"[project.SharpmakeCsPath]\..\tmp\generated-projects\[target.DevEnv]\" + platformString;
            configuration.IntermediatePath  = configuration.ProjectPath + @"intermediate\[target.DevEnv]\" + outputTypeString + @"\[project.Name]\";
            configuration.Name              = @"[target.Optimization]";

            string targetOutputPath = GetTargetOutputPath();

            string platformPath = @"[target.Platform]\";
            if (target.Platform == Platform.win32)
            {
                platformPath = @"x86\";
            }
            else if (target.Platform == Platform.win64)
            {
                platformPath = @"x64\";
            }

            string outputPath = @"[project.SharpmakeCsPath]\..\" + targetOutputPath + platformPath + @"[target.Optimization]\";

            configuration.TargetLibraryPath = outputPath;
            configuration.TargetPath = outputPath;

            ConfigureIncludePaths(configuration);
            ConfigurePlatform(configuration, target.Platform);
            ConfigureDefines(configuration, target);

            ConfigureProjectDependencies(configuration, target);

            ConfigureDisabledWarnings(configuration, target);

            configuration.Options.Add(Options.Vc.General.TreatWarningsAsErrors.Enable);
            configuration.Options.Add(Options.Vc.Compiler.RTTI.Disable);

            if (target.OutputType == OutputType.Lib)
            {
                configuration.Output = Configuration.OutputType.Lib;

                if (target.Optimization == OptimizationLevel.Debug)
                {
                    configuration.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebug);
                }
                else
                {
                    configuration.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreaded);
                }
            }
            else
            {
                configuration.Output = Configuration.OutputType.Dll;

                if (target.Optimization == OptimizationLevel.Debug)
                {
                    configuration.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebugDLL);
                }
                else
                {
                    configuration.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);
                }
            }
        }

        protected virtual void ConfigureIncludePaths(Configuration configuration)
        {
            
        }

        protected virtual void ConfigurePlatform(Configuration configuration, Platform platform)
        {
            configuration.Options.Add(Options.Vc.Compiler.Exceptions.Disable);

            // In practice, multi processor builds are faster than minimal rebuild: https://github.com/cinder/Cinder/issues/1158#issuecomment-148571748
            // This option cannot co-exist with multiprocessor builds, so let's disable it.
            configuration.Options.Add(Options.Vc.Compiler.MinimalRebuild.Disable);
        }

        protected virtual void ConfigureDefines(Configuration configuration, ShipyardTarget target)
        {
            switch (target.Optimization)
            {
                case OptimizationLevel.Debug:
                    configuration.Defines.Add("SHIP_DEBUG");
                    break;

                case OptimizationLevel.Profile:
                    configuration.Defines.Add("SHIP_OPTIMIZED");
                    break;

                case OptimizationLevel.Master:
                    configuration.Defines.Add("SHIP_OPTIMIZED");
                    configuration.Defines.Add("SHIP_MASTER");
                    break;
            }

            if (target.OutputType == OutputType.Lib)
            {
                configuration.Defines.Add("SHIPYARD_BUILD_STATIC");
            }

            configuration.Defines.Add("SHIPYARD_NONCLIENT_BUILD");
        }

        protected virtual void ConfigureProjectDependencies(Configuration configuration, ShipyardTarget target)
        {

        }

        protected virtual void ConfigureDisabledWarnings(Configuration configuration, ShipyardTarget target)
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

        protected abstract string GetTargetOutputPath();
    }
}
