using Sharpmake;

namespace ShipyardSharpmake
{
    class ShipyardUtils
    {
        public static Platform DefaultShipyardPlatforms
        {
            get
            {
                return Platform.win32 | Platform.win64;
            }
        }

        public static DevEnv DefaultShipyardDevEnv
        {
            get
            {
                return DevEnv.vs2015;
            }
        }

        public static Optimization DefaultShipyardOptimization
        {
            get
            {
                return Optimization.Debug | Optimization.Release;
            }
        }

        private static Target ms_DefaultShipyardTarget;
        public static Target DefaultShipyardTarget
        {
            get
            {
                if (ms_DefaultShipyardTarget == null)
                {
                    ms_DefaultShipyardTarget = new Target(DefaultShipyardPlatforms, DefaultShipyardDevEnv, DefaultShipyardOptimization);
                }

                return ms_DefaultShipyardTarget;
            }
        }

        private static Target ms_DefaultShipyardTargetLib;
        public static Target DefaultShipyardTargetLib
        {
            get
            {
                if (ms_DefaultShipyardTargetLib == null)
                {
                    ms_DefaultShipyardTargetLib = new Target(DefaultShipyardPlatforms, DefaultShipyardDevEnv, DefaultShipyardOptimization, OutputType.Dll);
                }

                return ms_DefaultShipyardTargetLib;
            }
        }

        private static Target ms_DefaultShipyardTargetSolution;
        public static Target DefaultShipyardTargetSolution
        {
            get
            {
                if (ms_DefaultShipyardTargetSolution == null)
                {
                    ms_DefaultShipyardTargetSolution = new Target(DefaultShipyardPlatforms, DefaultShipyardDevEnv, DefaultShipyardOptimization, OutputType.Dll | OutputType.Lib);
                }

                return ms_DefaultShipyardTargetSolution;
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
