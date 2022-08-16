import numpy as np
import matplotlib.pyplot as plt

x = np.array([8, 16, 32])

y = np.array([14, 42, 34])
plt.plot(x, y, 'o')
plt.xlabel("mazeEndIter")
plt.ylabel("drvs")
plt.axis([7, 40, 0, 44])

m, b = np.polyfit(x, y, 1)


plt.plot(x, m*x + b)
plt.show()