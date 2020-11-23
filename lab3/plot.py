import subprocess
import numpy as np
import matplotlib.pyplot as plt

correct = np.loadtxt("trace.txt", usecols=1)

plots = np.arange(start=10, stop=21, step=1)
results = np.zeros((11,))

for i, m in enumerate(plots):
    r = subprocess.run(["/Users/line/Desktop/nyu/Computer architecture ECE-GY 6913/lab3/cmake-build-debug/code", str(m)])
    if r.returncode != 0:
        print("m = %d, return code = %d" % (m, r.returncode))
        break
    predict = np.loadtxt("trace.txt.out")
    result = correct == predict
    precision = np.count_nonzero(result) / len(result)
    print(np.count_nonzero(result), len(result) - np.count_nonzero(result), len(result))
    results[i] = (1 - precision) * 100
    print("m = %d, Mis-Prediction: %.4f%%" % (m, (1 - precision) * 100))

plt.plot(plots, results)
plt.xlabel("m")
plt.ylabel("Mis-prediction rate (%)")
plt.xticks(plots)
plt.show()