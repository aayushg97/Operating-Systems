import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from math import log 

with open('input.txt', 'r') as fil:
	ylist = fil.readlines()
	ylist = ylist[:min(len(ylist),1000)]
page_list = []
itr_list = []
i = 0
while(i < len(ylist)):
	if(ylist[i].find('#')!=-1):
		ylist.pop(i);
	else:
		i = i + 1;

for i in range(0, len(ylist)):
	ylist[i] = (ylist[i]).split(' ')
	page_list.append(ylist[i][1])
	itr_list.append(i+1)

plt.scatter(itr_list,page_list,marker='o',s=0.5)
plt.title("Locality of Reference")
plt.xlabel("Instruction no")
plt.ylabel("Page no referenced")
plt.savefig('loc_ref.png')
