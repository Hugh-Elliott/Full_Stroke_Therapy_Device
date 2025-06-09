

#if defined(__cplusplus)
extern "C" {
#endif

void AllMagellanCommands(PMDAxisHandle* phAxis);
void ProfileMove(PMDAxisHandle* phAxis);
void PMDWaitForEventExample();
void MemoryExample(PMDDeviceHandle* phDevice, PMDMemoryAddress memtype);
void AtlasCommands(PMDAxisHandle* phAxis);
void DeviceFunctions(PMDDeviceHandle* phDevice);
void ConnectToNetworkedMagellans(PMDDeviceHandle hDevice);

#if defined(__cplusplus)
}
#endif


