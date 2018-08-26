using Sharpmake;

namespace ShipyardSharpmake
{
    class ShipyardUtils
    {
        private static Target ms_DefaultShipyardTarget;
        public static Target DefaultShipyardTarget
        {
            get
            {
                if (ms_DefaultShipyardTarget == null)
                {
                    ms_DefaultShipyardTarget = new Target(Platform.win32 | Platform.win64, DevEnv.vs2015, Optimization.Debug | Optimization.Release, OutputType.Dll);
                }

                return ms_DefaultShipyardTarget;
            }
        }

        public static DependencySetting DefaultDependencySettings
        {
            get
            {
                return DependencySetting.LibraryPaths | DependencySetting.LibraryFiles | DependencySetting.IncludePaths;
            }
        }
    }
}
