# add lib linking statement AddSystemHeader to all test.go, run python3 liblinking.py
import os
import glob
import re

def process_file(file_path):
    with open(file_path, 'r') as f:
        lines = f.readlines()
    content = ''.join(lines)
    # if this library is not imported, this rule is not implemented
    if "naive.systems/analyzer/cruleslib/testlib" not in content:
        return
    found_expect_ok = False
    variable_name = None
    for i, line in enumerate(lines):
        if re.search(r"opts\s*,.*:=", line):
            variable_name = "opts"
        elif re.search(r"options\s*,.*:=", line):
            variable_name = "options"
        elif "tc.ExpectOK(" in line:
            found_expect_ok = True
            if "testlib.AddSystemHeader(" not in lines[i - 1] and variable_name != None:
                lines.insert(i, f"	testlib.AddSystemHeader({variable_name})\n")
                print(f"AddSystemHeader added in {file_path} at line {i}")
            elif variable_name is None:
                print(f"Failure: opts or options not declared in the function of {file_path} at line {i}")
        elif re.match("func\s", line):
            variable_name = None

    if not found_expect_ok:
        print("Error: expectOK not found in the code")

    content = ''.join(lines)
    with open(file_path, 'w') as f:
        f.write(content)

def main():
    for root, dirs, files in os.walk("../"):
        for file in glob.glob(os.path.join(root, "*test.go")):
            process_file(file)
if __name__ == "__main__":
    main()
