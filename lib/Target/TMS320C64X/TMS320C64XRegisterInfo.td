//==- TMS320C64XRegisterInfo.td - TMS320C64X Register File -*- tablegen -*-==//
// 
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from CellSPU implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

class TMS320C64XMainReg<bits<6> num, bit side, string n> : Register<n> {
	field bits<6> Num = num;
	bit side = side;
}

def A0	:	TMS320C64XMainReg<0, 0, "A0">, DwarfRegNum<[0]>;
def A1	:	TMS320C64XMainReg<1, 0, "A1">, DwarfRegNum<[1]>;
def A2	:	TMS320C64XMainReg<2, 0, "A2">, DwarfRegNum<[2]>;
def A3	:	TMS320C64XMainReg<3, 0, "A3">, DwarfRegNum<[3]>;
def A4	:	TMS320C64XMainReg<4, 0, "A4">, DwarfRegNum<[4]>;
def A5	:	TMS320C64XMainReg<5, 0, "A5">, DwarfRegNum<[5]>;
def A6	:	TMS320C64XMainReg<6, 0, "A6">, DwarfRegNum<[6]>;
def A7	:	TMS320C64XMainReg<7, 0, "A7">, DwarfRegNum<[7]>;
def A8	:	TMS320C64XMainReg<8, 0, "A8">, DwarfRegNum<[8]>;
def A9	:	TMS320C64XMainReg<9, 0, "A9">, DwarfRegNum<[9]>;
def A10	:	TMS320C64XMainReg<10, 0, "A10">, DwarfRegNum<[10]>;
def A11	:	TMS320C64XMainReg<11, 0, "A11">, DwarfRegNum<[11]>;
def A12	:	TMS320C64XMainReg<12, 0, "A12">, DwarfRegNum<[12]>;
def A13	:	TMS320C64XMainReg<13, 0, "A13">, DwarfRegNum<[13]>;
def A14	:	TMS320C64XMainReg<14, 0, "A14">, DwarfRegNum<[14]>;
def A15	:	TMS320C64XMainReg<15, 0, "A15">, DwarfRegNum<[15]>;
def A16	:	TMS320C64XMainReg<16, 0, "A16">, DwarfRegNum<[16]>;
def A17	:	TMS320C64XMainReg<17, 0, "A17">, DwarfRegNum<[17]>;
def A18	:	TMS320C64XMainReg<18, 0, "A18">, DwarfRegNum<[18]>;
def A19	:	TMS320C64XMainReg<19, 0, "A19">, DwarfRegNum<[19]>;
def A20	:	TMS320C64XMainReg<20, 0, "A20">, DwarfRegNum<[20]>;
def A21	:	TMS320C64XMainReg<21, 0, "A21">, DwarfRegNum<[21]>;
def A22	:	TMS320C64XMainReg<22, 0, "A22">, DwarfRegNum<[22]>;
def A23	:	TMS320C64XMainReg<23, 0, "A23">, DwarfRegNum<[23]>;
def A24	:	TMS320C64XMainReg<24, 0, "A24">, DwarfRegNum<[24]>;
def A25	:	TMS320C64XMainReg<25, 0, "A25">, DwarfRegNum<[25]>;
def A26	:	TMS320C64XMainReg<26, 0, "A26">, DwarfRegNum<[26]>;
def A27	:	TMS320C64XMainReg<27, 0, "A27">, DwarfRegNum<[27]>;
def A28	:	TMS320C64XMainReg<28, 0, "A28">, DwarfRegNum<[28]>;
def A29	:	TMS320C64XMainReg<29, 0, "A29">, DwarfRegNum<[29]>;
def A30	:	TMS320C64XMainReg<30, 0, "A30">, DwarfRegNum<[30]>;
def A31	:	TMS320C64XMainReg<31, 0, "A31">, DwarfRegNum<[31]>;
def B0	:	TMS320C64XMainReg<32, 1, "B0">, DwarfRegNum<[32]>;
def B1	:	TMS320C64XMainReg<33, 1, "B1">, DwarfRegNum<[33]>;
def B2	:	TMS320C64XMainReg<34, 1, "B2">, DwarfRegNum<[34]>;
def B3	:	TMS320C64XMainReg<35, 1, "B3">, DwarfRegNum<[35]>;
def B4	:	TMS320C64XMainReg<36, 1, "B4">, DwarfRegNum<[36]>;
def B5	:	TMS320C64XMainReg<37, 1, "B5">, DwarfRegNum<[37]>;
def B6	:	TMS320C64XMainReg<38, 1, "B6">, DwarfRegNum<[38]>;
def B7	:	TMS320C64XMainReg<39, 1, "B7">, DwarfRegNum<[39]>;
def B8	:	TMS320C64XMainReg<40, 1, "B8">, DwarfRegNum<[40]>;
def B9	:	TMS320C64XMainReg<41, 1, "B9">, DwarfRegNum<[41]>;
def B10	:	TMS320C64XMainReg<42, 1, "B10">, DwarfRegNum<[42]>;
def B11	:	TMS320C64XMainReg<43, 1, "B11">, DwarfRegNum<[43]>;
def B12	:	TMS320C64XMainReg<44, 1, "B12">, DwarfRegNum<[44]>;
def B13	:	TMS320C64XMainReg<45, 1, "B13">, DwarfRegNum<[45]>;
def B14	:	TMS320C64XMainReg<46, 1, "B14">, DwarfRegNum<[46]>;
def B15	:	TMS320C64XMainReg<47, 1, "B15">, DwarfRegNum<[47]>;
def B16	:	TMS320C64XMainReg<48, 1, "B16">, DwarfRegNum<[48]>;
def B17	:	TMS320C64XMainReg<49, 1, "B17">, DwarfRegNum<[49]>;
def B18	:	TMS320C64XMainReg<50, 1, "B18">, DwarfRegNum<[50]>;
def B19	:	TMS320C64XMainReg<51, 1, "B19">, DwarfRegNum<[51]>;
def B20	:	TMS320C64XMainReg<52, 1, "B20">, DwarfRegNum<[52]>;
def B21	:	TMS320C64XMainReg<53, 1, "B21">, DwarfRegNum<[53]>;
def B22	:	TMS320C64XMainReg<54, 1, "B22">, DwarfRegNum<[54]>;
def B23	:	TMS320C64XMainReg<55, 1, "B23">, DwarfRegNum<[55]>;
def B24	:	TMS320C64XMainReg<56, 1, "B24">, DwarfRegNum<[56]>;
def B25	:	TMS320C64XMainReg<57, 1, "B25">, DwarfRegNum<[57]>;
def B26	:	TMS320C64XMainReg<58, 1, "B26">, DwarfRegNum<[58]>;
def B27	:	TMS320C64XMainReg<59, 1, "B27">, DwarfRegNum<[59]>;
def B28	:	TMS320C64XMainReg<60, 1, "B28">, DwarfRegNum<[60]>;
def B29	:	TMS320C64XMainReg<61, 1, "B29">, DwarfRegNum<[61]>;
def B30	:	TMS320C64XMainReg<62, 1, "B30">, DwarfRegNum<[62]>;
def B31	:	TMS320C64XMainReg<63, 1, "B31">, DwarfRegNum<[63]>;

def GeneralRegs : RegisterClass<"TMS320C64X", [i32], 32,
	[
		/* Following most other arches, make one half of registers
		 * volatile (and allocate first), the other half nonvolatile.
		 * Use B30 and B31 for frame/stack pointer, don't alloc. */
		A0, A1, A2, A3, A4, A5, A6, A7,
		A8, A9, A10, A11, A12, A13, A14, A15,
		B0, B1, B2, B3, B4, B5, B6, B7,
		B8, B9, B10, B11, B12, B13, B14, B15,
		A16, A17, A18, A19, A20, A21, A22, A23,
		A24, A25, A26, A27, A28, A29, A30, A31,
		B16, B17, B18, B19, B20, B21, B22, B23,
		B24, B25, B26, B27, B28, B29, B30, B31
	] >
{
let MethodProtos = [{
	iterator allocation_order_begin(const MachineFunction &MF) const;
	iterator allocation_order_end(const MachineFunction &MF) const;
}];
let MethodBodies = [{
	GeneralRegs::iterator
	GeneralRegs::allocation_order_begin(const MachineFunction &MF)
									const {
		 return begin();
	}
	GeneralRegs::iterator
	GeneralRegs::allocation_order_end(const MachineFunction &MF)
									const {
		// No allocation of B30 or B31
		return end()-2;
	}
}];
}

/* Copy+paste from above, but with different types */
def VectorRegs : RegisterClass<"TMS320C64X", [v4i8,v2i16], 32,
	[
		/* Following most other arches, make one half of registers
		 * volatile (and allocate first), the other half nonvolatile.
		 * Use B30 and B31 for frame/stack pointer, don't alloc. */
		A0, A1, A2, A3, A4, A5, A6, A7,
		A8, A9, A10, A11, A12, A13, A14, A15,
		B0, B1, B2, B3, B4, B5, B6, B7,
		B8, B9, B10, B11, B12, B13, B14, B15,
		A16, A17, A18, A19, A20, A21, A22, A23,
		A24, A25, A26, A27, A28, A29, A30, A31,
		B16, B17, B18, B19, B20, B21, B22, B23,
		B24, B25, B26, B27, B28, B29, B30, B31
	] >
{
let MethodProtos = [{
	iterator allocation_order_begin(const MachineFunction &MF) const;
	iterator allocation_order_end(const MachineFunction &MF) const;
}];
let MethodBodies = [{
	GeneralRegs::iterator
	GeneralRegs::allocation_order_begin(const MachineFunction &MF)
									const {
		 return begin();
	}
	GeneralRegs::iterator
	GeneralRegs::allocation_order_end(const MachineFunction &MF)
									const {
		// No allocation of B30 or B31
		return end()-2;
	}
}];
}
