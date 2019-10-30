from __future__ import division
from openpyxl import Workbook
from array import array
#from openpyxl.utils import coordinate_from_string, column_index_from_string
from openpyxl.utils import column_index_from_string
from openpyxl.utils.cell import coordinate_from_string
from openpyxl.utils import get_column_letter
import urllib2,re
from bs4 import BeautifulSoup
import urllib,sys
from urllib2 import Request, urlopen, URLError
import openpyxl

reload(sys)
sys.setdefaultencoding('UTF-8')

InputXlsxFile = "output/papers_authors.xlsx"   #input file containing data to be processed.
AuthorkeyFile = "output/author-key map.txt"    #reference output file for indexing authors
AuthorListFile = "output/author-list.txt"

wb = openpyxl.load_workbook(InputXlsxFile)
sh = wb.get_active_sheet()
exceldata = []

# Here we calculate the similarity of two conferences which builds up the similarity matrix of conferences.
def SimilarityOfConf(Conf1, Conf2):
    if (Conf1 == Conf2):
        return float(1.0)
    Conf1Auth = []
    Conf2Auth = []
    for counter in range(len(exceldata)):
        num_author = exceldata[counter][2]
        year = exceldata[counter][3]
        if (year == Conf1):
            for num in range (1,num_author +1):
                auth_link = exceldata[counter][3+num]
                Conf1Auth.append(auth_link)
        if (year == Conf2):
            for num in range (1,num_author +1):
                auth_link = exceldata[counter][3+num]
                Conf2Auth.append(auth_link)
    containsBoth = set(Conf1Auth).intersection(Conf2Auth)
    containsEither = list(set(Conf1Auth).union(Conf2Auth))
    similarity = float(len(containsBoth)/ len(containsEither))
    return similarity

                                                    #################******~~~~Execution starts here~~~~*******#######################

f_auth = open("output/Author.txt", 'w+')
f_conf = open("output/Conf.txt",'w+')
f_conf_matrix = open("output/Conf_matrix.txt",'w+')
f_auth_conf_graph = open("output/Auth-Conf_graph.txt",'w+')

f = open(AuthorkeyFile, 'w+')
#g = open(OPMetisFile, 'w+')
h = open(AuthorListFile, 'w+')

counter=0   #row pointer of the input file.
author_count=0  #total number of authors.
author_dict = {}    #list of all the authors.
conf_list = []      #list of all the conferences.

for r in sh.rows:   #loop over each line of input file
    eachrow = []
    counter+=1      #point to the next line of the input file
    if (counter==1):    #First row is header row, so skip.
        continue
    num_author= int(r[2].value)     #Get number of authors for the paper represented in that row.
    if (num_author > 0):       # If there is atleast one entry of authorship for that paper.
        year = int(r[3].value)
        if year not in conf_list:
            conf_list.append(year)
        eachrow.extend([int(r[0].value),str(r[1].value.decode('utf-8')),num_author, int(r[3].value)])
        for num in range(1,num_author+1):       #For each author of that paper
            auth_link = str(r[3+num].value)    #get the author's URI.
            eachrow.append(auth_link)
            if auth_link not in author_dict:    #If author not already in the author's dictionary then add him/her.
                author_count+=1     #update author count
                author_dict[auth_link] = author_count
    exceldata.append(eachrow)
conf_list.sort(key=int)

#Here we save the Author Vertices File.
BPGAuthorFile = ""
for key, value in sorted(author_dict.iteritems(), key=lambda (k,v): (v,k)):
    f.write("%s: %s\n" % (key, value))      #Print the entire author key mapping onto the AuthorKey file.
    BPGAuthorFile += "\n" + "A" + str(value)
    h.write(key)
    h.write("\n")
BPGAuthorFile = str((author_count)) + BPGAuthorFile
f_auth.write(BPGAuthorFile)
del BPGAuthorFile
h.close()
f.close()
f_auth.close()

#Here we save the Conference Vertices File.
BPGconfFile = ""
for conf in conf_list:
    BPGconfFile += "\n" + "C" + str(conf)
BPGconfFile = str(len(conf_list)) + BPGconfFile
f_conf.write(BPGconfFile)
del BPGconfFile
f_conf.close()

#Here we save the Conference Similarity Matrix File.
BPGraph_ConfSimilarity = [[0 for x in range(len(conf_list))] for y in range(len(conf_list))]

for conf1 in conf_list:
    for conf2 in conf_list:
        if (conf2 >= conf1):
            similarityValue = SimilarityOfConf(conf1,conf2)
            BPGraph_ConfSimilarity[conf1-2001][conf2-2001] = similarityValue
            BPGraph_ConfSimilarity[conf2-2001][conf1-2001] = BPGraph_ConfSimilarity[conf1-2001][conf2-2001]
BPGraph_string = ""
for i in range(len(conf_list)):
    for j in range(len(conf_list)):
        BPGraph_string +=  str(BPGraph_ConfSimilarity[i][j]) + " "
    BPGraph_string = BPGraph_string[:-1]
    BPGraph_string += "\n"
BPGraph_string = str(len(conf_list)) + "\n" + BPGraph_string
f_conf_matrix.write(BPGraph_string)
del BPGraph_ConfSimilarity
f_conf_matrix.close()

#Edges from authors to conferences. we find the weight of the edges depending on the number of times an author published in each conference.
BPGraph_dict=[[0 for x in range(len(conf_list))] for y in range(author_count)]  #A Matrix of size equal to total number of authors by the number of conferences.
for counter in range(len(exceldata)):
    num_author = exceldata[counter][2]
    year = exceldata[counter][3]
    for num in range (1,num_author +1):
        auth_link = exceldata[counter][3+num]
        auth_value = author_dict.get(auth_link)
        auth_year = year - 2001
        BPGraph_dict[auth_value-1][auth_year] += 1
edgesInBPG = 0
BPGdata = ""
for auth_counter in range(author_count):
    for year_counter in range(len(conf_list)):
        BPGvalue = BPGraph_dict[auth_counter][year_counter]
        if (BPGvalue >0):
            edgesInBPG +=1
            auth_value = author_dict.keys()[author_dict.values().index(auth_counter+1)]
            BPGdata += ('\nA{}\tC{}\tedge\t{}'.format((auth_counter+1), (year_counter + 2001), BPGvalue/10))
BPGdata = str(edgesInBPG) + BPGdata
f_auth_conf_graph.write(BPGdata)
del BPGdata
del BPGraph_dict
f_auth_conf_graph.close()
wb.close()
