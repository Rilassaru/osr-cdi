REN *.PLC *.GTO
REN *.STC *.GTS
REN *.CMP *.GTL
REN *.SOL *.GBL
REN *.STS *.GBS
REN *.PLS *.GBO
REN *.DIM *.GML
REN *.DRD *.TXT
DEL  *.DRI
DEL  *.MMC
DEL  *.MMS
DEL  *.DOC
DEL  *.L2
DEL  *.L3

MKDIR .\gerber
MOVE *.G* .\gerber
MOVE *.TXT .\gerber
pause
