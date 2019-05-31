
import json
from rdflib import Graph
import pprint



g = Graph()
g.parse(".../I40KG-Embeddings-master/sto/sto.nt", format="nt")
    
len(g) # prints 2
 #check printing of the graph    
'''for stmt in g:
    pprint.pprint(stmt)'''

#query to get the framework/standard from the sto.nt file
#to get standards we have to replace sto:StandardizationFramework by sto:Standard in the query
#we can get standards of the frameworks as well by just changing the query

qres = g.query(
    """PREFIX owl: <http://www.w3.org/2002/07/owl#>
       PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
       PREFIX sto: <https://w3id.org/i40/sto#>
    
    select ?s where {
            ?s rdf:type sto:StandardizationFramework .
            } limit 1000""")
    
       
#to get the corresponding embeddings of the frameworks/standards from the json file 
with open(".../I40KG-Embeddings-master/logs_sto/entities_to_embeddings.json",'rb') as f:
    array = json.load(f)
new_dict = {}
for row in qres:
    for key,value in array.items():
        if key == "%s" % row:
            tem = key
            new_dict[tem] = array[key] 
            print(new_dict)

#to put the frameworks/standards with their corresponding embeddings in a file            
with open('output.json','w') as f:
    # this would place the entire output on one line
    # use json.dump(lista_items, f, indent=4) to "pretty-print" with four spaces per indent
    json.dump(new_dict, f)     
              
        
        
        
        
        
        
        
   
