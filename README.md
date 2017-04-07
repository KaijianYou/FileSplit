# FileSplit
一个根据字节数切分文件的小程序。

适用于 Windows 平台，在 Visual Studio 2015 上编译通过。

# Quick Start
    E:\FileSplit.exe demo.txt -b 1000  # 每 1000 B 切分一个子文件
    E:\FileSplit.exe demo.txt -k 10    # 每 10 KB 切分一个子文件
    E:\FileSplit.exe demo.txt -m 100   # 每 100 MB 切分一个子文件
    E:\FileSplit.exe demo.txt -g 1     # 每 1 GB 切分一个子文件
