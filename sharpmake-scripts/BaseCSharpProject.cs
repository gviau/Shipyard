using Sharpmake;

namespace ShipyardSharpmake
{
    public abstract class BaseCSharpProject : CSharpProject
    {
        public BaseCSharpProject(string projectName, string baseSourcePath, Target target)
            : base(typeof(Target))
        {
            IsFileNameToLower = false;

            Name = projectName;

            AddTargets(target);

            RootPath = @"[project.SharpmakeCsPath]\" + baseSourcePath + @"\";
            SourceRootPath = baseSourcePath;
            AssemblyName = projectName;
        }

        [Configure]
        public virtual void ConfigureAll(Configuration configuration, Target target)
        {
            configuration.ProjectFileName =  @"[project.Name]";
            configuration.ProjectPath = @"[project.SharpmakeCsPath]\..\generated-projects\[target.DevEnv]\";
            configuration.IntermediatePath = configuration.ProjectPath + @"intermediate\[target.DevEnv]\[project.Name]\";
            configuration.Name = @"[target.Optimization]";

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
        }

        protected abstract string GetTargetOutputPath();
    }
}
