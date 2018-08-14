import matplotlib.pyplot as plt
import numpy as np

def fit_circle(x, y):
    x = np.array(x)
    y = np.array(y)
    X = np.square(x) + np.square(y)


if __name__ == "__main__":
    x = []
    y = []

    radius = 10
    for angle in np.arange(0.0, 2.0 * np.pi * 0 + 4.0, step=0.1):
        x.append(radius * np.cos(angle))
        y.append(radius * np.sin(angle))

    x.append(x[-1] + 1)
    y.append(y[-1] + 2)

    x.append(x[-1] + 3)
    y.append(y[-1] + 2)

    x.append(x[-1] + 3)
    y.append(y[-1] - 2)

    for angle in np.arange(5.0, 2.0 * np.pi, step=0.1):
        x.append(radius * np.cos(angle))
        y.append(radius * np.sin(angle))

    plt.plot(x, y)
    plt.show()
    plt.axis("equal")