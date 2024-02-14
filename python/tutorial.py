import pysimpactcyan
simpact = pysimpactcyan.PySimpactCyan()

data = { }
myMatrix = [ [1, 3], [2, 4] ]
data["csvMatrix"] = myMatrix

cfg = { }
cfg["person.geo.dist2d.type"] = "discrete"
cfg["person.geo.dist2d.discrete.densfile"] = "data:csvMatrix"

simpact.run(cfg, "/tmp/simpacttest", dataFiles=data)