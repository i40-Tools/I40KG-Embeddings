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
import os,glob
import os.path as path

reload(sys)
sys.setdefaultencoding('UTF-8')

inputpath = path.abspath(path.join(__file__ ,"../")) + "/nt-files/" #we have to iterate through the entire DBLP database
authorfile= os.getcwd() + "/output/author-list.txt"
author_Matrix_File = os.getcwd() + "/output/Auth_matrix.txt"
x = glob.glob("%s*.nt" %inputpath)

f_author_file = open(authorfile, 'r')
author_dict = {}
author_list = []

author_count = 0
for row in f_author_file:
    author_dict[row.rstrip("\n")] = author_count
    author_list.append([row.rstrip("\n"),0])
    author_count+=1

f_author_file.close()

# Here we calculate the similarity of two authors which builds up the similarity matrix of authors.
def SimilarityOfAuthors(auth1,auth2):
    if (auth1 == auth2):
        return float(1.0)
    #Things will change from here
    global author_list
    global author_dict

    counterBoth = 0
    counterEither = 0

    visited_Conf = []

    key1 = author_dict.get(auth1)
    key2 = author_dict.get(auth2)

    No_entries1 = author_list[key1][1]
    No_entries2 = author_list[key2][1]
    
    for i in range(No_entries1):
        Conf_x = author_list[key1][i+2][0]
        visited_Conf.append(Conf_x)
        Counter_x = author_list[key1][i+2][1]
        Conf_x_present = 0
        for j in range(No_entries2):
            Conf_y = author_list[key2][j+2][0]
            Counter_y = author_list[key2][j+2][1]
            if Conf_x == Conf_y:
                Conf_x_present = 1
                if Counter_y> Counter_x:
                    counterBoth = counterBoth + Counter_x
                    counterEither = counterEither + Counter_x
                else:
                    counterBoth = counterBoth + Counter_y
                    counterEither = counterEither + Counter_y
        if Conf_x_present == 0:
            counterEither = counterEither + Counter_x
    for i in range(No_entries2):
        Conf_y = author_list[key2][i+2][0]
        if Conf_y not in visited_Conf:
            counterEither = counterEither + author_list[key2][i+2][1]
    if counterEither == 0:
        return 0
    similarity = float(counterBoth)/counterEither
    return similarity

#################******************~~~~~~~~~~~~Processing starts here~~~~~~~~~~~~*******************#######################

for current_file in x:
    nt_file_pointer = open(current_file, 'r')
    for row in nt_file_pointer:
        if row == "\n":
            continue
        eachnumber = re.split(r' ', row)
        if "#authoredBy" not in eachnumber[1]:
            continue
        auth = eachnumber[2].strip("<").strip(">")
        if auth not in author_dict:
            continue
        conf = eachnumber[0].strip("<").strip(">").rsplit('/',1)[0]
        auth_pointer = author_dict.get(auth)
        if author_list[auth_pointer][1] == 0:
            author_list[auth_pointer][1] = 1
            author_list[auth_pointer].append([conf,1])
        else:
            counter = author_list[auth_pointer][1]
            list_pointer = 0
            for i in range(counter):
                if conf in author_list[auth_pointer][i+2]:
                    list_pointer = i+2
                    break
            if list_pointer ==0:
                author_list[auth_pointer][1] = counter + 1
                author_list[auth_pointer].append([conf,1])
                list_pointer = counter + 2
            conf_value = author_list[auth_pointer][list_pointer][1]
            author_list[auth_pointer][list_pointer] = [conf,conf_value+1]
    nt_file_pointer.close()

#Here we save the Author Similarity Matrix File.
f_auth_matrix = open(author_Matrix_File, 'w+')

BPGraph_AuthorSimilarity=[[0 for x in range(author_count)] for y in range(author_count)]

for key1, value1 in sorted(author_dict.iteritems(), key=lambda (k,v): (v,k)):
    for key2, value2 in sorted(author_dict.iteritems(), key=lambda (k,v): (v,k)):
        if (value2 >= value1):
            similarityValue = SimilarityOfAuthors(key1,key2)
            BPGraph_AuthorSimilarity[value1][value2] =similarityValue
            BPGraph_AuthorSimilarity[value2][value1] = BPGraph_AuthorSimilarity[value1][value2]
            #print similarityValue

BPGraph_string = ""
for i in range(author_count):
    for j in range(author_count):
        BPGraph_string += str(BPGraph_AuthorSimilarity[i][j]) + " "
    BPGraph_string = BPGraph_string[:-1]
    BPGraph_string += "\n"
BPGraph_string = str(author_count) + "\n" + BPGraph_string
f_auth_matrix.write(BPGraph_string)
del BPGraph_AuthorSimilarity
del BPGraph_string
f_auth_matrix.close()
