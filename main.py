import matplotlib.pyplot as plt
import numpy as np

file = open ("plot.txt")

xArray = []
yArray = []

for i in file:
    x, y = i.split()
    xArray.append(x)
    yArray.append(y)

print (xArray, yArray)
xaxis = np.array(xArray)

# Y axis parameter:
yaxis = np.array(yArray)

plt.axis ([xArray[0], xArray[-1], yArray[0], yArray[-1]])
plt.plot(xaxis, yaxis)
plt.show()
