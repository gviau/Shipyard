using Sharpmake;

namespace ShipyardSharpmake
{
    class BaseLibProject : BaseProject
    {
        public BaseLibProject(string projectName, string relativeProjectPath, ShipyardTarget shipyardTarget)
            : base(projectName, relativeProjectPath, shipyardTarget)
        {

        }

        public BaseLibProject(string projectName, string relativeProjectPath)
            : base(projectName, relativeProjectPath, ShipyardUtils.DefaultShipyardTargetDll)
        {

        }

        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            if (target.OutputType == OutputType.Dll)
            {
                configuration.ProjectFileName += ".dll";
                configuration.Output = Configuration.OutputType.Dll;
            }
            else
            {
                configuration.Output = Configuration.OutputType.Lib;
            }
        }

        protected override string GetTargetOutputPath()
        {
            return @"lib\";
        }
    }
}
