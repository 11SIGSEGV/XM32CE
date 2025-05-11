with open("./Source/frequencies.txt", "r") as f:
    totals = dict()
    temp_floatmap = []
    for ln in f.readlines():
        ln = ln.removesuffix("\n")

        if len(ln) > 50:
            # Means this line is a float map
            # For each 6 characters, add a float to the list
            temp_floatmap = []
            while ln:
                temp_floatmap.append(float(ln[:6]))
                ln = ln[6:]
        else:
            if "k" in ln:
                thousand, remaining = ln.split("k")
                thousand = int(thousand) * 1000
                remaining = int(remaining) * 10
                total = float(thousand + remaining)
            else:
                total = float(ln)
            totals[temp_floatmap[0]] = total
            temp_floatmap.pop(0)


with open("./Source/frequencies.out", "w") as f:
    # Write the frequencies to 200 characters per line
    finalString = ""
    lineString = ""
    for val, freq in totals.items():
        lineString += f"{{{val}, {freq}}}, "
        if len(lineString) > 200:
            finalString += lineString + "\n"
            lineString = ""
    if lineString != "":
        finalString += lineString + "\n"
    f.write(finalString)


