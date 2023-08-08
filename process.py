import json

with open("headers/index.json", "r") as f:
    contents = json.load(f)

del contents[-1] 
with open("test.json", "w") as f:
    json.dump(contents, f)