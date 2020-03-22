
import scipy
from scipy import spatial
import numpy as np
import math
import json
from scipy.spatial.distance import cdist

#function to calculate cosine distance 
def cosine_similarity(vec1,vec2):
    sum11, sum12, sum22 = 0, 0, 0
    for i in range(len(vec1)):
        x = vec1[i]; y = vec2[i]
        sum11 += x*x
        sum22 += y*y
        sum12 += x*y
    return sum12/math.sqrt(sum11*sum22)

#loading the json file containing frameworks/standards along with their embeddings
with open('output.json', 'r') as f:
    array = json.load(f)
#print (array)

#pass the frameworks/standards with their embeddings 
result = {}
for key,value in array.items():
    temp,tempDict= 0,{}
    for keyC,valueC in array.items():
        if keyC == key:
            continue
        temp = scipy.spatial.distance.cosine(value,valueC)
        tempDict[keyC] = temp
        val1 = min(tempDict, key=tempDict.get)
    res = {}
    res[val1] = tempDict[val1]
    #print (res)
    result[key]= res
    #result[key]= tempDict
        

print(result)


