import numpy as np
import matplotlib.pyplot as plt

GRAVITY = 9.81

def parabolic_arc(initial_position, initial_velocity, t):
    x = initial_position[0] + initial_velocity[0] * t
    y = initial_position[1] + initial_velocity[1] * t - 0.5 * GRAVITY * (t ** 2)
    return x, y

def least_squares(a, b):
    a_transpose = a.transpose()
    return np.linalg.solve(np.matmul(a_transpose, a), np.matmul(a_transpose, b))

def fit(x, y, t, evaluate_t):
    x = x.reshape((-1, 1))
    y = y.reshape((-1, 1))

    X = np.array([np.ones_like(t), t]).transpose()
    Y = np.array([np.ones_like(t), t, (t ** 2)]).transpose()

    # X_parameters = np.linalg.lstsq(X, x)[0]
    # Y_parameters = np.linalg.lstsq(Y, y)[0]
    X_parameters = least_squares(X, x)
    Y_parameters = least_squares(Y, y)

    # print(X_parameters)
    # print(Y_parameters)

    return parabolic_arc(
        [X_parameters[0], Y_parameters[0]],
        [X_parameters[1], Y_parameters[1]],
        evaluate_t)

def ransac_fit(x, y, t):
    best_fit = None
    best_inliers = 0

    for _ in range(10):
        sampled_x = []
        sampled_y = []
        sampled_t = []

        for i in range(5):
            index = np.random.randint(0, len(x) - 1)
            sampled_x.append(x[index])
            sampled_y.append(y[index])
            sampled_t.append(t[index])

        model_x, model_y = fit(np.array(sampled_x), np.array(sampled_y), np.array(sampled_t), np.array(t))

        inliers = 0
        for i in range(len(x)):
            if np.linalg.norm(np.array([x[i], y[i]]) - np.array([model_x[i], model_y[i]])) <= 0.1:
                inliers += 1

        if inliers > best_inliers:
            best_fit = (model_x, model_y)
            best_inliers = inliers

    print("RANSAC num inliers: {}".format(best_inliers))
    if best_inliers < 10:
        return None

    return best_fit

if __name__ == "__main__":
    np.random.seed(1337)

    t = np.arange(0.0, 1.3, step=0.05)
    x, y = parabolic_arc([0.0, 0.0], [10.0, 6.0], t)

    std_dev = 0.025 * 3
    x += np.random.normal(scale=std_dev, size=x.shape)
    y += np.random.normal(scale=std_dev, size=y.shape)
    # x[3] += 10

    fit_x, fit_y = fit(x, y, t, t)
    ransac_model = ransac_fit(x, y, t)

    plt.plot(x, y, label="data")
    plt.plot(fit_x, fit_y, label="least squares")

    if ransac_model is not None:
        plt.plot(ransac_model[0], ransac_model[1], label="RANSAC")

    plt.legend()

    plt.show()