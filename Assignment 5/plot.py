import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

fil = open('input.txt', 'r')
ylist = fil.readlines()
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

plt.plot(itr_list,page_list,marker='o')
plt.title("Page reference vs iterations")
plt.xlabel("Iteration")
plt.ylabel("Page reference")
plt.show()