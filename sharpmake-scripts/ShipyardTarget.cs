using Sharpmake;
using System;

namespace ShipyardSharpmake
{
    [Fragment, Flags]
    public enum OptimizationLevel
    {
        Debug = 0x01,
        Develop = 0x02,
        Profile = 0x04,
        Master = 0x08,
    }

    class ShipyardTarget : ITarget
    {
        public Sharpmake.Platform Platform;
        public Sharpmake.DevEnv DevEnv;
        public OptimizationLevel Optimization;
        public Sharpmake.OutputType OutputType;

        public ShipyardTarget()
        {
        }

        public ShipyardTarget(Platform platform, DevEnv devEnv, OptimizationLevel optimizationLevel, OutputType outputType)
        {
            Platform = platform;
            DevEnv = devEnv;
            Optimization = optimizationLevel;
            OutputType = outputType;
        }
    }
}
