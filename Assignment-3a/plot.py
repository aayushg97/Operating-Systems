import matplotlib.pyplot as plt

plt.plot([10, 50, 100], [40.21, 142.124, 272.763],marker='o',color = "red",label = "FCFS")
plt.plot([10, 50, 100], [30.67, 91.372, 169.421],marker='o',color = "green",label = "SJF")
plt.plot([10, 50, 100], [53.93, 199.158, 389.102],marker='o',color = "yellow",label = "RR1")
plt.plot([10, 50, 100], [54.45, 200.906, 392.197],marker='o',color = "lightblue",label = "RR2")
plt.plot([10, 50, 100], [56.33, 205.914, 404.752],marker='o',color = "darkblue",label = "RR5")

plt.title("Average Turnaround Time")
plt.xlabel("N")
plt.ylabel("ATN")

ax = plt.subplot(111)
box = ax.get_position()
ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])

ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))

ax.set_xlim(0, 110)

plt.show()
