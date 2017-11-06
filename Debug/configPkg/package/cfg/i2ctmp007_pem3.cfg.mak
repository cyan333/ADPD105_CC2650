# invoke SourceDir generated makefile for i2ctmp007.pem3
i2ctmp007.pem3: .libraries,i2ctmp007.pem3
.libraries,i2ctmp007.pem3: package/cfg/i2ctmp007_pem3.xdl
	$(MAKE) -f /Users/Fangming/Documents/CCS_Workspace/i2ctmp007_CC2650STK_TI/src/makefile.libs

clean::
	$(MAKE) -f /Users/Fangming/Documents/CCS_Workspace/i2ctmp007_CC2650STK_TI/src/makefile.libs clean

