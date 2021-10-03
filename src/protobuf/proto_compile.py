import os

OS_NAME = ""
INPUT_DIR = "./proto/"
TMP_DIR = "./.tmp_working_dir/"
CPP_OUTPUT_FILE = "../navigation/00_Protobuf.ino"
PYTHON_OUTPUT_DIR = "./python/"
PROTOC = "./nanopb/generator/protoc"

ADDED_FILES = []

def clearDest():
    open(CPP_OUTPUT_FILE, 'w').close()

def removeIncludes(lines):
    result = []
    for line in lines:
        not_include = True

        if "#include " in line:
            already_in = False
            for f_name in ADDED_FILES:
                if f_name in line:
                    not_include = False
                    break

        if not_include:
            result.append(line)

    return result

def copyInto(src, dest):
    input_f = open(src, 'r')
    lines_to_add = removeIncludes(input_f.readlines())
    input_f.close()

    of = open(dest, 'a')
    for line in lines_to_add:
        of.write(line)
    of.close()

    ADDED_FILES.append(src.split("/")[-1])

def addNanopbToFile():

    lines_to_add = []

    dir = "./nanopb/"

    h_files = [
        "pb.h",
        "pb_common.h",
        "pb_encode.h",
        "pb_decode.h"
    ]

    c_files = [
        "pb_common.c",
        "pb_encode.c",
        "pb_decode.c"
    ]

    for h in h_files:
        copyInto(dir+h, CPP_OUTPUT_FILE)

    for c in c_files:
        copyInto(dir+c, CPP_OUTPUT_FILE)

def addProtoToFile():
    h_files = []
    c_files = []

    for f_name in os.listdir(TMP_DIR):
        if ".h" in f_name:
            h_files.append(f_name)
        elif ".c" in f_name:
            c_files.append(f_name)

    for h in h_files:
        copyInto(TMP_DIR+h, CPP_OUTPUT_FILE)

    for c in c_files:
        copyInto(TMP_DIR+c, CPP_OUTPUT_FILE)

    

if __name__ == "__main__":

    if not "Linux" in os.uname():
        print("For now this script does not work in Windows or Mac")
        exit(-1)
    else:
        OS_NAME = "Linux"

    if OS_NAME == "Linux":

        # Create the tmp directory
        os.system("mkdir " + TMP_DIR)

        # Compile protobuf files
        proto_files = os.listdir(INPUT_DIR)
        for f in proto_files:
            if ".proto" in f:
                cmd = PROTOC + " --nanopb_out=" + TMP_DIR + " --python_out=" + PYTHON_OUTPUT_DIR + " --proto_path=" + INPUT_DIR + " " + f
                print(cmd)
                os.system(cmd)

        # Clean output file
        clearDest()

        # Add base nanopb library in the output file
        addNanopbToFile()

        # Add generated protobuf file to the output file
        addProtoToFile()

        # Remove tmp directory
        os.system("rm -rf " + TMP_DIR)

    
