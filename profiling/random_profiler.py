import matplotlib.pyplot as plt
import numpy as np

# genera GLSL vec3 random generator function
# vec3 random3(vec3 p) {
#     p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
#              dot(p, vec3(269.5, 183.3, 246.1)),
#              dot(p, vec3(113.5, 271.9, 124.6)));

#     return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
# }


def vec3(a, b, c):
    return np.array([a, b, c], dtype=np.float32)


def vec3_1(a):
    return np.array([a] * 3, dtype=np.float32)


def floor(a):
    return np.floor(a)


def fract(a: np.array):
    return a - floor(a)


def random3(p: np.array):
    _p = vec3(np.dot(p, vec3(127.1, 311.7, 74.7)), np.dot(
        p, vec3(269.5, 183.3, 246.1)), np.dot(p, vec3(113.5, 271.9, 124.6)))
    return vec3_1(-1.0) + 2.0 * fract(np.sin(_p) * 43758.5453123)


p = vec3(0.7, 1.2, 2.3)

p_collection = [p[0]]

for i in range(100000):
    nextp = random3(p)
    p_collection.append(nextp[0])
    p = nextp
    print(f'\r{i}', end='')

plt.hist(p_collection, bins=800, color='r', alpha=0.4, edgecolor='y')
plt.show()
