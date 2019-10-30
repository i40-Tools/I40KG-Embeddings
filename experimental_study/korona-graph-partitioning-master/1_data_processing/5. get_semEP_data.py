import sys, os

CONF = "ISWC"

authorfile = os.getcwd() + "/output/Author.txt"
authormap = os.getcwd() + "/output/author-key map.txt"
conferencefile = os.getcwd() + "/output/Conf.txt"
graphfile = os.getcwd() +  "/output/Auth-Conf_graph.txt"

authorfile_descrp = os.getcwd() + "/output/author-descrp.txt"
conference_descrp = os.getcwd() + "/output/conf-descrp.txt"
graph_descrp = os.getcwd() + "/output/Auth-Conf_graph-descrp.txt"

def get_author_name(author):
    tok = author.split("/")
    name = tok[-1][:-1].split(":")
    name.reverse()
    sname = ""
    for s in name[:-1]:
        sname += s + " "
    sname += name[-1]
    return sname

def load_author_map():
    author_l = {}
    with open(authormap) as f:
        for line in f:
            line = line.rstrip()
            tok = line.split(" ")
            name = get_author_name(tok[0])
            idf = "A"+tok[1]
            author_l[idf] = name
    return author_l

def load_entities(filename):
    entities = []
    with open(filename) as fd:
        fd.readline()
        for line in fd:
            line = line.rstrip()
            entities.append(line)
    return entities

def load_graph():
    edges = []
    with open(graphfile) as fd:
        fd.readline()
        for line in fd:
            tok = line.rstrip().split("\t")
            edges.append((tok[0], tok[1], tok[2], tok[3]))
    return edges

def write_authors(authors, authors_map):
    with open(authorfile_descrp, "w") as fd:
        fd.write(str(len(authors))+"\n")
        for author in authors:
            fd.write(authors_map[author]+"\n")

def write_conferences(conferences):
    with open(conference_descrp, "w") as fd:
        fd.write(str(len(conferences))+"\n")
        for conf in conferences:
            fd.write(CONF+"_"+conf[1:]+"\n")

def write_graph(edges, authors_map):
    with open(graph_descrp, "w") as fd:
        fd.write(str(len(edges))+"\n")
        for (author, conf, edge, weight) in edges:
            fd.write(authors_map[author]+"\t"+CONF+"_"+conf[1:]+"\t"+edge+"\t"+weight+"\n")

def main(*args):
    author_list = load_entities(authorfile)
    author_map = load_author_map()
    conference_list = load_entities(conferencefile)
    edges_list = load_graph()
    
    write_authors(author_list, author_map)
    write_conferences(conference_list)
    write_graph(edges_list, author_map)
    
if __name__ == '__main__':
    main(*sys.argv[1:])
