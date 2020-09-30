# invoke SourceDir generated makefile for app_ble.pem3
app_ble.pem3: .libraries,app_ble.pem3
.libraries,app_ble.pem3: package/cfg/app_ble_pem3.xdl
	$(MAKE) -f F:\SeniorCapstone_LEGO\BrickFirmware\V1\App\TOOLS/src/makefile.libs

clean::
	$(MAKE) -f F:\SeniorCapstone_LEGO\BrickFirmware\V1\App\TOOLS/src/makefile.libs clean

