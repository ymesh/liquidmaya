#line 1 "bumpy.sl"
displacement bumpy(
    float amplitude = 1;
    string texturename = "";)
{
    if (texturename != "")
        N += bump(texturename, N, dPdu, dPdv) * amplitude;
}
