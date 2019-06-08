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
                return DevEnv.vs2017;
            }
        }

        public static OptimizationLevel DefaultShipyardOptimization
        {
            get
            {
                return OptimizationLevel.Debug | OptimizationLevel.Develop | OptimizationLevel.Profile | OptimizationLevel.Master;
            }
        }

        private static ShipyardTarget ms_DefaultShipyardTarget;
        public static ShipyardTarget DefaultShipyardTarget
        {
            get
            {
                if (ms_DefaultShipyardTarget == null)
                {
                    ms_DefaultShipyardTarget = new ShipyardTarget(DefaultShipyardPlatforms, DefaultShipyardDevEnv, DefaultShipyardOptimization, OutputType.Lib);
                }

                return ms_DefaultShipyardTarget;
            }
        }

        private static ShipyardTarget ms_DefaultShipyardTargetDll;
        public static ShipyardTarget DefaultShipyardTargetDll
        {
            get
            {
                if (ms_DefaultShipyardTargetDll == null)
                {
                    ms_DefaultShipyardTargetDll = new ShipyardTarget(DefaultShipyardPlatforms, DefaultShipyardDevEnv, DefaultShipyardOptimization, OutputType.Dll);
                }

                return ms_DefaultShipyardTargetDll;
            }
        }

        private static ShipyardTarget ms_DefaultShipyardTargetLib;
        public static ShipyardTarget DefaultShipyardTargetLib
        {
            get
            {
                if (ms_DefaultShipyardTargetLib == null)
                {
                    ms_DefaultShipyardTargetLib = new ShipyardTarget(DefaultShipyardPlatforms, DefaultShipyardDevEnv, DefaultShipyardOptimization, OutputType.Lib);
                }

                return ms_DefaultShipyardTargetLib;
            }
        }

        private static ShipyardTarget ms_DefaultShipyardTargetSolution;
        public static ShipyardTarget DefaultShipyardTargetSolution
        {
            get
            {
                if (ms_DefaultShipyardTargetSolution == null)
                {
                    ms_DefaultShipyardTargetSolution = new ShipyardTarget(DefaultShipyardPlatforms, DefaultShipyardDevEnv, DefaultShipyardOptimization, OutputType.Dll | OutputType.Lib);
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
