import pefile, capstone

exe20_path = r"C:\Users\soezz\Desktop\minecraft be versions\26.20\Minecraft.Windows.exe"
pe = pefile.PE(exe20_path, fast_load=True)
text_s = [s for s in pe.sections if s.Name.decode().strip('\x00') == '.text'][0]

with open(exe20_path, "rb") as f:
    f.seek(text_s.PointerToRawData)
    t_data = f.read(text_s.SizeOfRawData)

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)

# scan 0x1a43000 to 0x1a48000
start_rva = 0x1a43000
end_rva = 0x1a48000

p_start = start_rva - text_s.VirtualAddress
p_end = end_rva - text_s.VirtualAddress

funcs = []
for p in range(p_start, p_end):
    if t_data[p-1] in (0xcc, 0xc3) and t_data[p] not in (0xcc, 0xc3):
        rva = text_s.VirtualAddress + p
        chunk = t_data[p:p+30]
        insns = list(md.disasm(chunk, rva))
        if insns and insns[0].mnemonic in ('push', 'sub', 'mov', 'lea'):
            dis = "; ".join(f"{i.mnemonic} {i.op_str}" for i in insns[:4])
            funcs.append((rva, dis))

print(f"Functions found between 0x{start_rva:x} and 0x{end_rva:x}: {len(funcs)}")
for rva, dis in funcs:
    print(f"  0x{rva:x}: {dis}")
