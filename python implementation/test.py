import pandas as pd
# df = pd.read_parquet('../data/59-126901327188416.parquet')
# print(df)

# df = pd.read_json('../data/rocket.json')
# print(df)
import resource

import psutil
import os

# inner psutil function
def process_memory():
    process = psutil.Process(os.getpid())
    mem_info = process.memory_info()
    return mem_info.rss


import pyarrow.parquet as pq

t1 = pq.read_table('../data/58-128114831827392.parquet')
t2 = pq.read_table('../data/57-139498284335552.parquet')

assert t1.equals(t2)