package pe

import (
	"encoding/binary"
	"fmt"
	"io"
)

type BaseRelocation struct {
	Offset uint32
	Type   int
}

func (m *Image) ReadImageBaseRelocs(r io.ReadSeeker) ([]BaseRelocation, error) {
	relocs := []BaseRelocation{}
	dirEntry := m.NTHeader.OptionalHeader.DataDirectory[ImageDirectoryEntryBaseReloc]
	if dirEntry.Size == 0 {
		return relocs, nil
	}
	if err := m.SeekToAddr(r, dirEntry.VirtualAddress); err != nil {
		return nil, fmt.Errorf("reading base relocs: %w", err)
	}
	return ReadRelocations(r, dirEntry.Size)
}

func ReadRelocations(r io.Reader, length uint32) ([]BaseRelocation, error) {
	read := uint32(0)
	relocs := []BaseRelocation{}

	for read < length {
		hdr := ImageBaseRelocation{}
		if err := binary.Read(r, binary.LittleEndian, &hdr); err != nil {
			return nil, fmt.Errorf("reading base reloc block header: %w", err)
		}
		data := make([]uint16, (hdr.SizeOfBlock-8)/2)
		if err := binary.Read(r, binary.LittleEndian, &data); err != nil {
			return nil, fmt.Errorf("reading base reloc block body: %w", err)
		}
		for _, n := range data {
			relocs = append(relocs, BaseRelocation{
				Offset: hdr.VirtualAddress + uint32(n&0xFFF),
				Type:   int(n >> 12),
			})
		}
		read += hdr.SizeOfBlock
	}

	return relocs, nil
}

func WriteRelocations(w io.Writer, relocs []BaseRelocation) (uint32, error) {
	if len(relocs) == 0 {
		return 0, nil
	}

	written := uint32(0)

	data := make([]uint16, 0, 4096)
	for i := 0; i < len(relocs); {
		currentPage := relocs[i].Offset & 0xFFFFF000
		data = data[:0]
		for ; i < len(relocs) && relocs[i].Offset&0xFFFFF000 == currentPage; i++ {
			data = append(data, (uint16(relocs[i].Type<<12))|(uint16(relocs[i].Offset)&0xFFF))
		}
		hdr := ImageBaseRelocation{
			VirtualAddress: uint32(currentPage),
			SizeOfBlock:    uint32(8 + len(data)*2),
		}
		if err := binary.Write(w, binary.LittleEndian, hdr); err != nil {
			return 0, fmt.Errorf("writing reloc block header: %w", err)
		}
		if err := binary.Write(w, binary.LittleEndian, data); err != nil {
			return 0, fmt.Errorf("writing reloc block body: %w", err)
		}
		written += hdr.SizeOfBlock
	}

	return written, nil
}

func RelocateSection(rels []BaseRelocation, delta uint32, data []byte, section ImageSectionHeader) error {
	o := binary.LittleEndian

	if delta == 0 {
		return nil
	}

	for _, rel := range rels {
		if rel.Offset < section.VirtualAddress || rel.Offset >= section.VirtualAddress+uint32(len(data)) {
			continue
		}
		off := rel.Offset - section.VirtualAddress
		switch rel.Type {
		case ImageRelBasedHigh:
			o.PutUint16(data[off:off+2], uint16(uint32(o.Uint16(data[off:off+2]))+(delta>>16)))
		case ImageRelBasedLow:
			o.PutUint16(data[off:off+2], uint16(uint32(o.Uint16(data[off:off+2]))+(delta>>0)))
		case ImageRelBasedHighLow:
			o.PutUint32(data[off:off+4], o.Uint32(data[off:off+4])+delta)
		default:
		}
	}

	return nil
}
