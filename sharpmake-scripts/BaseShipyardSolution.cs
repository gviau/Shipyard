using Sharpmake;

namespace ShipyardSharpmake
{
    class BaseSolution : Solution
    {
        public BaseSolution(string solutionName, ShipyardTarget target)
            : base(typeof(ShipyardTarget))
        {
            IsFileNameToLower = false;

            Name = solutionName;

            AddTargets(target);
        }

        public virtual void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            configuration.SolutionFileName = "[solution.Name].[target.DevEnv]";
            configuration.SolutionPath = @"[solution.SharpmakeCsPath]\..\generated-projects\";
            configuration.Name = @"[target.Optimization]";
        }
    }
}
