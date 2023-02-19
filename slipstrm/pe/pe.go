package pe

var MZSignature = [2]byte{'M', 'Z'}
var PESignature = [4]byte{'P', 'E', 0, 0}

const (
	ImageNTOptionalHeaderMagic = 0x010b

	SizeOfImageDOSHeader        = 64
	SizeOfImageFileHeader       = 20
	SizeOfImageOptionalHeader32 = 224
	SizeOfImageNTHeaders32      = 248
	SizeOfImageDataDirectory    = 8

	NumDirectoryEntries = 16
	SectionNameLength   = 8

	MaxNumSections = 96

	OffsetOfOptionalHeaderFromNTHeader = 0x18
)

const (
	ImageFileMachineUnknown    = 0x0000
	ImageFileMachineTargetHost = 0x0001
	ImageFileMachinei386       = 0x014c
	ImageFileMachineR3000BE    = 0x0160
	ImageFileMachineR3000      = 0x0162
	ImageFileMachineR4000      = 0x0166
	ImageFileMachineR10000     = 0x0168
	ImageFileMachineWCEMIPSv2  = 0x0169
	ImageFileMachineAlpha      = 0x0184
	ImageFileMachineSH3        = 0x01a2
	ImageFileMachineSH3DSP     = 0x01a3
	ImageFileMachineSH3E       = 0x01a4
	ImageFileMachineSH4        = 0x01a6
	ImageFileMachineSH5        = 0x01a8
	ImageFileMachineARM        = 0x01c0
	ImageFileMachineTHUMB      = 0x01c2
	ImageFileMachineARMNT      = 0x01c4
	ImageFileMachineAM33       = 0x01d3
	ImageFileMachinePowerPC    = 0x01F0
	ImageFileMachinePowerPCFP  = 0x01f1
	ImageFileMachineIA64       = 0x0200
	ImageFileMachineMIPS16     = 0x0266
	ImageFileMachineAlpha64    = 0x0284
	ImageFileMachineMIPSFPU    = 0x0366
	ImageFileMachineMIPSFPU16  = 0x0466
	ImageFileMachineAXP64      = ImageFileMachineAlpha64
	ImageFileMachineTricore    = 0x0520
	ImageFileMachineCEF        = 0x00CE
	ImageFileMachineEBC        = 0x0EBC
	ImageFileMachineAMD64      = 0x8664
	ImageFileMachineM32R       = 0x9041
	ImageFileMachineARM64      = 0xAA64
	ImageFileMachineCEE        = 0x0C0E
	ImageFileMachineRISCV32    = 0x5032
	ImageFileMachineRISCV64    = 0x5064
	ImageFileMachineRISCV128   = 0x5128
)

const (
	ImageFileRelocsStripped       = 0x0001
	ImageFileExecutableImage      = 0x0002
	ImageFileLineNumsStripped     = 0x0004
	ImageFileLocalSymsStripped    = 0x0008
	ImageFileAggressiveWSTrim     = 0x0010
	ImageFileLargeAddressAware    = 0x0020
	ImageFileBytesReversedLo      = 0x0080
	ImageFile32BitMachine         = 0x0100
	ImageFileDebugStripped        = 0x0200
	ImageFileRemovableRunFromSwap = 0x0400
	ImageFileNetRunFromSwap       = 0x0800
	ImageFileSystem               = 0x1000
	ImageFileDLL                  = 0x2000
	ImageFileUPSystemOnly         = 0x4000
	ImageFileBytesReversedHi      = 0x8000
)

const (
	ImageSubsystemUnknown                = 0
	ImageSubsystemNative                 = 1
	ImageSubsystemWindowsGUI             = 2
	ImageSubsystemWindowsCUI             = 3
	ImageSubsystemOS2CUI                 = 5
	ImageSubsystemPOSIXCUI               = 7
	ImageSubsystemNativeWindows          = 8
	ImageSubsystemWindowsCEGUI           = 9
	ImageSubsystemEFIApplication         = 10
	ImageSubsystemEFIBootServiceDriver   = 11
	ImageSubsystemEFIRuntimeDriver       = 12
	ImageSubsystemEFIROM                 = 13
	ImageSubsystemXBox                   = 14
	ImageSubsystemWindowsBootApplication = 16
	ImageSubsystemXBoxCodeCatalog        = 17
)

const (
	ImageDLLCharacteristicsHighEntropyVA       = 0x0020
	ImageDLLCharacteristicsDynamicBase         = 0x0040
	ImageDLLCharacteristicsForceIntegrity      = 0x0080
	ImageDLLCharacteristicsNXCompat            = 0x0100
	ImageDLLCharacteristicsNoIsolation         = 0x0200
	ImageDLLCharacteristicsNoSEH               = 0x0400
	ImageDLLCharacteristicsNoBind              = 0x0800
	ImageDLLCharacteristicsAppContainer        = 0x1000
	ImageDLLCharacteristicsWDMDriver           = 0x2000
	ImageDLLCharacteristicsGuardCF             = 0x4000
	ImageDLLCharacteristicsTerminalServerAware = 0x8000
)

const (
	ImageDirectoryEntryExport        = 0
	ImageDirectoryEntryImport        = 1
	ImageDirectoryEntryResource      = 2
	ImageDirectoryEntryException     = 3
	ImageDirectoryEntrySecurity      = 4
	ImageDirectoryEntryBaseReloc     = 5
	ImageDirectoryEntryDebug         = 6
	ImageDirectoryEntryCopyright     = 7
	ImageDirectoryEntryArchitecture  = 7
	ImageDirectoryEntryGlobalPtr     = 8
	ImageDirectoryEntryTLS           = 9
	ImageDirectoryEntryLoadConfig    = 10
	ImageDirectoryEntryBoundImport   = 11
	ImageDirectoryEntryIAT           = 12
	ImageDirectoryEntryDelayImport   = 13
	ImageDirectoryEntryCOMDescriptor = 14
)

const (
	ImageSectionCharacteristicsNoPad                     = 0x00000008
	ImageSectionCharacteristicsContainsCode              = 0x00000020
	ImageSectionCharacteristicsContainsInitializedData   = 0x00000040
	ImageSectionCharacteristicsContainsUninitailizedData = 0x00000080
	ImageSectionCharacteristicsLinkOther                 = 0x00000100
	ImageSectionCharacteristicsLinkInfo                  = 0x00000200
	ImageSectionCharacteristicsLinkRemove                = 0x00000800
	ImageSectionCharacteristicsLinkCOMDAT                = 0x00001000
	ImageSectionCharacteristicsNoDeferSpecExc            = 0x00004000
	ImageSectionCharacteristicsGPRel                     = 0x00008000
	ImageSectionCharacteristicsMemoryFarData             = 0x00008000
	ImageSectionCharacteristicsMemoryPurgeable           = 0x00020000
	ImageSectionCharacteristicsMemory16Bit               = 0x00020000
	ImageSectionCharacteristicsMemoryLocked              = 0x00040000
	ImageSectionCharacteristicsMemoryPreload             = 0x00080000
	ImageSectionCharacteristicsAlign1Bytes               = 0x00100000
	ImageSectionCharacteristicsAlign2Bytes               = 0x00200000
	ImageSectionCharacteristicsAlign4Bytes               = 0x00300000
	ImageSectionCharacteristicsAlign8Bytes               = 0x00400000
	ImageSectionCharacteristicsAlign16Bytes              = 0x00500000
	ImageSectionCharacteristicsAlign32Bytes              = 0x00600000
	ImageSectionCharacteristicsAlign64Bytes              = 0x00700000
	ImageSectionCharacteristicsAlign128Bytes             = 0x00800000
	ImageSectionCharacteristicsAlign256Bytes             = 0x00900000
	ImageSectionCharacteristicsAlign512Bytes             = 0x00A00000
	ImageSectionCharacteristicsAlign1024Bytes            = 0x00B00000
	ImageSectionCharacteristicsAlign2048Bytes            = 0x00C00000
	ImageSectionCharacteristicsAlign4096Bytes            = 0x00D00000
	ImageSectionCharacteristicsAlign8192Bytes            = 0x00E00000
	ImageSectionCharacteristicsAlignMask                 = 0x00F00000
	ImageSectionCharacteristicsLinkNumRelocOverflow      = 0x01000000
	ImageSectionCharacteristicsMemoryDiscardable         = 0x02000000
	ImageSectionCharacteristicsMemoryNotCached           = 0x04000000
	ImageSectionCharacteristicsMemoryNotPaged            = 0x08000000
	ImageSectionCharacteristicsMemoryShared              = 0x10000000
	ImageSectionCharacteristicsMemoryExecute             = 0x20000000
	ImageSectionCharacteristicsMemoryRead                = 0x40000000
	ImageSectionCharacteristicsMemoryWrite               = 0x80000000
)

const (
	ImageSectionTLSCharacteristicsScaleIndex = 0x00000001
)

const (
	ImageRelBasedAbsolute         = 0
	ImageRelBasedHigh             = 1
	ImageRelBasedLow              = 2
	ImageRelBasedHighLow          = 3
	ImageRelBasedHighAdj          = 4
	ImageRelBasedMachineSpecific5 = 5
	ImageRelBasedReserved         = 6
	ImageRelBasedMachineSpecific7 = 7
	ImageRelBasedMachineSpecific8 = 8
	ImageRelBasedMachineSpecific9 = 9
	ImageRelBasedDir64            = 10
)

type ImageDOSHeader struct {
	Signature     [2]byte
	LastPageBytes uint16
	CountPages    uint16
	CountRelocs   uint16
	HeaderLen     uint16
	MinAlloc      uint16
	MaxAlloc      uint16
	InitialSS     uint16
	InitialSP     uint16
	Checksum      uint16
	InitialIP     uint16
	InitialCS     uint16
	RelocAddr     uint16
	OverlayNum    uint16
	Reserved      [4]uint16
	OEMID         uint16
	OEMInfo       uint16
	Reserved2     [10]uint16
	NewHeaderAddr uint32
}

type ImageFileHeader struct {
	Machine              uint16
	NumberOfSections     uint16
	TimeDateStamp        uint32
	PointerToSymbolTable uint32
	NumberOfSymbols      uint32
	SizeOfOptionalHeader uint16
	Characteristics      uint16
}

type ImageOptionalHeader struct {
	Magic                   uint16
	MajorLinkerVersion      uint8
	MinorLinkerVersion      uint8
	SizeOfCode              uint32
	SizeOfInitializedData   uint32
	SizeOfUninitializedData uint32
	AddressOfEntryPoint     uint32
	BaseOfCode              uint32
	BaseOfData              uint32

	ImageBase                   uint32
	SectionAlignment            uint32
	FileAlignment               uint32
	MajorOperatingSystemVersion uint16
	MinorOperatingSystemVersion uint16
	MajorImageVersion           uint16
	MinorImageVersion           uint16
	MajorSubsystemVersion       uint16
	MinorSubsystemVersion       uint16
	Win32VersionValue           uint32
	SizeOfImage                 uint32
	SizeOfHeaders               uint32
	CheckSum                    uint32
	Subsystem                   uint16
	DllCharacteristics          uint16
	SizeOfStackReserve          uint32
	SizeOfStackCommit           uint32
	SizeOfHeapReserve           uint32
	SizeOfHeapCommit            uint32
	LoaderFlags                 uint32
	NumberOfRvaAndSizes         uint32
	DataDirectory               [NumDirectoryEntries]ImageDataDirectory
}

type ImageNTHeaders struct {
	Signature      [4]byte
	FileHeader     ImageFileHeader
	OptionalHeader ImageOptionalHeader
}

type ImageDataDirectory struct {
	VirtualAddress uint32
	Size           uint32
}

type ImageSectionHeader struct {
	Name                 [SectionNameLength]byte
	VirtualSize          uint32
	VirtualAddress       uint32
	SizeOfRawData        uint32
	PointerToRawData     uint32
	PointerToRelocations uint32
	PointerToLinenumbers uint32
	NumberOfRelocations  uint16
	NumberOfLinenumbers  uint16
	Characteristics      uint32
}

type ImageBaseRelocation struct {
	VirtualAddress uint32
	SizeOfBlock    uint32
}

type ImageImportDescriptor struct {
	OriginalFirstThunk uint32
	TimeDateStamp      uint32
	ForwarderChain     uint32
	Name               uint32
	FirstThunk         uint32
}

type ImageExportDirectory struct {
	Characteristics       uint32
	TimeDateStamp         uint32
	MajorVersion          uint16
	MinorVersion          uint16
	Name                  uint32
	Base                  uint32
	NumberOfFunctions     uint32
	NumberOfNames         uint32
	AddressOfFunctions    uint32
	AddressOfNames        uint32
	AddressOfNameOrdinals uint32
}

type ImageTLSDirectory struct {
	StartAddressOfRawData uint32
	EndAddressOfRawData   uint32
	AddressOfIndex        uint32
	AddressOfCallBacks    uint32
	SizeOfZeroFill        uint32
	Characteristics       uint32
}
