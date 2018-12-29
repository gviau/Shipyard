using Sharpmake;

namespace ShipyardSharpmake
{
    class BaseShipyardExecutableProject : BaseShipyardProject
    {
        public BaseShipyardExecutableProject(string projectName, string relativeProjectPath)
            : base(projectName, relativeProjectPath, ShipyardUtils.DefaultShipyardTargetLib)
        {

        }

        public override void ConfigureAll(Configuration configuration, Target target)
        {
            base.ConfigureAll(configuration, target);

            configuration.Output = Configuration.OutputType.Exe;
        }

        protected override string GetTargetOutputPath()
        {
            return @"\bin";
        }
    }
}
