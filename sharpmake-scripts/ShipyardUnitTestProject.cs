﻿using Sharpmake;

namespace ShipyardSharpmake
{
    [Generate]
    class ShipyardUnitTestProject : BaseShipyardExecutableProject
    {
        public ShipyardUnitTestProject()
            : base("ShipyardUnitTest", "../shipyard-unit-test")
        {
        }
        
        [Configure]
        public override void ConfigureAll(Configuration configuration, Target target)
        {
            base.ConfigureAll(configuration, target);
        }

        protected override void ConfigureProjectDependencies(Configuration configuration, Target target)
        {
            base.ConfigureProjectDependencies(configuration, target);

            configuration.AddPublicDependency<ShipyardProject>(target, ShipyardUtils.DefaultDependencySettings);
        }
    }
}
