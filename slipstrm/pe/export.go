package pe

import (
	"encoding/binary"
	"fmt"
	"io"
)

type ExportTable struct {
	OrdinalBase uint32
	Name        string
	Functions   []uint32
	Names       []string
	NameOrds    []uint16
}

func (m *Image) ReadImageExports(r io.ReadSeeker) (*ExportTable, error) {
	dirEntry := m.NTHeader.OptionalHeader.DataDirectory[ImageDirectoryEntryExport]

	// Load export directory dir
	dir := ImageExportDirectory{}
	if err := m.ReadAt(r, dirEntry.VirtualAddress, &dir); err != nil {
		return nil, fmt.Errorf("reading export directory: %w", err)
	}

	table := &ExportTable{
		OrdinalBase: dir.Base,
	}

	// Load functions
	table.Functions = make([]uint32, dir.NumberOfFunctions)
	if err := m.ReadAt(r, dir.AddressOfFunctions, table.Functions); err != nil {
		return nil, fmt.Errorf("reading export function addresses: %w", err)
	}

	// Load name addresses
	nameAddresses := make([]uint32, dir.NumberOfNames)
	if err := m.ReadAt(r, dir.AddressOfNames, nameAddresses); err != nil {
		return nil, fmt.Errorf("reading export function name addresses: %w", err)
	}

	// Load name ordinals
	table.NameOrds = make([]uint16, dir.NumberOfNames)
	if err := m.ReadAt(r, dir.AddressOfNameOrdinals, table.NameOrds); err != nil {
		return nil, fmt.Errorf("reading export name ordinals: %w", err)
	}

	// Load module name
	var err error
	if table.Name, err = m.ReadSzAt(r, dir.Name); err != nil {
		return nil, fmt.Errorf("reading export module name: %w", err)
	}

	// Load names
	for i, nameAddr := range nameAddresses {
		str, err := m.ReadSzAt(r, nameAddr)
		if err != nil {
			return nil, fmt.Errorf("reading name for function %d: %w", i, err)
		}
		table.Names = append(table.Names, str)
	}

	return table, nil
}

func (table *ExportTable) WriteImageExports(w io.Writer, addr uint32) error {
	dir := ImageExportDirectory{
		Base:              table.OrdinalBase,
		NumberOfFunctions: uint32(len(table.Functions)),
		NumberOfNames:     uint32(len(table.Names)),
	}

	nameAddrs := make([]uint32, len(table.Names))
	addr += uint32(binary.Size(dir))

	dir.AddressOfFunctions = addr
	addr += uint32(len(table.Functions) * 4)

	dir.AddressOfNames = addr
	addr += uint32(len(table.Names) * 4)

	dir.AddressOfNameOrdinals = addr
	addr += uint32(len(table.NameOrds) * 2)

	dir.Name = addr
	addr += uint32(len(table.Name) + 1)

	for i, name := range table.Names {
		nameAddrs[i] = addr
		addr += uint32(len(name) + 1)
	}

	if err := binary.Write(w, binary.LittleEndian, dir); err != nil {
		return err
	}

	if err := binary.Write(w, binary.LittleEndian, table.Functions); err != nil {
		return err
	}

	if err := binary.Write(w, binary.LittleEndian, nameAddrs); err != nil {
		return err
	}

	if err := binary.Write(w, binary.LittleEndian, table.NameOrds); err != nil {
		return err
	}

	if _, err := w.Write(append([]byte(table.Name), 0)); err != nil {
		return err
	}

	for _, name := range table.Names {
		if _, err := w.Write(append([]byte(name), 0)); err != nil {
			return err
		}
	}

	return nil
}
