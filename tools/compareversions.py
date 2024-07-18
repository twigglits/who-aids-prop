#!/usr/bin/env python

from __future__ import print_function
import pysimpactcyan
import oldpysimpactcyan
import sys
import random
import time

try:
    import comparesettings
except Exception as e:
    print("ERROR: {}".format(e))
    print("""
You need a 'comparesettings.py' file with settings for the output comparison
and speed comparison between different versions.

An example file could contain:

    # Config settings to compare the output files of different simpact cyan
    # executables
    outputTests = [ { "population.nummen": 100, "population.numwomen": 100 } ]

    # Config settings to compare the speed with which different simpact executables
    # run.
    speedTests = [ { "population.nummen": 1000, "population.numwomen": 1000 } ]

    # Each speed test will be run this amount of times, and the average and standard
    # deviation will be calculated
    speedTestRepeats = 10

""")
    sys.exit(-1)

def getSimpactForDir(d):

    for impl in [ pysimpactcyan, oldpysimpactcyan ]:

        s = impl.PySimpactCyan()
        s.setSimpactDirectory(d)
        try:
            s.setSimulationPrefix("simpact-cyan")
            print("Using {} for {}".format(impl.__name__, d))
            return s
        except:
            pass

    raise Exception("Neither the old nor the new simpact bindings seem to work with directory {}".format(d))

def calcAvgAndDev(tlist):
    if len(tlist) == 0:
        return (0.0,0.0)

    avg = 0.0
    for t in tlist:
        avg += t

    avg /= len(tlist)
    dev = 0.0
    for t in tlist:
        d = t - avg
        dev += d*d

    dev /= len(tlist)
    dev = dev**0.5

    return (avg, dev)

def main():

    if sys.version_info.major != 2 or sys.version_info.minor < 7:
        print("Need python 2.7")
        sys.exit(-1)

    try:
        dir1 = sys.argv[1]
        dir2 = sys.argv[2]
        outputDir = sys.argv[3]

        if len(sys.argv) > 4:
            raise Exception("Too many arguments")
    except Exception as e:
        print("Error: {}".format(e))
        print("Usage: {} dir1 dir2 outputdir".format(sys.argv[0]))
        sys.exit(-1)

    simpact1 = getSimpactForDir(dir1)
    simpact2 = getSimpactForDir(dir2)

    print("Ok, can use both directories")

    outputTests = comparesettings.outputTests

    print("TEST: RUNNING OUTPUT COMPARISONS")
    print("TEST: ==========================")
    for t in outputTests:
        for parallel in [ False, True ]:
            for opt in [ True, False ]:
                for release in [ True, False ]:
                    seed = int(random.random() * 1000000)
                    print("TEST: seed = {} parallel = {} opt = {} release = {}".format(seed,parallel,opt,release))
                    sys.stdout.flush()

                    if type(opt) == tuple:
                        opt1 = opt[0]
                        opt2 = opt[1]
                    else:
                        opt1 = opt
                        opt2 = opt

                    ret1 = simpact1.run(t, outputDir, parallel = parallel, opt = opt, release = release, seed = seed)
                    ret2 = simpact2.run(t, outputDir, parallel = parallel, opt = opt, release = release, seed = seed)

                    evt1 = open(ret1["logevents"]).read()
                    evt2 = open(ret2["logevents"]).read()
                    if evt1 == evt2:
                        print("TEST: PASS")
                    else:
                        print("TEST: FAILED")
                    sys.stdout.flush()

    print("TEST: RUNNING SPEED COMPARISONS")
    print("TEST: =========================")

    speedTests = comparesettings.speedTests
    repeats = comparesettings.speedTestRepeats

    for t in speedTests:
        parallel = False
        opt = True
        release = True
        seed = int(random.random() * 1000000)

        print("TEST: seed = {} parallel = {} opt = {} release = {}".format(seed,parallel,opt,release))
        sys.stdout.flush()

        times1 = [ ]
        times2 = [ ]
        for r in range(repeats):

            if type(opt) == tuple:
                opt1 = opt[0]
                opt2 = opt[1]
            else:
                opt1 = opt
                opt2 = opt

            t0 = time.time()
            ret1 = simpact1.run(t, outputDir, parallel = parallel, opt = opt, release = release, seed = seed)
            t1 = time.time()
            ret2 = simpact2.run(t, outputDir, parallel = parallel, opt = opt, release = release, seed = seed)
            t2 = time.time()

            dt1 = t1-t0
            dt2 = t2-t1

            times1.append(dt1)
            times2.append(dt2)

            evt1 = open(ret1["logevents"]).read()
            evt2 = open(ret2["logevents"]).read()
            if evt1 == evt2:
                print("TEST: PASS, DT1 = {}, DT2 = {}".format(dt1,dt2))
            else:
                print("TEST: FAILED")
            sys.stdout.flush()

        avg1, dev1 = calcAvgAndDev(times1)
        avg2, dev2 = calcAvgAndDev(times2)

        print("TEST: RUNTIME1 = {} +/- {}".format(avg1,dev1))
        print("TEST: RUNTIME2 = {} +/- {}".format(avg2,dev2))
        print("TEST: ------------------------------------------------------")

if __name__ == "__main__":
    main()
