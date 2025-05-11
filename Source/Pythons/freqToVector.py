with open("./Source/frequencies.txt", "r") as f:
    totals = []
    for ln in f.readlines():
        ln = ln.removesuffix("\n")
        if "k" in ln:
            thousand, remaining = ln.split("k")
            thousand = int(thousand) * 1000
            remaining = int(remaining) * 10
            total = float(thousand + remaining)
        else:
            total = float(ln)
        if total in totals:
            continue
        totals.append(total)

with open("./Source/frequencies.out", "w") as f:
    # Write the frequencies to 200 characters per line
    finalString = ""
    lineString = ""
    for total in totals:
        lineString += str(total) + ", "
        if len(lineString) >= 200:
            finalString += lineString + "\n"
            lineString = ""
    if lineString != "":
        finalString += lineString + "\n"
    f.write(finalString)


