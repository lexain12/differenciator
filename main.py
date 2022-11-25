import matplotlib.pyplot as plt
import numpy as np

file = open ("plot.txt")

xArray = []
yArray = []

for i in file:
    x, y = i.split()
    xArray.append(float(x))
    yArray.append(float(y))

print (xArray, yArray)
xaxis = np.array(xArray)

# Y axis parameter:
yaxis = np.array(yArray)

plt.plot(xaxis, yaxis)
plt.savefig ("graph.png")

