#!/bin/sh

# 3Delight
find src -name "*.sl" -exec shaderdl {} \; >&/dev/null
# AIR
find src -name "*.sl" -exec shaded {} \; >&/dev/null
# Aqsis
find src -name "*.sl" -exec aqsl {} \; >&/dev/null
# Pixie
find src -name "*.sl" -exec sdrc {} \; >&/dev/null
# PRMan
find src -name "*.sl" -exec shader {} \; >&/dev/null
# RenderDotC
find src -name "*.sl" -exec shaderdc {} \; >&/dev/null