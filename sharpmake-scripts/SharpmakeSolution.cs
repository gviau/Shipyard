using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    public class SharpmakeSolution : BaseCSharpSolution
    {
        public SharpmakeSolution()
            : base("Sharpmake", new Target(Platform.anycpu, ShipyardUtils.DefaultShipyardDevEnv, Optimization.Debug))
        {

        }

        public override void ConfigureAll(Configuration configuration, Target target)
        {
            base.ConfigureAll(configuration, target);

            configuration.AddProject<SharpmakeProject>(target);
        }
    }
}
