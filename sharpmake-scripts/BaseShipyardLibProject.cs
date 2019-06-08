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

            configuration.Output = (target.OutputType == OutputType.Lib) ? Configuration.OutputType.Lib : Configuration.OutputType.Dll;
        }

        protected override string GetTargetOutputPath()
        {
            return @"lib\";
        }
    }
}
