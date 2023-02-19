package pe

import (
	"encoding/binary"
	"errors"
	"fmt"
	"io"

	"golang.org/x/exp/slices"
)

var (
	ErrBadMZSignature             = errors.New("mz: bad signature")
	ErrBadPESignature             = errors.New("pe: bad signature")
	ErrUnknownOptionalHeaderMagic = errors.New("pe: unknown optional header magic")
)

type Image struct {
	DOSHeader ImageDOSHeader
	DOSStub   []byte
	NTHeader  ImageNTHeaders
	Sections  []ImageSectionHeader
}

func LoadPE32Image(r io.ReadSeeker) (*Image, error) {
	img := &Image{}

	r.Seek(0, io.SeekStart)
	dos := ImageDOSHeader{}
	if err := binary.Read(r, binary.LittleEndian, &dos); err != nil {
		return nil, err
	}
	if dos.Signature != MZSignature {
		return nil, ErrBadMZSignature
	}
	img.DOSHeader = dos

	img.DOSStub = make([]byte, dos.NewHeaderAddr-SizeOfImageDOSHeader)
	if _, err := r.Read(img.DOSStub); err != nil {
		return nil, err
	}

	r.Seek(int64(dos.NewHeaderAddr), io.SeekStart)
	pesig := [4]byte{}
	if err := binary.Read(r, binary.LittleEndian, &pesig); err != nil {
		return nil, err
	}

	if pesig != PESignature {
		return nil, ErrBadPESignature
	}

	optmagic := uint16(0)
	r.Seek(int64(dos.NewHeaderAddr)+OffsetOfOptionalHeaderFromNTHeader, io.SeekStart)
	if err := binary.Read(r, binary.LittleEndian, &optmagic); err != nil {
		return nil, err
	}

	r.Seek(int64(dos.NewHeaderAddr), io.SeekStart)
	if optmagic != ImageNTOptionalHeaderMagic {
		return nil, ErrUnknownOptionalHeaderMagic
	}

	if err := binary.Read(r, binary.LittleEndian, &img.NTHeader); err != nil {
		return nil, err
	}

	r.Seek(int64(dos.NewHeaderAddr)+OffsetOfOptionalHeaderFromNTHeader+int64(img.NTHeader.FileHeader.SizeOfOptionalHeader), io.SeekStart)

	for i := uint16(0); i < img.NTHeader.FileHeader.NumberOfSections; i++ {
		section := ImageSectionHeader{}
		if err := binary.Read(r, binary.LittleEndian, &section); err != nil {
			return nil, err
		}
		img.Sections = append(img.Sections, section)
	}

	return img, nil
}

func (m *Image) SaveHeader(w io.Writer) error {
	cw := NewCountedWriter(w)
	dos := m.DOSHeader
	dos.NewHeaderAddr = uint32(len(m.DOSStub)) + SizeOfImageDOSHeader
	if err := binary.Write(cw, binary.LittleEndian, dos); err != nil {
		return err
	}
	if _, err := cw.Write(m.DOSStub); err != nil {
		return err
	}
	m.NTHeader.FileHeader.NumberOfSections = uint16(len(m.Sections))
	if err := binary.Write(cw, binary.LittleEndian, m.NTHeader); err != nil {
		return err
	}
	if err := binary.Write(cw, binary.LittleEndian, m.Sections); err != nil {
		return err
	}

	// Padding. note: size of headers should be file-aligned.
	headersz := int(m.NTHeader.OptionalHeader.SizeOfHeaders)
	if headersz < cw.BytesWritten {
		return fmt.Errorf("header too big: 0x%08x > 0x%08x", cw.BytesWritten, headersz)
	}
	if _, err := cw.Write(make([]byte, headersz-cw.BytesWritten)); err != nil {
		return err
	}
	return nil
}

func (m *Image) Clone() *Image {
	return &Image{
		DOSHeader: m.DOSHeader,
		DOSStub:   m.DOSStub,
		NTHeader:  m.NTHeader,
		Sections:  slices.Clone(m.Sections),
	}
}

func (m *Image) AddrToOffset(addr uint32) int64 {
	for _, section := range m.Sections {
		if addr >= section.VirtualAddress && addr < section.VirtualAddress+section.SizeOfRawData {
			return int64(addr + section.PointerToRawData - section.VirtualAddress)
		}
	}
	return -1
}

func (m *Image) OffsetToAddr(offset int64) uint32 {
	for _, section := range m.Sections {
		if uint32(offset) >= section.PointerToRawData && uint32(offset) < section.PointerToRawData+section.SizeOfRawData {
			return uint32(offset) + section.VirtualAddress - section.PointerToRawData
		}
	}
	return 0
}

func (m *Image) SeekToAddr(r io.Seeker, addr uint32) error {
	offset := m.AddrToOffset(addr)
	if offset == -1 {
		return fmt.Errorf("address 0x%08x is not within any initialized bounds", addr)
	}
	if _, err := r.Seek(offset, io.SeekStart); err != nil {
		return fmt.Errorf("seek to address 0x%08x (offset 0x%08x): %w", addr, offset, err)
	}
	return nil
}

func (m *Image) ReadAt(r io.ReadSeeker, addr uint32, data any) error {
	if err := m.SeekToAddr(r, addr); err != nil {
		return fmt.Errorf("seeking to %T structure: %w", data, err)
	}
	if err := binary.Read(r, binary.LittleEndian, data); err != nil {
		return fmt.Errorf("reading %T structure: %w", data, err)
	}
	return nil
}

func (m *Image) ReadSzAt(r io.ReadSeeker, addr uint32) (string, error) {
	if err := m.SeekToAddr(r, addr); err != nil {
		return "", fmt.Errorf("seeking to string addr 0x%08x: %w", addr, err)
	}

	str, err := m.ReadSz(r)
	if err != nil {
		return "", fmt.Errorf("reading string at addr 0x%08x: %w", addr, err)
	}

	return str, nil
}

func (m *Image) ReadSz(r io.Reader) (string, error) {
	name := []byte{}
	for {
		b := [1]byte{}
		if _, err := r.Read(b[:]); err != nil {
			return "", err
		}
		if b[0] == 0 {
			break
		}
		name = append(name, b[0])
	}
	return string(name), nil
}
