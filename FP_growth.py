import sys
class TreeNode():
    def __init__(self,name):
        self.name = name
        self.frq = 0
        self.child = {}
        self.parent = None
        self.link = None
def load_data(input_file):
    dataSet = []
    num_tx = 0
    with open(input_file,'r') as inp :
        transaction = inp.readline().strip()
        while(transaction):
            num_tx += 1
            items = transaction.split(',')
            dataSet.append(items)
            transaction = inp.readline().strip()
    return dataSet,num_tx

def construct_FP_tree(dataSet,min_sup):
    headertable = {}
    for trans in dataSet: # iterate each transaction in dataSet
        for item in trans: # iterate each item in transaction
            #assign headertable[item]= 1 if item not in headertable , otherwise assign headertable[item]++ 
            headertable[item] = headertable.get(item, 0) + 1
    
    error = 0.0001
    key_set = set(headertable.keys()) # key_set include all names of items
    for key in key_set:  #delete item if its total frequency smaller than min_support
        if headertable[key] + error < min_sup:
            del headertable[key]
    if len(headertable.keys()) == 0: #headertable and FP_tree are None if no items are greater than or equal to min_support 
        return None, None
    headertable = dict(sorted(headertable.items(),key = lambda item:int(item[0])))
    headertable = dict(sorted(headertable.items(),key = lambda item:item[1],reverse=True))
    
    frq_DB = []
    for trans in dataSet:
        tmp = []
        for item in headertable:
            if(item in trans):
                tmp.append(item)
        frq_DB.append(tmp)
    for key in headertable:
        # translation  datastructre of headertable into   {name : [count, pointer]
        headertable[key] = [headertable[key], None]
    
    root = TreeNode('Null')
    root.frq = 1
    for transaction in frq_DB:
        tmp_root = root
        for item in transaction:
            if(item in tmp_root.child):
                tmp_root.child[item].frq += 1
                tmp_root = tmp_root.child[item]
            else:
                new_node = TreeNode(item)
                new_node.frq = 1
                tmp_root.child[item] = new_node
                new_node.parent = tmp_root
                if(headertable[item][1] is None):
                    headertable[item][1] = new_node
                else:
                    node = headertable[item][1]
                    while(node.link):
                        node = node.link
                    node.link = new_node
                tmp_root = new_node
    return root,headertable

def findPrefixPath(treeNode):
    #given a item as suffix , find all prefix path 
    condaPats = [] 
    while treeNode != None:
        prefixPath = []  #prefix path
        node = treeNode
        while node.parent != None:
            prefixPath.append(node.name)
            node = node.parent
        if len(prefixPath) >= 1:
            for i in range(treeNode.frq):
                condaPats.append(prefixPath[1:]) #for same prefix path ,add treeNode.frq it to variable condaPats
        treeNode = treeNode.link
    return condaPats

def minTree(root, headertable, min_sup, preFix, freqPattern):
    #sort headertable by total frequency,and put each name into suffix_L
    suffix_L = [ v[0] for v in sorted(headertable.items(), key = lambda p: p[1][0])]
    
    for suffix in suffix_L:  #from item which has smallest frequency  to  largest frequenct 
        newFreqSet = preFix.copy()
        newFreqSet.append(suffix)
        freqPattern.append(newFreqSet)
        
        condPattBases = findPrefixPath(headertable[suffix][1])
        condTree, condHead  = construct_FP_tree(condPattBases,min_sup)
        if condHead != None:
            minTree(condTree, condHead, min_sup, newFreqSet, freqPattern)

def process_FQP(freqPattern):
    for idx in range(len(freqPattern)):
        items = freqPattern[idx]
        items = list(map(int,items))
        items = sorted(items)
        freqPattern[idx] = items
    freqPattern = sorted(freqPattern,key=lambda x:len(x)) #sort elements of freqPattern by length
    #print(freqPattern)
    len_idx = {}
    for idx in range(len(freqPattern)):
        items = freqPattern[idx]
        len_idx[len(items)] = idx
    
    idx_s = 0
    #sort patterns that have same length by index
    for idx_e in len_idx.values():
        while(idx_s < idx_e):
            for i in range(idx_e-1,idx_s-1,-1):
                if(freqPattern[i+1] < freqPattern[i]):
                    freqPattern[i],freqPattern[i+1] = freqPattern[i+1],freqPattern[i]
            idx_s += 1
        idx_s += 1
    return freqPattern

def calSuppData(headertable, freqPattern):
    suppData = []
    N = len(freqPattern)
    order = list(headertable.keys())[::-1]
    for idx in range(N):
        tmp = set(freqPattern[idx])
        items = []
        for item in order:
            if( int(item) in tmp):
                items.append(item)
        #sort items by frequency
        base = findPrefixPath(headertable[items[0]][1])
        #base is one of items which have smallest frequency
        # compute sup
        support = 0
        for B in base:#B is one of paths
            # items[1:] => path which function findPrefixPath() return is exclude start point
            if set(items[1:]).issubset(set(B)):
                support += 1
        # base is None and items only have one element => root's child 
        if len(base)==0 and len(items)==1:
            support = headertable[items[0]][0]
        suppData.append(support)
    return suppData

def output(output_file,freqPattern,suppData):
    N = len(freqPattern)
    with open(output_file,'w') as output:
        for i in range(N):
            items = [str(j) for j in freqPattern[i]]
            #output.write(','.join(freqPattern[i]) )
            output.write("{}:{:.4f}\n".format(','.join(items),suppData[i]) )
            
def levelorder(root):
    LastLayer = []
    CurrLayer = [root]
    NextLayer = []
    seq = []
    while(CurrLayer):
        level =[]
        for node in CurrLayer:
            for child in node.child:
                NextLayer.append(node.child[child]) 
            level.append(node.name)
        seq.append(level)
        LastLayer = CurrLayer
        CurrLayer = NextLayer
        NextLayer = []
    for i in seq:
        print(i)


if __name__ == "__main__" :
    min_support = float(sys.argv[1])
    input_file = sys.argv[2]
    output_file = sys.argv[3]
    dataSet,num_tx = load_data(input_file)
    FP_tree,headertable= construct_FP_tree(dataSet,min_support*num_tx)
    freqPattern = []
    minTree(FP_tree, headertable, min_support*num_tx, [], freqPattern)
    freqPattern = process_FQP(freqPattern)
    suppData = calSuppData(headertable, freqPattern)
    suppData = [i/float(num_tx) for i in suppData]
    output(output_file , freqPattern , suppData)




