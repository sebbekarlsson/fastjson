import glob
import subprocess
import os
import datetime
import json

files = glob.glob('./sources/*.json')
DIR = os.path.dirname(os.path.abspath(__file__))
BIN_PATH = os.path.join(DIR, "../build/fjson_e")

def parse(filepath):
    com = subprocess.Popen([BIN_PATH, filepath])
    return com.communicate()




entries = []

for filepath in files:
    start = datetime.datetime.now()
    parse(filepath)
    end = datetime.datetime.now()

    diff = end - start

    entries.append(dict(filepath=filepath, time=diff))



open('bench.json', 'w+').write(json.dumps(entries, sort_keys=True, indent=2, default=str))
print('bench.json written.')
