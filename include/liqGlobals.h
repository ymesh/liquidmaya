class liqGlobals {
  public:
    struct {
      bool showProgress;
      bool outputDetailedComments;
      bool shaderDebugging;
    } feedback;
      
    struct {
      bool noShadowRibs;
      bool fullShadowRibs;
      bool lazyCompute;
    } shadowMaps;
        
    struct {
      bool opacityThreshold;
      bool outputAllShaders;
    } depthShadows;
    
    struct {
      bool outputAllShaders;
      bool outputLights;
    } deepShadows;
    
    struct {
      bool readArchiveable;
      
      struct {
        bool allCurves;
        bool meshUVs;
      } output
      
      struct {
        bool noSurfaces;
        bool noLights;
        bool noDisplacements;
        bool noVolumes;
        bool expandArrays;
      } shaders;
      
      struct {
        bool projectRelative;
        bool shaders;
      } paths;
      
      struct {
        bool binary;
        bool gZip;
      } format;
    
      struct {
        MString preWorld;
        MString postWorld;
        MString preObject;
      } box;
    
      enum {
        FRAME
        WORLD
        
      } chop;
      
    } rib; 
}