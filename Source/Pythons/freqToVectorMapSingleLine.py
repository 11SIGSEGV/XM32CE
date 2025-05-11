with open("./Source/frequencies.txt", "r") as f:
    totals = dict()
    temp_floatmap = []
    previousWasFloat = False
    for ln in f.readlines():
        ln = ln.removesuffix("\n")

        if len(ln) > 3:
            if not previousWasFloat:
                # Means this line is the first line of float map
                temp_floatmap = []
                previousWasFloat = True
            # Means this line is a float map
            # For each 6 characters, add a float to the list
            temp_floatmap.append(float(ln))
        else:
            previousWasFloat = False
            total = float(ln)
            # if total not in totals.values():
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


