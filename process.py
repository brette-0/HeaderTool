import os, json

contents = list()
for path in os.listdir("./headers"): contents.append(path) 
with open("headers/index.json", "w") as f:
    json.dump(contents, f)
    print(*[f"{c}\n" for c in contents])