using Sharpmake;

namespace ShipyardSharpmake
{
    class BaseShipyardSolution : Solution
    {
        public BaseShipyardSolution(string solutionName, ITarget target)
        {
            IsFileNameToLower = false;

            Name = solutionName;

            AddTargets(target);
        }

        public virtual void ConfigureAll(Configuration configuration, Target target)
        {
            configuration.SolutionFileName = "[solution.Name]";
            configuration.SolutionPath = @"[solution.SharpmakeCsPath]\..\generated-projects\";
        }
    }
}
