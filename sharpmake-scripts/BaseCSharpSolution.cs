using Sharpmake;

namespace ShipyardSharpmake
{
    public abstract class BaseCSharpSolution : Solution
    {
        public BaseCSharpSolution(string solutionName, Target target)
            : base(typeof(Target))
        {
            IsFileNameToLower = false;

            Name = solutionName;

            AddTargets(target);
        }

        [Configure]
        public virtual void ConfigureAll(Configuration configuration, Target target)
        {
            configuration.SolutionFileName = "[solution.Name].[target.DevEnv]";
            configuration.SolutionPath = @"[solution.SharpmakeCsPath]\..\tmp\generated-projects\";
            configuration.Name = @"[target.Optimization]";
        }
    }
}
