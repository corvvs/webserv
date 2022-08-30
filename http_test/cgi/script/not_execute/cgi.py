print("Content-type: text/html")
print()
with open("../sample.html") as f:
    print(f.read(),end="")
