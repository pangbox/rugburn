package patcher

import (
	"bytes"
	"compress/zlib"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"log"

	"github.com/pangbox/rugburn/slipstrm/pe"
	"golang.org/x/exp/slices"
)

var (
	ijl15Sha256Sum = [32]uint8{
		0x33, 0x4a, 0xa1, 0x2f, 0x7d, 0xee, 0x45, 0x3d,
		0x1c, 0x6c, 0xb1, 0xb6, 0x61, 0xa3, 0xbb, 0x34,
		0x94, 0xd3, 0xe4, 0xcc, 0x9c, 0x2f, 0xf3, 0xf9,
		0x00, 0x20, 0x64, 0xc7, 0x84, 0x04, 0xe4, 0x3a,
	}
)

func UnpackOriginal(ijl15 []byte) ([]byte, error) {
	ijl15Reader := bytes.NewReader(ijl15)
	ijl15Module, err := pe.LoadPE32Image(ijl15Reader)
	if err != nil {
		return nil, fmt.Errorf("parsing original ijl15 dll: %w", err)
	}

	var origSectionName [pe.SectionNameLength]byte
	copy(origSectionName[:], ".orig")
	for _, section := range ijl15Module.Sections {
		if section.Name == origSectionName {
			zr, err := zlib.NewReader(bytes.NewReader(ijl15[section.PointerToRawData : section.PointerToRawData+section.SizeOfRawData]))
			if err != nil {
				return nil, fmt.Errorf("unpacking original ijl15 dll from patched dll: %w", err)
			}
			uncompressed, err := io.ReadAll(zr)
			if err != nil {
				return nil, fmt.Errorf("uncompressing original ijl15 dll from patched dll: %w", err)
			}
			err = zr.Close()
			if err != nil {
				return nil, fmt.Errorf("uncompressing original ijl15 dll from patched dll: %w", err)
			}
			return uncompressed, nil
		}
	}

	return ijl15, nil
}

func CheckOriginal(ijl15 []byte) bool {
	return sha256.Sum256(ijl15) == ijl15Sha256Sum
}

func Patch(log *log.Logger, ijl15, rugburn []byte) ([]byte, error) {
	// Load goat
	ijl15Reader := bytes.NewReader(ijl15)
	ijl15Module, err := pe.LoadPE32Image(ijl15Reader)
	if err != nil {
		return nil, fmt.Errorf("parsing original ijl15 dll: %w", err)
	}
	log.Printf("Loaded PE module for original ijl15. Sections=%d", len(ijl15Module.Sections))

	// Load payload
	payloadReader := bytes.NewReader(rugburn)
	payloadModule, err := pe.LoadPE32Image(payloadReader)
	if err != nil {
		return nil, fmt.Errorf("parsing rugburn dll: %w", err)
	}
	payloadExports, err := payloadModule.ReadImageExports(payloadReader)
	if err != nil {
		return nil, fmt.Errorf("parsing export table from rugburn dll: %w", err)
	}
	log.Printf("Loaded PE module for rugburn dll. Sections=%d", len(payloadModule.Sections))

	// Initialize new module identical to goatfile.
	newModule := ijl15Module.Clone()

	// Discard ijl15's .reloc section.
	newModule.Sections = newModule.Sections[:5]

	// Populate section data for original sections.
	sectionDatas := [][]byte{}
	for _, section := range newModule.Sections {
		sectionDatas = append(sectionDatas, ijl15[section.PointerToRawData:section.PointerToRawData+section.SizeOfRawData])
	}

	// Memory alignment/addressing setup.
	sectionAlign := newModule.NTHeader.OptionalHeader.SectionAlignment
	fileAlign := newModule.NTHeader.OptionalHeader.FileAlignment
	tailSection := newModule.Sections[len(newModule.Sections)-1]
	calcEnd := func(a, l uint32) uint32 {
		if l == 0 {
			return a + sectionAlign
		}
		return (a + l + sectionAlign - 1) / sectionAlign * sectionAlign
	}
	appendAddr := calcEnd(tailSection.VirtualAddress, tailSection.VirtualSize)
	addrOffset := appendAddr - payloadModule.Sections[0].VirtualAddress
	appendOffset := tailSection.PointerToRawData + tailSection.SizeOfRawData

	// Merge reloc tables.
	tailSection = newModule.Sections[len(newModule.Sections)-1]
	appendAddr = calcEnd(tailSection.VirtualAddress, tailSection.VirtualSize)
	ijl15Relocs, err := ijl15Module.ReadImageBaseRelocs(ijl15Reader)
	if err != nil {
		return nil, fmt.Errorf("reading original ijl15 base relocations: %w", err)
	}
	rugburnRelocs, err := payloadModule.ReadImageBaseRelocs(payloadReader)
	if err != nil {
		return nil, fmt.Errorf("reading rugburn base relocations: %w", err)
	}
	newRelocs := slices.Clone(ijl15Relocs)
	for _, reloc := range rugburnRelocs {
		reloc.Offset += addrOffset
		newRelocs = append(newRelocs, reloc)
	}

	// Inject payload sections.
	log.Printf("Injecting new sections at 0x%08x (offset: +0x%08x)", appendAddr, addrOffset)

	// Copy payload sections except for .reloc and .edata sections.
	for i, section := range payloadModule.Sections[:2] {
		// Make padded data for insertion into new image.
		paddedLen := (int(section.SizeOfRawData) + int(fileAlign) - 1) / int(fileAlign) * int(fileAlign)
		data := make([]byte, paddedLen)
		copy(data, rugburn[section.PointerToRawData:section.PointerToRawData+section.SizeOfRawData])

		name := fmt.Sprintf(".slip%d", i)
		copy(section.Name[:], name)
		section.VirtualAddress += addrOffset
		section.SizeOfRawData = uint32(paddedLen)
		section.PointerToRawData = appendOffset
		appendOffset += section.SizeOfRawData

		// Relocate section addrs to mathc new virtual address.
		pe.RelocateSection(newRelocs, ijl15Module.NTHeader.OptionalHeader.ImageBase+addrOffset-payloadModule.NTHeader.OptionalHeader.ImageBase, data, section)

		newModule.Sections = append(newModule.Sections, section)
		sectionDatas = append(sectionDatas, data)
	}

	tailSection = newModule.Sections[len(newModule.Sections)-1]
	appendAddr = calcEnd(tailSection.VirtualAddress, tailSection.VirtualSize)

	// Make new export table. Take SlipstrmDllMain as EP and export OEP.
	newEP := uint32(0)
	for i, name := range payloadExports.Names {
		if name == "SlipstrmDllMain" {
			newEP = payloadExports.Functions[payloadExports.NameOrds[i]]
			break
		}
	}
	if newEP == 0 {
		return nil, errors.New("could not find SlipstrmDllMain in rugburn")
	}

	newEP += addrOffset
	oldEP := ijl15Module.NTHeader.OptionalHeader.AddressOfEntryPoint
	newModule.NTHeader.OptionalHeader.AddressOfEntryPoint = newEP
	log.Printf("New entrypoint will be 0x%08x", newEP)

	newModule.DOSStub = make([]byte, 16)
	binary.LittleEndian.PutUint32(newModule.DOSStub, oldEP)

	relocBuffer := new(bytes.Buffer)
	if _, err := pe.WriteRelocations(relocBuffer, newRelocs); err != nil {
		return nil, fmt.Errorf("generating new relocs section: %w", err)
	}

	relocPaddedLen := (relocBuffer.Len() + int(fileAlign) - 1) / int(fileAlign) * int(fileAlign)
	reloc := make([]byte, relocPaddedLen)
	copy(reloc, relocBuffer.Bytes())

	log.Printf("Adding new reloc section at 0x%08x", appendAddr)
	relocSection := pe.ImageSectionHeader{
		VirtualSize:          uint32(relocBuffer.Len()),
		VirtualAddress:       appendAddr,
		SizeOfRawData:        uint32(relocPaddedLen),
		PointerToRawData:     appendOffset,
		PointerToRelocations: 0,
		PointerToLinenumbers: 0,
		NumberOfRelocations:  0,
		NumberOfLinenumbers:  0,
		Characteristics:      pe.ImageSectionCharacteristicsContainsInitializedData | pe.ImageSectionCharacteristicsMemoryRead | pe.ImageSectionCharacteristicsMemoryDiscardable,
	}
	copy(relocSection.Name[:], ".reloc")
	sectionDatas = append(sectionDatas, reloc)
	newModule.Sections = append(newModule.Sections, relocSection)

	tailSection = newModule.Sections[len(newModule.Sections)-1]
	appendAddr = calcEnd(tailSection.VirtualAddress, tailSection.VirtualSize)
	appendOffset += relocSection.SizeOfRawData

	// Update reloc data directory.
	newModule.NTHeader.OptionalHeader.DataDirectory[pe.ImageDirectoryEntryBaseReloc].VirtualAddress = relocSection.VirtualAddress
	newModule.NTHeader.OptionalHeader.DataDirectory[pe.ImageDirectoryEntryBaseReloc].Size = uint32(relocBuffer.Len())

	// Add a copy of the original ijl15.
	origBuffer := new(bytes.Buffer)
	{
		zw := zlib.NewWriter(origBuffer)
		zw.Write(ijl15)
		zw.Close()
	}
	origPaddedLen := (origBuffer.Len() + int(fileAlign) - 1) / int(fileAlign) * int(fileAlign)
	orig := make([]byte, origPaddedLen)
	copy(orig, origBuffer.Bytes())
	log.Printf("Adding original ijl15 at 0x%08x", appendAddr)
	origSection := pe.ImageSectionHeader{
		VirtualSize:          uint32(origBuffer.Len()),
		VirtualAddress:       appendAddr,
		SizeOfRawData:        uint32(origPaddedLen),
		PointerToRawData:     appendOffset,
		PointerToRelocations: 0,
		PointerToLinenumbers: 0,
		NumberOfRelocations:  0,
		NumberOfLinenumbers:  0,
		Characteristics:      pe.ImageSectionCharacteristicsContainsInitializedData | pe.ImageSectionCharacteristicsMemoryRead | pe.ImageSectionCharacteristicsMemoryDiscardable,
	}
	copy(origSection.Name[:], ".orig")
	sectionDatas = append(sectionDatas, orig)
	newModule.Sections = append(newModule.Sections, origSection)

	tailSection = newModule.Sections[len(newModule.Sections)-1]
	appendAddr = calcEnd(tailSection.VirtualAddress, tailSection.VirtualSize)
	appendOffset += origSection.SizeOfRawData

	for i, section := range newModule.Sections {
		log.Printf("Section %d: addr=%08x size=%08x ; rawaddr=%08x rawsize=%08x: %s", i, section.VirtualAddress, section.VirtualSize, section.PointerToRawData, section.SizeOfRawData, string(section.Name[:]))
	}

	newModule.NTHeader.OptionalHeader.SizeOfImage = appendAddr

	outBuf := new(bytes.Buffer)

	log.Printf("Saving PE headers.")
	if err := newModule.SaveHeader(outBuf); err != nil {
		return nil, fmt.Errorf("saving new headers: %w", err)
	}

	log.Printf("Writing section data.")
	for i, data := range sectionDatas {
		if _, err := outBuf.Write(data); err != nil {
			return nil, fmt.Errorf("saving new section %s data: %w", newModule.Sections[i].Name, err)
		}
	}

	log.Printf("Finished.")
	return outBuf.Bytes(), nil
}
