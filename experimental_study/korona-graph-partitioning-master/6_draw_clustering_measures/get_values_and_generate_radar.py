#!/usr/bin/env python

import sys, pygal
from pygal.style import TurquoiseStyle
from pygal.style import Style
from os import listdir
from os.path import isfile, join

class Cmeasure:
    def __init__(self, conductance, coverage, modularity, total_cut, performance):
        self.conductance = conductance
        self.coverage = coverage
        self.modularity = modularity
        self.performance = performance
        self.total_cut = total_cut

    def __str__(self):
        return str(self.conductance)+","+str(self.coverage)+","+str(self.modularity)+","+str(self.performance)+","+str(self.total_cut)

def get_percentil_from_file(filename):
    tok = filename.split(".")
    percentile = ((tok[0].split("_"))[-1])[-2:]
    if not percentile.isdigit():
        print("Error getting the percentile")
        sys.exit(1)
    return percentile
    
def get_solver_results(dirname):
    files = [f for f in listdir(dirname) if isfile(join(dirname, f))]
    total_results = {}
    for f in files:
        percentile = get_percentil_from_file(f)
        total_results[percentile] = get_solver_cluster_measure(join(dirname, f))
    return total_results
    
def get_solver_cluster_measure(filename):
    conductance = None
    coverage = None
    modularity = None
    performance = None
    total_cut = None
    with open(filename) as fd:
        for line in fd:
            if "Average conductance:" in line:
                conductance = float( ((line.strip("\n")).split("\t"))[1] )
            elif "Coverage:" in line: 
                coverage = float( ((line.strip("\n")).split("\t"))[1] )
            elif "Modularity:" in line:
                modularity  = float( ((line.strip("\n")).split("\t"))[1] )
            elif "Total cut:" in line:
                total_cut = float( ((line.strip("\n")).split("\t"))[1] )
            elif "Performance:" in line:
                performance = float( ((line.strip("\n")).split("\t"))[1] )
            else:
                pass
    if conductance != None and coverage != None and modularity != None and performance != None and total_cut != None:
        cresults = Cmeasure(conductance, coverage, modularity, total_cut, performance)
    else:
        print("Error getting the clustering measure")
        sys.exit(1)
    return cresults

def get_all_total_weight(dirname):
    files = [f for f in listdir(dirname) if (isfile(join(dirname, f)) and "matrix_p" in f)]
    total_cut = {}
    for f in files:
        percentile = get_percentil_from_file(f)
        total_cut[percentile] = get_total_weight(join(dirname, f))
    return total_cut

def get_total_weight(filename):
    s = 0
    total = []
    with open(filename) as f:
        for line in f:
            line = line.rstrip()
            tok = line.split(",")
            l = [float(x) for x in tok]
            s = s + sum(l)
    return s

def draw_radar(semep, metis, percentile, dirname):
    title = "Percentile "+percentile
    custom_style = Style(
        background='transparent',
        plot_background='transparent',
        #foreground='#53E89B',
        #foreground_strong='#53A0E8',
        #foreground_subtle='#630C0D',
        #opacity='.6',
        #opacity_hover='.9',
        transition='400ms ease-in',
        #colors=('#0093AF','#FFAE64')
        colors=('#FF7C00', '#006C80'))
    
    radar_chart = pygal.Radar(style=custom_style,height=400)
    radar_chart.fill = True
    radar_chart.title = title 
    radar_chart.x_labels = ['Inv. Conductance', 'Coverage', 'Norm. Modularity', 'Inv. Norm. Total Cut', 'Performance']
    radar_chart.add('Korona_SemEP', [ semep.conductance, semep.coverage, semep.modularity, semep.total_cut, semep.performance ])
    radar_chart.add('Korona_Metis', [ metis.conductance, metis.coverage, metis.modularity, metis.total_cut, metis.performance ])
    radar_chart.render()
    filename = "radar_chart_"+percentile+".svg"
    radar_chart.render_to_file(join(dirname, filename))

def normalization(c_results, max_cut):
    c_results.conductance = 1.0 - c_results.conductance
    c_results.modularity = (c_results.modularity + 0.5)/1.5 
    c_results.total_cut = 1.0 - c_results.total_cut/max_cut 
        
def get_all_results(solver, maxcut):
    for percentile in solver:
        normalization(solver[percentile], maxcut[percentile])

if __name__ == "__main__":
    semep = get_solver_results(sys.argv[1])
    metis = get_solver_results(sys.argv[2])
    if (len(semep) != len(metis)) or (semep.keys() != metis.keys()):
        print("Error, number of results in the solver are different")
        sys.exit(1)
    maxcut = get_all_total_weight(sys.argv[3])
    if (len(maxcut) != len(metis)) or (maxcut.keys() != metis.keys()):
        print("Error, the percentiles are diferent")
        sys.exit(1)
    get_all_results(semep, maxcut)
    get_all_results(metis, maxcut)
    for percentile in semep:
        draw_radar(semep[percentile], metis[percentile], percentile, sys.argv[4])
    print("Done")
