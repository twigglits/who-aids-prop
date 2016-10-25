#!/usr/bin/env python

import subprocess
import json
import pprint
import glob
import sys

paramNamesConfig = set()
for exe in ["simpact-cyan-release", "maxart-release" ]:
    data = subprocess.check_output([exe, "--showconfigoptions"])
    opts = json.loads(data)

    conf = opts["configNames"]
    for n in conf:
        params = conf[n]["params"]
        for p in params:
            paramNamesConfig.add(p[0])

dist = opts["distTypes"]
for d in dist:
    paramNamesConfig.add("some.option.dist")
    for p in dist[d]["params"]:
        paramNamesConfig.add("some.option.dist." + d + "." + p[0])

dist = opts["distTypes2D"]
for d in dist:
    paramNamesConfig.add("some.option.dist2d")
    for p in dist[d]["params"]:
        paramNamesConfig.add("some.option.dist2d." + d + "." + p[0])

paramNamesDoc = set()
data = [ ]
for n in glob.glob("*.rst"):
    data += open(n).readlines()

for l in data:
    l = l.strip()
    if l.startswith("- ``"):
        if l.endswith(": |br|"):
            parts = l.split('`')
            name = parts[2]
            suff = [ (".dist.type", ".dist"), (".dist2d.type", ".dist2d") ]
            for s in suff:
                if name.endswith(s[0]):
                    name = name[:-len(s[0])] + s[1]
            
            paramNamesDoc.add(name)

diff1 = paramNamesConfig-paramNamesDoc
diff2 = paramNamesDoc-paramNamesConfig

diff1 = sorted([ n for n in diff1 ])
diff2 = sorted([ n for n in diff2 ])

print "In config file but not in documentation"
for n in diff1:
    print "  " + n
print

print "In documentation but not in config file"
for n in diff2:
    print "  " + n
