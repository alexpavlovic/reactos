#include <internal/arm/asmmacro.S>

GENERATE_ARM_STUB KiSwapContext
GENERATE_ARM_STUB DbgBreakPointWithStatus 
GENERATE_ARM_STUB ExInterlockedAddLargeInteger 
GENERATE_ARM_STUB ExInterlockedAddLargeStatistic 
GENERATE_ARM_STUB ExInterlockedAddUlong 
GENERATE_ARM_STUB ExInterlockedCompareExchange64
GENERATE_ARM_STUB ExInterlockedDecrementLong 
GENERATE_ARM_STUB ExInterlockedExchangeUlong 
GENERATE_ARM_STUB ExInterlockedFlushSList 
GENERATE_ARM_STUB ExInterlockedIncrementLong 
GENERATE_ARM_STUB ExInterlockedInsertHeadList 
GENERATE_ARM_STUB ExInterlockedInsertTailList 
GENERATE_ARM_STUB ExInterlockedPopEntryList 
GENERATE_ARM_STUB ExInterlockedPopEntrySList 
GENERATE_ARM_STUB ExInterlockedPushEntryList 
GENERATE_ARM_STUB ExInterlockedPushEntrySList 
GENERATE_ARM_STUB ExInterlockedRemoveHeadList 
GENERATE_ARM_STUB ExfInterlockedAddUlong 
GENERATE_ARM_STUB ExfInterlockedCompareExchange64
GENERATE_ARM_STUB ExfInterlockedInsertHeadList 
GENERATE_ARM_STUB ExfInterlockedInsertTailList 
GENERATE_ARM_STUB ExfInterlockedPopEntryList 
GENERATE_ARM_STUB ExfInterlockedPushEntryList 
GENERATE_ARM_STUB ExfInterlockedRemoveHeadList 
GENERATE_ARM_STUB Exfi386InterlockedDecrementLong 
GENERATE_ARM_STUB Exfi386InterlockedExchangeUlong 
GENERATE_ARM_STUB Exfi386InterlockedIncrementLong 
GENERATE_ARM_STUB Exi386InterlockedDecrementLong 
GENERATE_ARM_STUB Exi386InterlockedExchangeUlong 
GENERATE_ARM_STUB Exi386InterlockedIncrementLong 
GENERATE_ARM_STUB InterlockedCompareExchange 
GENERATE_ARM_STUB InterlockedDecrement 
GENERATE_ARM_STUB InterlockedExchange 
GENERATE_ARM_STUB InterlockedExchangeAdd 
GENERATE_ARM_STUB InterlockedIncrement 
GENERATE_ARM_STUB InterlockedPopEntrySList 
GENERATE_ARM_STUB InterlockedPushEntrySList 
GENERATE_ARM_STUB Ke386CallBios 
GENERATE_ARM_STUB KeConnectInterrupt 
GENERATE_ARM_STUB KeDcacheFlushCount 
GENERATE_ARM_STUB KeDisconnectInterrupt 
GENERATE_ARM_STUB KeFlushEntireTb 
GENERATE_ARM_STUB KeGetRecommendedSharedDataAlignment 
GENERATE_ARM_STUB KeI386AllocateGdtSelectors 
GENERATE_ARM_STUB KeI386FlatToGdtSelector 
GENERATE_ARM_STUB KeI386ReleaseGdtSelectors 
GENERATE_ARM_STUB KeIcacheFlushCount 
GENERATE_ARM_STUB KeInitializeInterrupt 
GENERATE_ARM_STUB KeNumberProcessors 
GENERATE_ARM_STUB KeQueryActiveProcessors 
GENERATE_ARM_STUB KeRaiseUserException 
GENERATE_ARM_STUB KeRestoreFloatingPointState 
GENERATE_ARM_STUB KeSaveFloatingPointState 
GENERATE_ARM_STUB KeSaveStateForHibernate 
GENERATE_ARM_STUB KeSetDmaIoCoherency 
GENERATE_ARM_STUB KeSynchronizeExecution 
GENERATE_ARM_STUB KeUpdateRunTime 
GENERATE_ARM_STUB KeUpdateSystemTime 
GENERATE_ARM_STUB KeUserModeCallback 
GENERATE_ARM_STUB Kei386EoiHelper 
GENERATE_ARM_STUB KiCoprocessorError 
GENERATE_ARM_STUB KiDispatchInterrupt 
GENERATE_ARM_STUB KiUnexpectedInterrupt  
GENERATE_ARM_STUB MmGetPhysicalAddress 
GENERATE_ARM_STUB NtVdmControl 
GENERATE_ARM_STUB READ_REGISTER_BUFFER_UCHAR 
GENERATE_ARM_STUB READ_REGISTER_BUFFER_ULONG 
GENERATE_ARM_STUB READ_REGISTER_BUFFER_USHORT 
GENERATE_ARM_STUB READ_REGISTER_UCHAR 
GENERATE_ARM_STUB READ_REGISTER_ULONG 
GENERATE_ARM_STUB READ_REGISTER_USHORT 
GENERATE_ARM_STUB RtlCaptureContext 
GENERATE_ARM_STUB RtlCompareMemory 
GENERATE_ARM_STUB RtlCompareMemoryUlong 
GENERATE_ARM_STUB RtlFillMemory 
GENERATE_ARM_STUB RtlFillMemoryUlong 
GENERATE_ARM_STUB RtlGetCallersAddress 
GENERATE_ARM_STUB RtlMoveMemory 
GENERATE_ARM_STUB RtlPrefetchMemoryNonTemporal 
GENERATE_ARM_STUB RtlUlongByteSwap 
GENERATE_ARM_STUB RtlUlonglongByteSwap 
GENERATE_ARM_STUB RtlUnwind 
GENERATE_ARM_STUB RtlUshortByteSwap 
GENERATE_ARM_STUB RtlZeroMemory 
GENERATE_ARM_STUB WRITE_REGISTER_BUFFER_UCHAR 
GENERATE_ARM_STUB WRITE_REGISTER_BUFFER_ULONG 
GENERATE_ARM_STUB WRITE_REGISTER_BUFFER_USHORT 
GENERATE_ARM_STUB WRITE_REGISTER_UCHAR 
GENERATE_ARM_STUB WRITE_REGISTER_ULONG 
GENERATE_ARM_STUB WRITE_REGISTER_USHORT 
GENERATE_ARM_STUB _abnormal_termination 
GENERATE_ARM_STUB _alldiv 
GENERATE_ARM_STUB _alldvrm 
GENERATE_ARM_STUB _allmul 
GENERATE_ARM_STUB _alloca_probe 
GENERATE_ARM_STUB _allrem 
GENERATE_ARM_STUB _allshl 
GENERATE_ARM_STUB _allshr 
GENERATE_ARM_STUB _aulldiv 
GENERATE_ARM_STUB _aulldvrm 
GENERATE_ARM_STUB _aullrem 
GENERATE_ARM_STUB _aullshr 
GENERATE_ARM_STUB _except_handler2 
GENERATE_ARM_STUB _except_handler3
GENERATE_ARM_STUB _global_unwind2  
GENERATE_ARM_STUB _local_unwind2  
GENERATE_ARM_STUB KiSaveProcessorControlState
GENERATE_ARM_STUB KiInitializeUserApc
GENERATE_ARM_STUB KeDisableInterrupts
GENERATE_ARM_STUB KeContextToTrapFrame
GENERATE_ARM_STUB KiDispatchException
GENERATE_ARM_STUB NtSetLdtEntries
GENERATE_ARM_STUB NtRaiseException
GENERATE_ARM_STUB NtCallbackReturn
GENERATE_ARM_STUB NtContinue
GENERATE_ARM_STUB KiSwapProcess
GENERATE_ARM_STUB MmUpdatePageDir
GENERATE_ARM_STUB KeArmInitThreadWithContext
GENERATE_ARM_STUB MmGetPfnForProcess
GENERATE_ARM_STUB MmCreateVirtualMapping
GENERATE_ARM_STUB CmpInitializeMachineDependentConfiguration
GENERATE_ARM_STUB KeI386VdmInitialize
GENERATE_ARM_STUB KdDebuggerInitialize1
GENERATE_ARM_STUB MmSetDirtyPage
GENERATE_ARM_STUB MmSetCleanPage
GENERATE_ARM_STUB MmIsDirtyPage
GENERATE_ARM_STUB MmEnableVirtualMapping
GENERATE_ARM_STUB MmCreatePageFileMapping
GENERATE_ARM_STUB MmDeleteVirtualMapping
GENERATE_ARM_STUB MmDisableVirtualMapping
GENERATE_ARM_STUB MmIsPageSwapEntry
GENERATE_ARM_STUB MmSetPageProtect
GENERATE_ARM_STUB MmIsPagePresent
GENERATE_ARM_STUB MmCreateHyperspaceMapping
GENERATE_ARM_STUB MmDeleteHyperspaceMapping
GENERATE_ARM_STUB MmDeletePageFileMapping
GENERATE_ARM_STUB MmRawDeleteVirtualMapping
GENERATE_ARM_STUB MmCreateVirtualMappingUnsafe
GENERATE_ARM_STUB MmInitializeHandBuiltProcess
GENERATE_ARM_STUB MmCreateProcessAddressSpace
GENERATE_ARM_STUB Mmi386ReleaseMmInfo
GENERATE_ARM_STUB RtlCreateUserThread
GENERATE_ARM_STUB RtlInitializeContext
GENERATE_ARM_STUB RtlpGetExceptionAddress
GENERATE_ARM_STUB RtlDispatchException
GENERATE_ARM_STUB DebugService2
GENERATE_ARM_STUB KdPortPutByteEx
GENERATE_ARM_STUB KdPortInitializeEx
GENERATE_ARM_STUB KdpGdbStubInit
GENERATE_ARM_STUB KeSwitchKernelStack
GENERATE_ARM_STUB MiInitPageDirectoryMap
GENERATE_ARM_STUB MmGetPageDirectory
GENERATE_ARM_STUB MmInitGlobalKernelPageDirectory
GENERATE_ARM_STUB MmDeletePageTable
GENERATE_ARM_STUB MmGetPageProtect
GENERATE_ARM_STUB MmCreateVirtualMappingForKernel
GENERATE_ARM_STUB MiGetUserPageDirectoryCount
GENERATE_ARM_STUB RtlpGetStackLimits
GENERATE_ARM_STUB KiInitMachineDependent
GENERATE_ARM_STUB KiComputeTimerTableIndex
GENERATE_ARM_STUB _SEHCurrentRegistration
GENERATE_ARM_STUB _SEHUnregisterFrame
GENERATE_ARM_STUB _SEHRegisterFrame
GENERATE_ARM_STUB _SEHCleanHandlerEnvironment
GENERATE_ARM_STUB _SEHGlobalUnwind

