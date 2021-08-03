from elfUtils import extract as f_elf_extract, openFile as f_elf_open
from struct import unpack as f_unpack


def main():
    elfObj = f_elf_open('Turbo_kiosk.elf')

    tableBin = f_elf_extract(elfObj, 0x1013cbf4, 0x1013dc04 - 0x1013cbf4)
    numFloats = 4 * (256 + 1)
    assert len(tableBin) == (numFloats * 4)

    table = f_unpack('>%uf' % numFloats, tableBin)
    table_str = '\n'.join([('f, '.join(map(str, table[i:i+4])) + 'f,') for i in range(0, numFloats, 4)])
    print(table_str)


if __name__ == '__main__':
    main()
