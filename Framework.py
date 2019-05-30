import json
from rdflib import Graph
import pprint


g = Graph()
g.parse(".../sto/sto.nt", format="nt")
    
len(g) # prints 2
#check printing of graph   
'''for stmt in g:                         
    pprint.pprint(stmt)'''

#Query to find Framework/Standard

qres = g.query(
    """PREFIX owl: <http://www.w3.org/2002/07/owl#>
       PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
       PREFIX sto: <https://w3id.org/i40/sto#>
    
    select ?s where {
            ?s rdf:type sto:StandardizationFramework .       #to find standard we just have to replace the query with sto:Standard instead of sto:StandardizationFramework
            } limit 1000""")
    
#Find the embeddings from logs_sto folder for the corresponding framework/standard by just matching the framework/standard that we received from the query results
        
with open(".../logs_sto/entities_to_embeddings.json",'rb') as f:
    array = json.load(f)
new_dict = {}
for row in qres:
    for key,value in array.items():
        if key == "%s" % row:
            tem = key
            new_dict[tem] = array[key] 
            print(new_dict)

#write the frameworks/standards with their embeddings in a separate file
with open('output.json','w') as f:
    # this would place the entire output on one line
    json.dump(new_dict, f)     
