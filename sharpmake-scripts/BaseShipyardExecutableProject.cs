using Sharpmake;

namespace ShipyardSharpmake
{
    class BaseExecutableProject : BaseProject
    {
        public BaseExecutableProject(string projectName, string relativeProjectPath, ShipyardTarget shipyardTarget)
            : base(projectName, relativeProjectPath, shipyardTarget)
        {

        }

        public BaseExecutableProject(string projectName, string relativeProjectPath)
            : base(projectName, relativeProjectPath, ShipyardUtils.DefaultShipyardTargetDll)
        {

        }

        public override void ConfigureAll(Configuration configuration, ShipyardTarget target)
        {
            base.ConfigureAll(configuration, target);

            configuration.Output = Configuration.OutputType.Exe;
        }

        protected override string GetTargetOutputPath()
        {
            return @"\tmp\bin\";
        }
    }
}
