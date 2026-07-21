from re import X
import matplotlib.pyplot as plt
from matplotlib.ticker import StrMethodFormatter

import pandas as pd
import numpy as np

folder = "..\\implementation\\results\\"
#outputFolder = "..\\..\\..\\thesis\\MainMatter\\paper3\\graphics\\graphs\\"
outputFolder = "..\\writeup\\figures\\graphs\\"


DSNameMapping = {
    "AAS1DModeReportDS" : "Report",
    "AAS1DModeRangeTreesDS" : "RangeTrees",
    "AAS1DModeChanDS" : "Blocks",
    "AAS1DModeGridDS" : "Grid",
    "AAS1DModeArrangementDS" : "Cutting",
    "AAS2DModeReportDS" : "2D Report",
    "AAS2DModePerColorDS" : "2D RangeTrees",
    "AAS2DModeGridDS" : "2D Grid",
    "Amc::GridSelectColor" : "Grid-relevantColors",
    }

DScolors = {
    "AAS1DModeReportDS" : "red",
    "AAS1DModeRangeTreesDS" : "lightgreen",
    "AAS1DModeChanDS" : "c",
    "AAS1DModeGridDS" : "orange",
    "AAS1DModeArrangementDS" : "y",
    "AAS2DModeReportDS" : "red",
    "AAS2DModePerColorDS" : "lightgreen",
    "AAS2DModeGridDS" : "orange",
    "Amc::GridSelectColor" : "darkblue",
    }



parameterMapping = {
    "numPoints" : "n",
    "s" : "s",
    "numColors" : "Φ",
    "alpha" : "α"
    }

measureMapping = {
    "modeQueryTimeExcludingFirst" : "Query time",
    "modeSpace" : "Space usage",
    "modeBuildTime": "Build time"}




make1Dzoom                  = False
make1Dfull                  = False
make2Dfull                  = False
make2DfullGridDetail        = False
make2DfullRangeTreesDetail  = False
makeKdependency             = False
makeSdependency             = False
makePhiDependency           = True
makeAlphaDependency         = True
makeRealDataset             = False


show = True


def makefigure(inputFile, DS, x, measure, outputFile="", logscale = False, scientific=False):  
    plt.figure()

    ax = plt.gca()

    df = pd.read_csv(folder + inputFile + ".csv", sep=';')
    subset = df

    subset = subset[subset["DSType"] == DS]

    #use continuous color spectrum
    ks = sorted(subset["k"].unique())
    cmap = plt.get_cmap("plasma")      # or "viridis", "plasma", "cividis", "magma"
    colors = cmap(np.linspace(0, 1, len(ks)))
    color_map = dict(zip(ks, colors))

    if (measure == "modeQueryTime" or measure == "modeQueryTimeExcludingFirst"):
        for k in ks:
            group = subset[subset["k"] == k]
            plt.plot(group[x], group[measure], 'o-', color=color_map[k], label=f"$k = {k:,.0f}$")
    else:
        plt.plot(group[x], group[measure], 'o-')


    if(scientific):
        ax.xaxis.set_major_formatter(StrMethodFormatter('{x:,.0f}'))
        ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
        ax.yaxis.get_major_formatter().set_useMathText(True)
    if(logscale):
        ax.set_yscale('log')

    plt.xlabel(parameterMapping[x])
    if (measure == "modeQueryTimeExcludingFirst"):
        plt.legend()
    if(outputFile == ""):
        outputFile = inputFile + "_" + DSs[0] + "_(" + x + "," + measure + ").pdf"
        outputFile = "".join(filter(lambda c: c not in " ?!/;:", outputFile))

    plt.savefig(outputFolder + outputFile, bbox_inches='tight')
    if(show):
        plt.show()
    plt.close()
    print("created file " + outputFile)

#1D figure zoomed in on n <= 100k to show cutting is bad, for both query time and space
# if make1Dzoom:
#     for measure in ["modeQueryTimeExcludingFirst", "modeSpace", "modeBuildTime"]:
#         plt.figure()
#         ax = plt.gca()
#         df = pd.read_csv(folder + "1DincreasingNsmall.csv", sep=';')
#         subset = df

#         subset = subset[subset["numPoints"] <= 100000]
  
#         for DS in ["AAS1DModeReportDS", "AAS1DModeRangeTreesDS", "AAS1DModeChanDS", "AAS1DModeGridDS", "AAS1DModeArrangementDS"]:
#             DSsubset = subset[subset["DSType"] == DS]
        
#             if(measure=="modeQueryTimeExcludingFirst"):
#                 k = 10000
#                 DSsubset = DSsubset[DSsubset["k"] == k]
#             plt.plot(DSsubset["numPoints"], DSsubset[measure], 'o-', color=DScolors[DS], label=DSNameMapping[DS])

#         #use scientific notation
#         ax.xaxis.set_major_formatter(StrMethodFormatter('{x:,.0f}'))
#         ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
#         ax.yaxis.get_major_formatter().set_useMathText(True)

#         ax.set_yscale('log')

#         plt.xlabel("n")
#         plt.ylabel(measureMapping[measure])
#         plt.legend()
#         outputFile = "1D_zoomedComparison_" + measure + ".pdf"

#         plt.savefig(outputFolder + outputFile, bbox_inches='tight')
#         if(show):
#             plt.show()
#         plt.close()
#         print("created file " + outputFile)



#1D full figure of query times and space
if make1Dfull:
    for measure in ["modeQueryTimeExcludingFirst", "modeSpace", "modeBuildTime"]:
        plt.figure()
        ax = plt.gca()
        df = pd.read_csv(folder + "1DincreasingN.csv", sep=';')
        subset = df
  
        for DS in ["AAS1DModeReportDS", "AAS1DModeRangeTreesDS", "AAS1DModeChanDS", "AAS1DModeGridDS", "AAS1DModeArrangementDS"]:
            DSsubset = subset[subset["DSType"] == DS]
        
            if(measure=="modeQueryTimeExcludingFirst"):
                k = 100000
                DSsubset = DSsubset[DSsubset["k"] == k]
            plt.plot(DSsubset["numPoints"], DSsubset[measure], 'o-', color=DScolors[DS], label=DSNameMapping[DS])

        #use scientific notation
        ax.xaxis.set_major_formatter(StrMethodFormatter('{x:,.0f}'))
        ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
        ax.yaxis.get_major_formatter().set_useMathText(True)

        ax.set_yscale('log')

        ax.set_xlim((None, None))
        ax.set_ylim((None,None))

        plt.xlabel("n")
        plt.ylabel(measureMapping[measure])
        plt.legend()
        outputFile = "1D_fullComparison_" + measure + ".pdf"

        plt.savefig(outputFolder + outputFile, bbox_inches='tight')
        if(show):
            plt.show()
        plt.close()
        print("created file " + outputFile)



#2D full comparison, including range query
if make2Dfull:
    for measureCombi in [("modeQueryTimeExcludingFirst","rangeQueryTime"), ("modeSpace","rangeSpace")]:
        measure, rangeMeasure = measureCombi
        plt.figure()
        ax = plt.gca()
        df = pd.read_csv(folder + "2D_increasingN.csv", sep=';')
        subset = df
  
        if(measure == "modeQueryTimeExcludingFirst"):
            k = 100000
            subset = subset[subset["k"] == k]

        for DS in ["AAS2DModeReportDS", "AAS2DModePerColorDS", "AAS2DModeGridDS", "Amc::GridSelectColor"]:
            DSsubset = subset[subset["DSType"] == DS]
       
            plt.plot(DSsubset["numPoints"], DSsubset[measure], 'o-', color=DScolors[DS], label=DSNameMapping[DS])
    
        subset = subset[subset["DSType"] == "AAS2DModeReportDS"]#arbitrary choice
        plt.plot(subset["numPoints"], subset[rangeMeasure], 'o-', color="brown", label="Range query")

        #use scientific notation
        ax.xaxis.set_major_formatter(StrMethodFormatter('{x:,.0f}'))
        ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
        ax.yaxis.get_major_formatter().set_useMathText(True)

        ax.set_yscale('log')

        ax.set_xlim((None, None))
        ax.set_ylim((None, None))

        plt.xlabel("n")
        plt.ylabel(measureMapping[measure])
        plt.legend()
        outputFile = "2D_fullComparison_" + measure + ".pdf"

        plt.savefig(outputFolder + outputFile, bbox_inches='tight')
        if(show):
            plt.show()
        plt.close()
        print("created file " + outputFile)



if(make2DfullGridDetail):
    makefigure("2D_increasingN", "AAS2DModeGridDS", "numPoints", "modeQueryTimeExcludingFirst", "2D_fullGridDetail.pdf")


if(make2DfullRangeTreesDetail):
    makefigure("2D_increasingN", "AAS2DModePerColorDS", "numPoints", "modeQueryTimeExcludingFirst", "2D_fullRangeTreesDetail.pdf")


#2D k dependency
if makeKdependency:
    plt.figure()
    ax = plt.gca()
    df = pd.read_csv(folder + "2D_increasingN.csv", sep=';')
    subset = df
  
    for DS in ["AAS2DModeReportDS", "AAS2DModePerColorDS", "AAS2DModeGridDS", "Amc::GridSelectColor"]:
        DSsubset = subset[subset["DSType"] == DS]
        
        n = 500000
        DSsubset = DSsubset[DSsubset["numPoints"] == n]
        plt.plot(DSsubset["k"], DSsubset["modeQueryTimeExcludingFirst"], 'o-', color=DScolors[DS], label=DSNameMapping[DS])

    #use scientific notation
    ax.xaxis.set_major_formatter(StrMethodFormatter('{x:,.0f}'))
    ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    ax.yaxis.get_major_formatter().set_useMathText(True)

    #ax.set_yscale('log')

    ax.set_xlim((None, None))
    ax.set_ylim((None,2e7))

    plt.xlabel("k")
    plt.ylabel(measureMapping["modeQueryTimeExcludingFirst"])
    plt.legend()
    outputFile = "2D_k_dependency.pdf"

    plt.savefig(outputFolder + outputFile, bbox_inches='tight')
    if(show):
        plt.show()
    plt.close()
    print("created file " + outputFile)




#2D s dependency
if makeSdependency:
    plt.figure()
    ax = plt.gca()
    df = pd.read_csv(folder + "2D_increasingS.csv", sep=';')
    subset = df

    rangeTreesDf = pd.read_csv(folder + "2D_increasingN.csv", sep=';')
    rangeTreesDf = rangeTreesDf[rangeTreesDf["DSType"] == "AAS2DModePerColorDS"]
    rangeTreesDf = rangeTreesDf[rangeTreesDf["numPoints"] == 250000]
    rangeTreesDf = rangeTreesDf[rangeTreesDf["k"] == 10]
    rangeTreesSpace = rangeTreesDf["modeSpace"].values[0]

    DSsubset = subset[subset["DSType"] == "AAS2DModeGridDS"]
       
    plt.plot(DSsubset["s"], DSsubset["modeSpace"] - rangeTreesSpace, 'o-', color=DScolors["AAS2DModeGridDS"], label="S-dependent space")
    plt.axhline(y=rangeTreesSpace, linestyle='-', color=DScolors["AAS2DModePerColorDS"], label="S-independent space")


    #use scientific notation
    ax.xaxis.set_major_formatter(StrMethodFormatter('{x:,.0f}'))
    ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    ax.yaxis.get_major_formatter().set_useMathText(True)

    #ax.set_yscale('log')

    ax.set_xlim((None, None))
    ax.set_ylim((None, None))

    plt.xlabel("s")
    plt.ylabel(measureMapping["modeSpace"])
    plt.legend()
    outputFile = "2D_sDependency.pdf"

    plt.savefig(outputFolder + outputFile, bbox_inches='tight')
    if(show):
         plt.show()
    plt.close()
    print("created file " + outputFile)


if (makePhiDependency):
    for DS in ["AAS2DModeGridDS","AAS2DModePerColorDS"]:
        plt.figure()

        ax = plt.gca()

        df = pd.read_csv(folder + "2D_increasingPhi.csv", sep=';')
        subset = df

        subset = subset[subset["DSType"] == DS]

        #use continuous color spectrum
        ks = sorted(subset["k"].unique())
        #ks = allks[kvalues]
        cmap = plt.get_cmap("plasma")      # or "viridis", "plasma", "cividis", "magma"
        colors = cmap(np.linspace(0, 1, len(ks)))
        color_map = dict(zip(ks, colors))


        for k in ks:
            group = subset[subset["k"] == k]
            plt.plot(group["numColors"], group["modeQueryTimeExcludingFirst"], 'o-', color=color_map[k], label=f"$k = {k:,.0f}$")


        ax.xaxis.set_major_formatter(StrMethodFormatter('{x:,.0f}'))
        ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
        ax.yaxis.get_major_formatter().set_useMathText(True)

        #ax.set_yscale('log')

        plt.xlabel(parameterMapping["numColors"])
        plt.ylabel(measureMapping["modeQueryTimeExcludingFirst"])
        plt.legend()
        outputFile = "2D_phiDependency_" + DS + ".pdf"

        plt.savefig(outputFolder + outputFile, bbox_inches='tight')
        if(show):
            plt.show()
        plt.close()
        print("created file " + outputFile)


if (makeAlphaDependency):
    for DS in ["AAS2DModeGridDS","AAS2DModePerColorDS"]:

        plt.figure()

        ax = plt.gca()

        df = pd.read_csv(folder + "2D_grouped.csv", sep=';')
        subset = df

        subset = subset[subset["DSType"] == DS]

        #use continuous color spectrum
        ks = sorted(subset["k"].unique())
        #ks = allks[kvalues]
        cmap = plt.get_cmap("plasma")      # or "viridis", "plasma", "cividis", "magma"
        colors = cmap(np.linspace(0, 1, len(ks)))
        color_map = dict(zip(ks, colors))


        for k in ks:
            group = subset[subset["k"] == k]
            plt.plot(group["alpha"], group["modeQueryTimeExcludingFirst"], 'o-', color=color_map[k], label=f"$k = {k:,.0f}$")


        # ax.xaxis.set_major_formatter(StrMethodFormatter('{x:,.0f}'))
        # ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
        # ax.yaxis.get_major_formatter().set_useMathText(True)

        #ax.set_yscale('log')

        plt.xlabel(parameterMapping["alpha"])
        plt.ylabel(measureMapping["modeQueryTimeExcludingFirst"])
        plt.legend()
        outputFile = "2D_alphaDependency_" + DS + ".pdf"

        plt.savefig(outputFolder + outputFile, bbox_inches='tight')
        if(show):
            plt.show()
        plt.close()
        print("created file " + outputFile)



if (makeRealDataset):
    for file in ["2D_bbg","2D_bbgSynthetic","2D_bbgSyntheticGrouped", ]:
    #for file in ["2D_bbg"]:
        plt.figure()

        ax = plt.gca()

        df = pd.read_csv(folder + file + ".csv", sep=';')
        subset = df

        subset = subset[subset["DSType"] == "AAS2DModeGridDS"]
        #subset = subset[subset["DSType"] == "AAS2DModePerColorDS"]
        #subset = subset[subset["DSType"] == "Amc::GridSelectColor"]

        #use continuous color spectrum
        ks = sorted(subset["k"].unique())
        #ks = allks[kvalues]
        cmap = plt.get_cmap("plasma")      # or "viridis", "plasma", "cividis", "magma"
        colors = cmap(np.linspace(0, 1, len(ks)))
        color_map = dict(zip(ks, colors))


        for k in ks:
            group = subset[subset["k"] == k]
            plt.plot(group["numPoints"], group["modeQueryTimeExcludingFirst"], 'o-', color=color_map[k], label=f"$k = {k:,.0f}$")


        ax.xaxis.set_major_formatter(StrMethodFormatter('{x:,.0f}'))
        ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
        ax.yaxis.get_major_formatter().set_useMathText(True)

        #ax.set_yscale('log')

        plt.xlabel(parameterMapping["numPoints"])
        plt.ylabel(measureMapping["modeQueryTimeExcludingFirst"])
        plt.legend()
        outputFile = file + ".pdf"

        plt.savefig(outputFolder + outputFile, bbox_inches='tight')
        if(show):
            plt.show()
        plt.close()
        print("created file " + outputFile)



# #1D uniform increasing n
# makefigure("1DincreasingN",["AAS1DModeReportDS"], "numPoints", "modeQueryTimeExcludingFirst",None)
# makefigure("1DincreasingN",["AAS1DModeRangeTreesDS"], "numPoints", "modeQueryTimeExcludingFirst","1D",(0,100000))
# makefigure("1DincreasingN",["AAS1DModeChanDS"], "numPoints", "modeQueryTimeExcludingFirst","1D",(0,100000))
# makefigure("1DincreasingN",["AAS1DModeGridDS"], "numPoints", "modeQueryTimeExcludingFirst","1D",(0,100000))
# makefigure("1DincreasingN",["AAS1DModeArrangementDS"], "numPoints", "modeQueryTimeExcludingFirst",None,(0,100000))

# #1D uniform increasing s
#figures.append(Figure("1DincreasingS","AAS1DModeChanDS", "s"))
#figures.append(Figure("1DincreasingS","AAS1DModeGridDS", "s"))
#figures.append(Figure("1DincreasingS","AAS1DModeArrangementDS", "s"))


# #1D uniform increasing phi
# figures.append(Figure("1DincreasingPhi","AAS1DModeReportDS", "numColors"))
# figures.append(Figure("1DincreasingPhi","AAS1DModeRangeTreesDS", "numColors"))
# figures.append(Figure("1DincreasingPhi","AAS1DModeChanDS", "numColors"))
# figures.append(Figure("1DincreasingPhi","AAS1DModeGridDS", "numColors"))
# figures.append(Figure("1DincreasingPhi","AAS1DModeArrangementDS", "numColors"))

# #1D grouped increasing alpha
# figures.append(Figure("1DgroupedIncreasingAlpha","AAS1DModeReportDS", "alpha"))
# figures.append(Figure("1DgroupedIncreasingAlpha","AAS1DModeRangeTreesDS", "alpha"))
# figures.append(Figure("1DgroupedIncreasingAlpha","AAS1DModeChanDS", "alpha"))
# figures.append(Figure("1DgroupedIncreasingAlpha","AAS1DModeGridDS", "alpha"))
# figures.append(Figure("1DgroupedIncreasingAlpha","AAS1DModeArrangementDS", "alpha"))




#2D uniform increasing n
#for measure in ["modeQueryTimeExcludingFirst","modeSpace","modeBuildTime"]:
# makefigure("2D_increasingN","AAS2DModeReportDS", "numPoints", "modeQueryTimeExcludingFirst", 0, 7e7)
# makefigure("2D_increasingN","AAS2DModePerColorDS", "numPoints", "modeQueryTimeExcludingFirst", 0, 7e7)
# makefigure("2D_increasingN","AAS2DModeGridDS", "numPoints", "modeQueryTimeExcludingFirst", 0, 7e7)
# makefigure("2D_increasingN","Amc::GridSelectColor", "numPoints", "modeQueryTimeExcludingFirst", 0, 7e7)

##2D uniform increasing s
#figures.append(Figure("2D_increasingS","Amc::GridNoColor", "s"))
#figures.append(Figure("2D_increasingS","Amc::GridSelectColor", "s"))
#figures.append(Figure("2D_increasingS","Amc::GridColumnColor", "s"))
#figures.append(Figure("2D_increasingS","Amc::GridSortedColor", "s"))

##2D uniform increasing phi
## figures.append(Figure("2D_increasingPhi","AAS2DModeReportDS", "numColors"))
## figures.append(Figure("2D_increasingPhi","AAS2DModePerColorDS", "numColors"))
#figures.append(Figure("2D_increasingPhi","Amc::GridNoColor", "numColors"))
#figures.append(Figure("2D_increasingPhi","Amc::GridColumnColor", "numColors"))
#figures.append(Figure("2D_increasingPhi","Amc::GridSelectColor", "numColors"))
#figures.append(Figure("2D_increasingPhi","Amc::GridSortedColor", "numColors"))

#2D grouped increasing alpha
# figures.append(Figure("2D_grouped","AAS2DModeReportDS", "alpha"))
# figures.append(Figure("2D_grouped","AAS2DModePerColorDS", "alpha"))
# figures.append(Figure("2D_grouped","Amc::GridNoColor", "alpha"))
# figures.append(Figure("2D_grouped","Amc::GridColumnColor", "alpha"))
# figures.append(Figure("2D_grouped","Amc::GridSelectColor", "alpha"))
# figures.append(Figure("2D_grouped","Amc::GridSortedColor", "alpha"))

# #2D real data walking
# figures.append(Figure("2D_realWalking","AAS2DModeReportDS", "numPoints"))
# figures.append(Figure("2D_realWalking","AAS2DModePerColorDS", "numPoints"))
# figures.append(Figure("2D_realWalking","Amc::GridNoColor", "numPoints"))
# figures.append(Figure("2D_realWalking","Amc::GridColumnColor", "numPoints"))
# figures.append(Figure("2D_realWalking","Amc::GridSelectColor", "numPoints"))
# figures.append(Figure("2D_realWalking","Amc::GridSortedColor", "numPoints"))

# #2D real data ransom
# figures.append(Figure("2D_realRansom","AAS2DModeReportDS", "numPoints"))
# figures.append(Figure("2D_realRansom","AAS2DModePerColorDS", "numPoints"))
# figures.append(Figure("2D_realRansom","Amc::GridNoColor", "numPoints"))
# figures.append(Figure("2D_realRansom","Amc::GridColumnColor", "numPoints"))
# figures.append(Figure("2D_realRansom","Amc::GridSelectColor", "numPoints"))
# figures.append(Figure("2D_realRansom","Amc::GridSortedColor", "numPoints"))

# #2D real bbg
# figures.append(Figure("2D_bbg","AAS2DModeReportDS", "numPoints"))
# figures.append(Figure("2D_bbg","AAS2DModePerColorDS", "numPoints"))
# figures.append(Figure("2D_bbg","Amc::GridNoColor", "numPoints"))
# figures.append(Figure("2D_bbg","Amc::GridColumnColor", "numPoints"))
# figures.append(Figure("2D_bbg","Amc::GridSelectColor", "numPoints"))
# figures.append(Figure("2D_bbg","Amc::GridSortedColor", "numPoints"))

# #2D real bieleveld
# figures.append(Figure("2D_bieleveld","AAS2DModeReportDS", "numPoints"))
# figures.append(Figure("2D_bieleveld","AAS2DModePerColorDS", "numPoints"))
# figures.append(Figure("2D_bieleveld","Amc::GridNoColor", "numPoints"))
# figures.append(Figure("2D_bieleveld","Amc::GridColumnColor", "numPoints"))
# figures.append(Figure("2D_bieleveld","Amc::GridSelectColor", "numPoints"))
# figures.append(Figure("2D_bieleveld","Amc::GridSortedColor", "numPoints"))

# #2D real sciencePark
# figures.append(Figure("2D_sciencePark","AAS2DModeReportDS", "numPoints"))
# figures.append(Figure("2D_sciencePark","AAS2DModePerColorDS", "numPoints"))
# figures.append(Figure("2D_sciencePark","Amc::GridNoColor", "numPoints"))
# figures.append(Figure("2D_sciencePark","Amc::GridColumnColor", "numPoints"))
# figures.append(Figure("2D_sciencePark","Amc::GridSelectColor", "numPoints"))
# figures.append(Figure("2D_sciencePark","Amc::GridSortedColor", "numPoints"))



