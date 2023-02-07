# Will generate the file data.qrc to be used when embedding data into the
# static executable for releases. This will only be run if -DEMBED_DATA=True
# is passed as a flag when building with Cmake.
import os

def main():
    directories = ['../logic/data', '../asm/patch_diffs', '../assets', '../assets/tracker']
    individual_files = ['../asm/custom_symbols.json']
    with open('data.qrc', 'w') as qrc:
        qrc.write('<RCC>\n')
        qrc.write('    <qresource prefix="/">\n')
        for directory in directories:
            for file in os.listdir(directory):
                f = os.path.join(directory, file)
                if os.path.isfile(f):
                    f = f.replace('\\', '/')
                    individual_files.append(f)

        for file in individual_files:
            embed_path = file.replace('../', '')
            qrc.write('        <file alias=\"' + embed_path + '\">' + file + '</file>\n')

        qrc.write('    </qresource>\n')
        qrc.write('</RCC>')

main()
