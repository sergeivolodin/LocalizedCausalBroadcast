import numpy as np
from tqdm import tqdm
import pickle
from get_time import get_time

# repetitions
repetitions = range(5)

# n processes
n = 5

# messages
ms = np.arange(1, 20000, 3000)

# results: map m messages -> array over repetitions
results = {m: [] for m in ms}

# doing the computations
for m in ms:
    for _ in tqdm(repetitions):
        results[m].append(get_time(m, n))

# saving data
open('results_m_lcb.pkl', 'wb').write(pickle.dumps(results))
