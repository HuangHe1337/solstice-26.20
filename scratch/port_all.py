import pefile, capstone, struct, re

exe20_path = r"C:\Users\soezz\Desktop\minecraft be versions\26.20\Minecraft.Windows.exe"
exe30_path = r"C:\Users\soezz\Desktop\minecraft be versions\26.30\Minecraft.Windows.exe"

def get_text_and_rdata(path):
    pe = pefile.PE(path, fast_load=True)
    text_s = [s for s in pe.sections if s.Name.decode().strip('\x00') == '.text'][0]
    rdata_s = [s for s in pe.sections if s.Name.decode().strip('\x00') == '.rdata'][0]
    with open(path, "rb") as f:
        f.seek(text_s.PointerToRawData)
        t_data = f.read(text_s.SizeOfRawData)
        f.seek(rdata_s.PointerToRawData)
        r_data = f.read(rdata_s.SizeOfRawData)
    return pe, text_s, t_data, rdata_s, r_data

pe30, t30_s, t30_data, r30_s, r30_data = get_text_and_rdata(exe30_path)
pe20, t20_s, t20_data, r20_s, r20_data = get_text_and_rdata(exe20_path)

with open(r"C:\Users\soezz\.gemini\antigravity\brain\ef65178c-4ab5-4dab-9399-a4e149cb0858\scratch\sigs_2630_rva.txt") as f:
    rva30_map = {}
    for l in f:
        parts = l.strip().split()
        if len(parts) == 2:
            rva30_map[parts[0]] = int(parts[1], 16)

already_done = {
    'ScreenView_setupAndRender', 'MinecraftGame_instance', 'HoverTextRenderer_render',
    'ItemUseInventoryTransaction_vtable', 'ComplexInventoryTransaction_vtable',
    'ItemUseOnActorInventoryTransaction_vtable', 'ItemReleaseInventoryTransaction_vtable',
    'ItemStack_vTable', 'NetworkStackItemDescriptor_ctor', 'GetCollisionShapeActorProxy_getFeetAttachPos',
    # and the 17 originally matched
    'ClientInstance_mBgfx', 'ContainerScreenController_handleAutoPlace', 'GameMode_getDestroyRate',
    'GuiData_displayClientMessage', 'ItemStack_getCustomName', 'MouseDevice_feed',
    'SimulatedPlayer_simulateJump', 'ConnectionRequest_create', 'BlockSource_fireBlockChanged',
    'JSON_parse', 'ConcreteBlockLegacy_getCollisionShapeForCamera', 'WaterBlockLegacy_getCollisionShapeForCamera',
    'ClientInstance_isPreGame', 'ConnectionRequest_create_DefaultInputMode', 'CameraComponent_applyRotation',
    'Actor_canSee', 'LevelRenderer_renderLevel'
}

remaining = {k: v for k, v in rva30_map.items() if k not in already_done}
print(f"Remaining to inspect: {len(remaining)}")

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
md.detail = True

def find_anchors_in_30(rva, max_len=600):
    off = rva - t30_s.VirtualAddress
    code = t30_data[off:off+max_len]
    strings = []
    constants = []
    for insn in md.disasm(code, rva):
        if insn.mnemonic == 'ret':
            # don't go too far past function end
            pass
        for op in insn.operands:
            if op.type == capstone.x86.X86_OP_MEM and op.mem.base == capstone.x86.X86_REG_RIP:
                target = insn.address + insn.size + op.mem.disp
                if r30_s.VirtualAddress <= target < r30_s.VirtualAddress + r30_s.Misc_VirtualSize:
                    r_off = target - r30_s.VirtualAddress
                    # read null terminated string
                    s_bytes = r30_data[r_off:r_off+80].split(b'\x00')[0]
                    if len(s_bytes) >= 5 and all(32 <= b < 127 for b in s_bytes):
                        strings.append(s_bytes)
            elif op.type == capstone.x86.X86_OP_IMM:
                val = op.imm
                if val > 0x10000 and val != 0xffffffff:
                    constants.append(val)
    return list(set(strings)), list(set(constants))

results = {}
for name, rva30 in remaining.items():
    strs, consts = find_anchors_in_30(rva30)
    # search strings in 26.20 rdata
    found_20_strs = []
    for s in strs:
        pos20 = r20_data.find(s)
        if pos20 != -1:
            found_20_strs.append((s.decode('latin1', errors='ignore'), r20_s.VirtualAddress + pos20))
    results[name] = (rva30, found_20_strs, consts[:3])

for name, (rva30, s20, consts) in sorted(results.items()):
    print(f"[{name}] (30 RVA: 0x{rva30:x})")
    if s20:
        for text, rva in s20[:3]:
            print(f"   str: '{text[:40]}' at 26.20 RVA 0x{rva:x}")
    if consts:
        print(f"   consts: {[hex(c) for c in consts]}")
