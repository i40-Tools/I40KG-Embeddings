# -*- coding: utf-8 -*-
"""
Created on Tue May 21 20:53:50 2019

@author: Kaushikee
"""
import json
from rdflib import Graph
import pprint



g = Graph()
g.parse("C:/Users/Kaushikee/.spyder-py3/I40KG-Embeddings-master/sto/sto.nt", format="nt")
    
len(g) # prints 2
    
'''for stmt in g:
    pprint.pprint(stmt)'''
    
qres = g.query(
    """PREFIX owl: <http://www.w3.org/2002/07/owl#>
       PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
       PREFIX sto: <https://w3id.org/i40/sto#>
    
    select ?s where {
            ?s rdf:type sto:StandardizationFramework .
            } limit 1000""")
    
'''    
with open("framework_entity.nt", "w") as fd:
    for row in qres:
        fd.write("%s" % row + "\n")
        #print("%s" % row)'''
        
#oldfile = open("framework_entity.nt", "r")
with open("C:/Users/Kaushikee/.spyder-py3/I40KG-Embeddings-master/logs_sto/entities_to_embeddings.json",'rb') as f:
    array = json.load(f)
new_dict = {}
for row in qres:
#for line in oldfile:
    #line1 = line.strip("\n")
    for key,value in array.items():
        if key == "%s" % row:
            tem = key
            new_dict[tem] = array[key] 
            print(new_dict)

with open('output.json','w') as f:
    # this would place the entire output on one line
    # use json.dump(lista_items, f, indent=4) to "pretty-print" with four spaces per indent
    json.dump(new_dict, f)     
              
        
        
        
        
        
        
        
   