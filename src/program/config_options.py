import sys
import pprint
import copy

def getBasicSettingsOptions():

    distTypes = { }
    distTypes['fixed'] = { 'params': [ ('value', 0) ],
                           'info': None
                         }
    distTypes['uniform'] = { 'params': [ ('min', 0), ('max', 1) ],
                             'info': "Parameters for a uniform distribution"
                           }
    distTypes['beta'] = { 'params': [ ('a', None), ('b', None), ('min', None), ('max', None) ],
                          'info': """Parameters for a beta distribution (rescaled)
prob(x) = gamma(a+b)/(gamma(a)*gamma(b))*((x-min)/(max-min))^(a-1.0)*(1.0-((x-min)/(max-min)))^(b-1.0) * 1.0/(max-min)"""
                        }
    distTypes['gamma'] = { 'params': [ ('a', None), ('b', None) ],
                           'info': """Parameters for a gamma distribution
prob(x) = (x^(a-1.0))*exp(-x/b)/((b^a)*gamma(a))"""
                         }
    distTypes['lognormal'] = { 'params': [ ('zeta', 0), ('sigma', None) ],
                               'info': """Parameters for a log-normal distribution
prob(x) = 1.0/(x*s*sqrt(2.0*pi)) * exp(- (ln(x)-z)^2 / (2.0*s^2))"""
                             }

    configNames = { }
    configNames['EventChronicStage'] = { 'depends': None,
                                         'params': [ ('chronicstage.acutestagetime', 1.0/4.0) ],
                                         'info': "Duration of the acute stage. 3 months = 3/12 = 0.25" 
                                       }
    configNames['EventAIDSStage'] = { 'depends': None,
                                      'params': [ ('aidsstage.start', 1.25),
                                                  ('aidsstage.final', 0.5) ],
                                      'info': """Indicates the time interval before death that the AIDS stages occur
The defaults are 15 and 6 months before death"""
                                    }

    configNames['EventBirth'] = { 'depends': None,
                                  'params': [ ('birth.boygirlratio', 1.0/2.01) ],
                                  'info': """When someone is born, a random number is chosen from [0,1], 
and if smaller than this boygirlratio, the new child is male. Otherwise, a 
woman is added to the population.

Default is 1.0/2.01"""
                                }

    configNames['EventBirth_pregduration'] = { 'depends': None,
                                               'params': [ ('birth.pregnancyduration.dist', distTypes, # We'll override the default params
                                                                                            ('fixed', [ ('value', 268.0/365.0 ) ] ) ) ],
                                               'info': """This parameter is used to specify the pregnancy duration. The default
is the fixed value of 268/365"""
                                             }

    configNames['EventConception'] = { 'depends': None,
                                       'params': [ ('conception.alpha_base', -3),
                                                   ('conception.alpha_ageman', 0),
                                                   ('conception.alpha_agewoman', 0),
                                                   ('conception.alpha_wsf', 0),
                                                   ('conception.beta', 0),
                                                   ('conception.t_max', 200),
                                                   ('conception.wsf.dist', distTypes) ],

                                       'info': """Parameters for the conception event. Hazard is 
    h = exp(alpha_base + alpha_ageman * AgeMan(t) + alpha_agewoman * AgeWoman(t) 
            + alpha_wsf * WSF + beta*(t-t_ref) )

Here, WSF is a number that's generated at random from the specified distribution
when a conception event is scheduled.
"""
                                     }

    configNames['EventDebut'] = { 'depends': None, 
                                  'params' : [ ('debut.debutage', 15) ],
                                  'info': """Age at which a person becomes sexually active and can form
relationships"""
                                }

    configNames['EventDissolution'] = { 'depends': None,
                                        'params': [ ('dissolution.alpha_0', 0.1),
                                                    ('dissolution.alpha_1', 0),
                                                    ('dissolution.alpha_2', 0),
                                                    ('dissolution.alpha_3', 0),
                                                    ('dissolution.alpha_4', 0),
                                                    ('dissolution.alpha_5', 0),
                                                    ('dissolution.Dp', 0),
                                                    ('dissolution.beta', 0),
                                                    ('dissolution.t_max', 200) ],
                                        'info': """These are the parameters for the hazard in the dissolution event.
see http://research.edm.uhasselt.be/~jori/simpact/documentation/simpactcyan.html
for more information.
"""
                                       }

    configNames['EventFormationTypes'] = { 'depends': None,
                                           'params': [ ('formation.hazard.type', 'agegap', [ 'simple', 'agegap' ] ) ],
                                           'info': None 
                                         }

    configNames['EventFormation_simple'] = { 'depends': ('EventFormationTypes', 'formation.hazard.type', 'simple'),
                                             'params': [ ('formation.hazard.simple.alpha_0', 0.1),
                                                         ('formation.hazard.simple.alpha_1', 0),
                                                         ('formation.hazard.simple.alpha_2', 0),
                                                         ('formation.hazard.simple.alpha_3', 0),
                                                         ('formation.hazard.simple.alpha_4', 0),
                                                         ('formation.hazard.simple.alpha_5', 0),
                                                         ('formation.hazard.simple.alpha_6', 0),
                                                         ('formation.hazard.simple.alpha_7', 0),
                                                         ('formation.hazard.simple.Dp', 0),
                                                         ('formation.hazard.simple.beta', 0),
                                                         ('formation.hazard.simple.t_max', 200) ],
                                             'info': """These are the parameters for the hazard in the simple formation event.
see http://research.edm.uhasselt.be/~jori/simpact/documentation/simpactcyan.html
for more information.
"""
                                            }

    configNames['EventFormation_agegap'] = { 'depends': ( 'EventFormationTypes', 'formation.hazard.type', 'agegap' ),
                                             'params': [ ('formation.hazard.agegap.baseline', 0.1),
                                                         ('formation.hazard.agegap.numrel_man', 0),
                                                         ('formation.hazard.agegap.numrel_woman', 0),
                                                         ('formation.hazard.agegap.numrel_diff', 0),
                                                         ('formation.hazard.agegap.meanage', 0),
                                                         ('formation.hazard.agegap.eagerness_sum', 0),
                                                         ('formation.hazard.agegap.eagerness_diff', 0),
                                                         ('formation.hazard.agegap.gap_factor_man', 0),
                                                         ('formation.hazard.agegap.gap_agescale_man', 0),
                                                         ('formation.hazard.agegap.gap_factor_woman', 0),
                                                         ('formation.hazard.agegap.gap_agescale_woman', 0),
                                                         ('formation.hazard.agegap.beta', 0),
                                                         ('formation.hazard.agegap.t_max', 200) ],
                                             'info': """These are the parameters for the hazard in the 'agegap' formation event.
see http://research.edm.uhasselt.be/~jori/simpact/documentation/simpactcyan.html
for more information.
"""
                                           }

    configNames['EventIntervention'] = { 'depends': None,
                                         'params': [ ('intervention.enabled', 'no', [ 'yes', 'no'] ) ],
                                         'info': """If you enable the intervention event, you need to specify a number of times
at which this event should fire. On these times, some new configuration lines
will be read, overriding the initial parameters read from config file."""
                                       }

    configNames['EventIntervention_enabled'] = { 'depends': ( 'EventIntervention', 'intervention.enabled', 'yes'),
                                                 'params': [ ('intervention.baseconfigname', None),
                                                             ('intervention.times', None),
                                                             ('intervention.fileids', None) ],
                                                 'info': """In 'intervention.times' you need to specify the times at which the 
intervention event should fire. All times must be positive and the list
of times must be increasing.

The 'intervention.baseconfigname' is the filename template that should be
used to read the config settings from for the intervention events. For each
intervention time, the '%' character will either be replaced by the 
corresponding string from 'intervention.fileids', or by the time specified 
in 'intervention.times' if you leave 'intervention.fileids' empty.

For example:
  intervention.baseconfigname = intconfig_%.txt
  intervention.times = 1,2,3
  intervention.fileids = A,B,C
will read intervention settings from 'intconfig_A.txt', 'intconfig_B.txt' and
'intconfig_C.txt'.

If you leave the file IDs empty,
  intervention.fileids =
then the files used would be 'intconfig_1.txt', 'intconfig_2.txt' and
'intconfig_3.txt'.
""" 
                                               }

    configNames['EventMortality_AIDS'] = { 'depends': None,
                                           'params': [ ('mortality.aids.survtime.C', 1325.0),
                                                       ('mortality.aids.survtime.k', -0.49) ],
                                           'info': """Parameters for the calculation of the survival time from the
# set-point viral load: t_surv = C/Vsp^(-k)"""
                                         }

    configNames['EventMortality_Normal'] = { 'depends': None,
                                             'params':  [ ('mortality.normal.weibull.shape', 4.0),
                                                          ('mortality.normal.weibull.scale', 70.0),
                                                          ('mortality.normal.weibull.genderdiff', 5.0) ],
                                             'info': """Parameters for the weibull distribution from which a non-aids
time of death is picked."""
                                           }

    configNames['EventTransmission'] = { 'depends': None,
                                         'params': [ ('transmission.param.a', -1.3997),
                                                     ('transmission.param.b', -12.0220),
                                                     ('transmission.param.c', 0.1649),
                                                     ('transmission.param.d1', 0),
                                                     ('transmission.param.d2', 0) ],
                                         'info': """The hazard of transmission is h = exp(a + b * V^(-c) + d1*Pi + d2*Pj), 
where V can be either the set-point viral load or the acute stage 
viral load. 

Default parameters originate from a fit to the Lingappa et al. data."""
                                       }

    configNames['EventTreatment'] = { 'depends': None,
                                      'params': [ ('treatment.fraction.time', 0.8),
                                                  ('treatment.fraction.log_viralload', 0.7),
                                                  ('treatment.enabled', 'yes', ['yes', 'no'] ) ],
                                      'info': """A person receives treatment after a certain fraction of t_surv has passed.
The effect will be to lower the set-point viral load in such a way that
on a log scale the Vsp value will be multiplied by the specified fraction."""
                                    }

    configNames['EventSeeding'] = { 'depends': None, 
                                    'params': [ ('hivseed.time', 0),
                                                ('hivseed.fraction', 0.2) ],
                                    'info': """Controls when the initial HIV seeders are introduced. If the time is negative,
no seeders will be introduced since the event will be ignored (simulation time
starts at t = 0)."""
                                  }

    configNames['PersonVspAcute'] = { 'depends': None,
                                      'params': [ ('person.vsp.toacute.x', 10.0),
                                                  ('person.vsp.toaids.x', 7.0),
                                                  ('person.vsp.tofinalaids.x', 12.0),
                                                  ('person.vsp.maxvalue', 1e9) ],
                                      'info': """The viral load during the other stages is based on the set point viral load:
   V = [ max(ln(x)/b + Vsp^(-c), maxvalue^(-c)) ]^(-1/c)
The b and c parameters are specified in the parameters from the transmission
event."""
                                    }

    configNames['PersonEagerness'] = { 'depends': None, 
                                       'params': [ ('person.eagerness.dist', distTypes) ],
                                       'info': """The per-person parameter for the eagerness to form a relationship is chosen
from a specific distribution with certain parameters."""
                                     }

    configNames['PersonAgeGapMen'] = { 'depends': None, 
                                       'params': [ ('person.agegap.man.dist', distTypes) ],
                                       'info': None
                                     }

    configNames['PersonAgeGapWomen'] = { 'depends': None, 
                                         'params': [ ('person.agegap.woman.dist', distTypes) ],
                                         'info': None
                                       }

    configNames['PersonVspModelTypes'] = { 'depends': None, 
                                           'params': [ ('person.vsp.model.type', 'logbinormal', [ 'logweibullwithnoise', 'logbinormal'] ) ],
                                           'info': """The type of model to use for the Vsp value of the seeders and for inheriting
Vsp values."""
                                         }

    configNames['PersonVspModel_weibullnoise'] = { 'depends': ('PersonVspModelTypes', 'person.vsp.model.type', 'logweibullwithnoise'),
                                                   'params': [ ('person.vsp.model.logweibullwithnoise.weibullscale', 5.05),
                                                               ('person.vsp.model.logweibullwithnoise.weibullshape', 7.2),
                                                               ('person.vsp.model.logweibullwithnoise.fracsigma', 0.1),
                                                               ('person.vsp.model.logweibullwithnoise.onnegative', 'logweibull', [ 'logweibull', 'noiseagain'] ) ],
                                                   'info': """For 'seeders', people marked as infected at the start of the simulation,
the logarithm of set-point viral load is chosen from a weibull distribution.

In Vsp heritability, added random noise uses a sigma that's 10% of the 
original Vsp. When after adding noise upon inheriting the Vsp value, the
Vsp is negative: use 'noiseagain' to pick from gaussian(VspOrigin,sigma) 
again, or use 'logweibull' to pick from the initial distribution again."""
                                                 }

    configNames['PersonVspModel_binormal'] = { 'depends': ('PersonVspModelTypes', 'person.vsp.model.type', 'logbinormal'),
                                               'params': [ ('person.vsp.model.logbinormal.mean', 4),
                                                           ('person.vsp.model.logbinormal.sigma', 1),
                                                           ('person.vsp.model.logbinormal.rho', 0.33),
                                                           ('person.vsp.model.logbinormal.min', 1),
                                                           ('person.vsp.model.logbinormal.max', 8),
                                                           ('person.vsp.model.logbinormal.usealternativeseeddist', 'no', [ 'yes', 'no'])],
                                               'info': """Both the initial 'seed' value and the inherited Vsp value are
chosen so that the 2D distribution is a clipped binormal distribution 
(on a log scale). The shape parameters (mean, sigma), clipping parameters 
(min, max) and the correlation parameter (rho) are all configurable here.

Additionally, you can also specify that an alternative distribution must
be used to pick the Vsp values of the seeders. Note that these will also
be limited to the (min,max) interval.
"""
                                             }

    configNames['PersonVspModel_binormal_altseed'] = { 'depends': ('PersonVspModel_binormal', 'person.vsp.model.logbinormal.usealternativeseeddist', 'yes'),
                                                       'params': [ ('person.vsp.model.logbinormal.alternativeseed.dist', distTypes) ],
                                                       'info': None }

    configNames['Population_1'] = { 'depends': None,
                                    'params': [ ('population.nummen', 100),
                                              ('population.numwomen', 100),
                                              ('population.simtime', 15),
                                              ('population.agedistfile', None) ],
                                    'info': None
                                  }

    configNames['Population_2'] = { 'depends': None,
                                    'params': [ ('population.eyecap.fraction', 1) ],
                                     'info': """If set to 1, formation events will be scheduled for all man,woman
pairs (who are both sexually active). This is the default behaviour.
If set to a smaller number, only a fraction of the formation events 
that would otherwise be scheduled are now used. This fraction is not 
only used in the initial scheduling of formation events, but also 
when a debut event fires, to limit the newly scheduled formation events."""
                                  }

    configNames['LogSystem'] = { 'depends': None,
                                 'params': [ ('logsystem.filename.events', None),
                                             ('logsystem.filename.persons', None),
                                             ('logsystem.filename.relations', None) ],
                                 'info': None
                               }

    return (configNames, distTypes)

def getExpandedSettingsOptions():

    configNames, distTypes = getBasicSettingsOptions()

    # Change the config entries which have a 'distTypes' setting

    newConfig = copy.deepcopy(configNames)
    for n in configNames:
        params = configNames[n]['params']
        for i in range(len(params)):
            p = params[i]
            pName = p[0]
            pValue = p[1]
            if pValue is distTypes:

                defaultDistName = 'fixed'
                defaultDistParams = None
                if len(p) == 3:
                    defaultDistOptions = p[2]
                    defaultDistName = defaultDistOptions[0]
                    defaultDistParams = defaultDistOptions[1]

                # Adjust the entry in 'newConfig'
                possibleNames = [ t for t in distTypes ]
                newConfig[n]['params'][i] = (pName + ".type", defaultDistName, possibleNames)

                #print "configNames[%s] = " % n
                #pprint.pprint(newConfig[n])
                #print

                for distName in distTypes:
                    distParams = distTypes[distName]['params']

                    if len(params) > 1:
                        newConfName = n + "_" + str(i) + "_" + distName
                    else:
                        newConfName = n + "_" + distName

                    newConfig[newConfName] = { 'depends': (n, pName + ".type", distName) }

                    if not defaultDistParams:
                        newConfig[newConfName]['params'] = [ (pName + "." + distName + "." + dp, dv) for dp,dv in distParams ]
                    else:
                        newConfig[newConfName]['params'] = [ (pName + "." + distName + "." + dp, dv) for dp,dv in defaultDistParams ]

                    newConfig[newConfName]['info'] = distTypes[distName]['info']


    return newConfig

def processConfigPart(cfg, userConfig, configNames, requiredKeys):

    params = cfg['params']
    deps = cfg['depends']

    # Check if we need to process dependencies
    if deps is not None:

        depObjName = deps[0]
        depObj = configNames[depObjName]
        depKey = deps[1]
        depVal = deps[2]
        
        #print "processConfigPart", depObjName
        #pprint.pprint(depObj)
        if not processConfigPart(depObj, userConfig, configNames, requiredKeys):
            # Parent dependency not fulfilled, so this one isn't either
            return False
        #print "done: processConfigPart", depObjName

        if not depKey in userConfig:
            pprint.pprint(userConfig)
            raise Exception("Key %s was not set" % depKey)
        
        if userConfig[depKey] != depVal:
            return False # Dependency not fulfilled

        for k in params:
            if len(k) == 3:
                requiredKeys[k[0]] = k[2]
            else:
                requiredKeys[k[0]] = None

    for p in params:

        key = p[0]
        val = p[1]

        if len(p) == 3:

            requiredKeys[key] = p[2]
        else:
            requiredKeys[key] = None

        # See if we should check defaults
        if not key in userConfig: 
            #if val is None:
            #    raise Exception("Key %s is not set" % key)

            userConfig[key] = val
        
    return True

def createConfigLines(inputConfig, checkNone = True, ignoreKeys = [ ]):
    userConfig = copy.deepcopy(inputConfig)
    configNames = getExpandedSettingsOptions()

    requiredKeys = { }

    for n in configNames:
        cfg = configNames[n]
        #print "createConfigLines", n
        #print cfg
        processConfigPart(cfg, userConfig, configNames, requiredKeys)

    for k in userConfig:
        if not k in requiredKeys:
            raise Exception("Encountered unknown key %s" % k)

        val = userConfig[k]

        possibleValues = requiredKeys[k]
        if possibleValues is not None:
            if not val in possibleValues:
                raise Exception("Value '%s' for key %s is not allowed, should be one of %s" % (val, k, possibleValues))

        if checkNone:
            if val is None:
                raise Exception("Key %s is not set" % k)

    # Display the final config file

    lines = [ ]
    unusedlines = [ ]
    # In principle this should contain the same info as userConfig at the end,
    # but we'll introduce some ordering here so we can feed it back to R in a better
    # way
    resultingConfig = [ ] 

    names = [ key for key in configNames ]
    names.sort()
    for key in names:
        deps = configNames[key]["depends"]
        params = configNames[key]["params"]
        info = configNames[key]["info"]

        usedparams = [ ]
        unusedparams = [ ]
        for p in params:
            k = p[0]
            if k in requiredKeys:
                
                v = userConfig[k]
                ns = 60-len(k)
                k += " "*ns

                if len(p) == 3: # Limited number of possibilities
                    usedparams.append("# Valid values are: %s" % str(p[2]))

                if v is None:
                    usedparams.append("%s = " % k)
                elif type(v) == float:
                    usedparams.append("%s = %.15g" % (k, v))
                elif type(v) == int:
                    usedparams.append("%s = %d" % (k, v))
                else:
                    usedparams.append("%s = %s" % (k, str(v)))

                idx = len(resultingConfig)+1

                if v is None:
                    resultingConfig.append((idx, p[0], ""))
                else:
                    resultingConfig.append((idx, p[0], v))

            else:
                unusedparams.append("# " + p[0])

        if usedparams:
            if deps:
                lines += [ "# The following depends on %s = %s" % (deps[1], deps[2]) ]

            if info:
                lines += [ "# " + l for l in info.splitlines() ]

            lines += usedparams
            lines += [ "" ]

        if unusedparams:
            if deps:
                unusedlines += [ "# The following depends on %s = %s" % (deps[1], deps[2]) ]

            unusedlines += unusedparams
            unusedlines += [ "#" ]

    introlines = [ "# Note: the configuration file format is very simple, it is",
                   "# just a set of \"key = value\" lines. Lines that start with a '#'",
                   "# sign are treated as comments and are ignored. No calculations",
                   "# can be done in the file, so instead of writing 1.0/2.0, you'd ",
                   "# need to write 0.5 for example.",
                   ""
                 ]
    return (userConfig, introlines + unusedlines + [ "" ] + lines, resultingConfig)

def checkKnownKeys(keyList):

    configNames = getExpandedSettingsOptions()
    allKnownKeys = [ ]
    for n in configNames:
        paramList = configNames[n]["params"]
        paramKeys = [ ]
        for p in paramList:
            paramKeys.append(p[0])

        allKnownKeys += paramKeys

    for k in keyList:
        if not k in allKnownKeys:
            raise Exception("Encountered unknown key '%s'" % k)

def main():

    # Read the input

    userConfig = { }
    line = sys.stdin.readline()
    while line:

        line = line.strip()
        if line:
            parts = [ p.strip() for p in line.split('=') ]
            
            key, value = parts[0], parts[1]
            userConfig[key] = value

        line = sys.stdin.readline()

    # In principle, the 'resultingConfigNotNeeded' should contain the same things
    # as finalConfig, but some ordering was introduced
    (finalConfig, lines, resultingConfigNotNeeded) = createConfigLines(userConfig, False)

    lines.append('')
    sys.stdout.write('\n'.join(lines))


if __name__ == "__main__":
    main()


