# Will generate the file data.qrc to be used when embedding data into the
# executable for releases. This will only be run if -DEMBED_DATA=True
# is passed as a flag when building with CMake.
import os
import sys
import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("data_dir")
    args = parser.parse_args(sys.argv[1:])

    data_root = args.data_dir
    individual_files = []
    with open('data.qrc', 'w') as qrc:
        qrc.write('<RCC>\n')
        qrc.write('    <qresource prefix="/">\n')
        for root, dirs, files in os.walk(data_root):
            for file in files:
                f = os.path.join(root, file)
                if os.path.isfile(f):
                    f = f.replace('\\', '/')
                    individual_files.append(f)

        for file in individual_files:
            embed_path = file.replace(data_root, '')
            qrc.write('        <file alias=\"' + embed_path + '\">' + file + '</file>\n')

        qrc.write('    </qresource>\n')
        qrc.write('</RCC>')

main()
