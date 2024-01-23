# Will generate the file data.qrc to be used when embedding data into the
# executable for releases. This will only be run if -DEMBED_DATA=True
# is passed as a flag when building with CMake.
import os

def main():
    directories = ['../logic/data', '../asm/patch_diffs', '../assets', '../customizer']
    individual_files = ['../asm/custom_symbols.yaml']
    with open('data.qrc', 'w') as qrc:
        qrc.write('<RCC>\n')
        qrc.write('    <qresource prefix="/">\n')
        for directory in directories:
            for root, dirs, files in os.walk(directory):
                for file in files:
                    f = os.path.join(root, file)
                    if os.path.isfile(f):
                        f = f.replace('\\', '/')
                        individual_files.append(f)

        for file in individual_files:
            embed_path = file.replace('../', '')
            qrc.write('        <file alias=\"' + embed_path + '\">' + file + '</file>\n')

        qrc.write('    </qresource>\n')
        qrc.write('</RCC>')

main()
