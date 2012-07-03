/*
**
** The contents of this file are subject to the Mozilla Public License Version 1.1 (the
** "License"); you may not use this file except in compliance with the License. You may
** obtain a copy of the License at http://www.mozilla.org/MPL/
**
** Software distributed under the License is distributed on an "AS IS" basis, WITHOUT
** WARRANTY OF ANY KIND, either express or implied. See the License for the specific
** language governing rights and limitations under the License.
**
** The Original Code is the Liquid Rendering Toolkit.
**
** The Initial Developer of the Original Code is Colin Doncaster. Portions created by
** Colin Doncaster are Copyright (C) 2002. All Rights Reserved.
**
** Contributor(s): Philippe Leprince.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/



surface
liquidPfxHair(  uniform float specularPower = 3.0;
                uniform float translucence  = 0.0;
                uniform color specularColor = 0.0;
                output varying color pfxHair_vtxColor   = 0.0;
                output varying color pfxHair_vtxOpacity = 1.0;
              )
{
  varying vector T = normalize (dPdv);
  varying vector V = -normalize(I);
  varying color Diff = 0, Spec = 0;
  uniform string pass = "";
  option( "user:pass", pass );

  if ( pass == "deepshadow" ) {
    // skip illuminance computations
  } else {

    illuminance (P) {

      // diffuse
      float theta = acos (T.normalize(L));
      Diff += Cl * sin(theta);

      // specular
      float cosang = cos (abs (theta - acos (-T.V)));
      Spec += Cl * pow (cosang, specularPower);

    }
    Ci = Diff * pfxHair_vtxColor + Spec * specularColor + ambient();
  }

  Oi = pfxHair_vtxOpacity;
  Ci *= Oi;
}
