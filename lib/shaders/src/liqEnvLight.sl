
#include "liquidtools.h"
light
liqEnvLight (
    uniform string envname = "";
    uniform string envspace = "world";
    uniform float  Intensity = 1.0;
    uniform float  reflBlur = 0;
    uniform float	 samples = 4;
		uniform float	 Coloration = 0;
		uniform color	 cFilt = 1;
		uniform string Filter = "gaussian";
		uniform float  filterWidth = 1;
		uniform float  useSampledBlur = 0;
		uniform float  DiffHemisphere = .95;
    uniform float  lerp = 1;
    output uniform string __category = "environment";
    output varying float  __nondiffuse = 1;
    output varying float  __nonspecular = 1;
  
)
{
    extern point Ps;
    point Q = Ps;

    extern vector I;
		extern normal N;
		
		varying float blur;
		
#ifdef DELIGHT
    normal shading_normal = normalize( faceforward(Ns, I) );
#else
    normal Nn = normalize( N );
    normal Nf = faceforward( Nn, I, Nn );
    normal shading_normal = normalize( Nf );
#endif
    
    vector IN = normalize (I);
    vector Rfldir;
    vector Rsp;
    float kr = 1;
    
    Cl = 0;		
    solar () 
    {
      if( envname != "" ) 
      {
        color env, filt;
        
        if( 0 == surface ("__L", Rfldir) )     
      	  Rfldir = reflect( IN, shading_normal );

      	if( 0 == surface ("__blur", blur) )
    		 	blur = reflBlur;
    		
    		filt = mix (color 1, cFilt, Coloration); 
    		if ( useSampledBlur )
    		{
    		  vector r = Rfldir;
    		  float w = liqCalculateEnvSampleArea( DiffHemisphere, samples );
    		  
    		  vector v1, v2, v3, v4;
    		  liqBuildEnvironmentVectors(w, r, v1, v2, v3, v4);
    		  v1 = vtransform( envspace, v1 );
    		  v2 = vtransform( envspace, v2 );
    		  v3 = vtransform( envspace, v3 );
    		  v4 = vtransform( envspace, v4 );
    		  env = color environment ( envname, 
    		        v1, v2, v3, v4, 
    				    "blur", blur, 
    				    "samples", samples, 
    				    "filter", Filter,
    				    "width", filterWidth,
    					  "lerp", lerp); 
    		}
    		else
    		{
    		  Rsp = normalize ( vtransform ( envspace, Rfldir) );
    		  env = color environment ( envname, 
    		        Rsp, 
    				    "blur", blur, 
    				    "samples", samples, 
    				    "filter", Filter,
    				    "width", filterWidth,
    					  "lerp", lerp); 
    		}
    		Cl = env * filt;			  
      }
    }
    Cl *= Intensity;
}

