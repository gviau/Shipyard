using Sharpmake;

namespace ShipyardSharpmake
{
    class BaseShipyardLibProject : BaseShipyardProject
    {
        public BaseShipyardLibProject(string projectName, string relativeProjectPath)
            : base(projectName, relativeProjectPath, ShipyardUtils.DefaultShipyardTargetLib)
        {

        }

        public override void ConfigureAll(Configuration configuration, Target target)
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
