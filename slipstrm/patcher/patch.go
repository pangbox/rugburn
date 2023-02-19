package patcher

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"log"

	"github.com/pangbox/rugburn/slipstrm/pe"
	"golang.org/x/exp/slices"
)

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

	// Update reloc data directory.
	newModule.NTHeader.OptionalHeader.DataDirectory[pe.ImageDirectoryEntryBaseReloc].VirtualAddress = relocSection.VirtualAddress
	newModule.NTHeader.OptionalHeader.DataDirectory[pe.ImageDirectoryEntryBaseReloc].Size = uint32(relocBuffer.Len())

	for i, section := range newModule.Sections {
		log.Printf("Section %d: addr=%08x size=%08x ; rawaddr=%08x rawsize=%08x: %s", i, section.VirtualAddress, section.VirtualSize, section.PointerToRawData, section.SizeOfRawData, string(section.Name[:]))
	}

	newModule.NTHeader.OptionalHeader.SizeOfImage = appendAddr

	outBuf := new(bytes.Buffer)

	if err := newModule.SaveHeader(outBuf); err != nil {
		return nil, fmt.Errorf("saving new headers: %w", err)
	}

	for i, data := range sectionDatas {
		if _, err := outBuf.Write(data); err != nil {
			return nil, fmt.Errorf("saving new section %s data: %w", newModule.Sections[i].Name, err)
		}
	}

	return outBuf.Bytes(), nil
}
