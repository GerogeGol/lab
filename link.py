#!/bin/python3

import sys

r = sys.argv[1].strip(",")
i = sys.argv[2]

print(
    f"https://mandel.gart.nz/?Re={r}&Im={i}&iters=1500&zoom=10000&colourmap=0&maprotation=0&axes=0&smooth=0"
)
