//==- TMS320C64XInstrInfo.td - TMS320C64X Instruction table -*- tablegen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file Copyright Jeremy Morse (jmorse+llvm@studentrobotics.org), pending
// what student robotics decides to do with this code
// Derived from CellSPU implementation, see LLVM's LICENSE.TXT
//
//===----------------------------------------------------------------------===//

// misc, to begin with

def SDT_Wrapper : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;
def SDT_brcond : SDTypeProfile<0, 2, [SDTCisVT<0, OtherVT>]>;
def SDT_call : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_call_seq_start : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_call_seq_end : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

// This just copies the last two operands to the node being operated upon into
// the output operands: this makes tablegen happy. Note that this will probably
// explode if ever applied to an automagically generated node, should only be
// used for custom-generated-nodes
def predwrapper : ComplexPattern<OtherVT, 2, "bounce_predicate", [], []>;

def brcond_node : SDNode<"TMSISD::BRCOND", SDT_brcond, [SDNPHasChain]>;
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_call_seq_start,
				[SDNPHasChain, SDNPOutFlag]>;
def callseq_end : SDNode<"ISD::CALLSEQ_END", SDT_call_seq_end,
				[SDNPHasChain, SDNPOptInFlag, SDNPOutFlag]>;
def cmpeq_node : SDNode<"TMSISD::CMPEQ", SDTIntBinOp, [SDNPOutFlag]>;
def cmpgt_node : SDNode<"TMSISD::CMPGT", SDTIntBinOp, [SDNPOutFlag]>;
def cmpgtu_node : SDNode<"TMSISD::CMPGTU", SDTIntBinOp, [SDNPOutFlag]>;
def cmplt_node : SDNode<"TMSISD::CMPLT", SDTIntBinOp, [SDNPOutFlag]>;
def cmpltu_node : SDNode<"TMSISD::CMPLTU", SDTIntBinOp, [SDNPOutFlag]>;
def Wrapper : SDNode<"TMSISD::WRAPPER", SDT_Wrapper>;

// XXX These should probably be marked as for codegen modelling only.
// Don't have flexibility to test right now though.
def fake_call_start : inst<(outs), (ins i32imm:$val), "#callstart",
		[(callseq_start timm:$val)], 0, unit_d>;
def fake_call_end : inst<(outs), (ins i32imm:$val, i32imm:$val2), "#callend",
		[(callseq_end timm:$val, timm:$val2)], 0, unit_d>;

def TMS320C64Xcall : SDNode<"TMSISD::CALL", SDT_call,
				[SDNPHasChain, SDNPOutFlag, SDNPOptInFlag]>;

def retflag : SDNode<"TMSISD::RET_FLAG", SDTNone, [SDNPHasChain, SDNPOptInFlag]>;



// Shifts and rotates

def srl_r : TMSInst<(outs GPRegs:$out), (ins GPRegs:$reg, GPRegs:$shift),
	"shr $reg, $shift, $out",
	[(set GPRegs:$out, (srl GPRegs:$reg, GPRegs:$shift))]>;

def srl_i5 : TMSInst<(outs GPRegs:$out), (ins GPRegs:$reg, i32imm:$shift),
	"shr $reg, $shift, $out",
	[(set GPRegs:$out, (srl GPRegs:$reg, (i8 uconst5:$shift)))]>;

def shl_r : TMSInst<(outs GPRegs:$out), (ins GPRegs:$reg, GPRegs:$shift),
	"shl $reg, $shift, $out",
	[(set GPRegs:$out, (shl GPRegs:$reg, GPRegs:$shift))]>;

def shl_i5 : TMSInst<(outs GPRegs:$out), (ins GPRegs:$reg, i32imm:$shift),
	"shl $reg, $shift, $out",
	[(set GPRegs:$out, (shl GPRegs:$reg, (i8 uconst5:$shift)))]>;


//Some arithmatic and logic

def add_r : TMSInst<(outs GPRegs:$reg), (ins GPRegs:$reg1, GPRegs:$reg2),
	"add $reg1, $reg2, $reg",
	[(set GPRegs:$reg, (add GPRegs:$reg1, GPRegs:$reg2))]>;

def add_i5 : TMSInst<(outs GPRegs:$reg),
	(ins GPRegs:$reg1, i32imm:$immarg),
	"add $reg1, $immarg, $reg",
	[(set GPRegs:$reg, (add GPRegs:$reg1, sconst5:$immarg))]>;

def and_r : TMSInst<(outs GPRegs:$reg), (ins GPRegs:$reg1, GPRegs:$reg2),
	"and $reg1, reg2, $reg",
	[(set GPRegs:$reg, (and GPRegs:$reg1, GPRegs:$reg2))]>;

def and_i5 : TMSInst<(outs GPRegs:$reg),
	(ins GPRegs:$reg1, i32imm:$immarg),
	"and $reg1, $immarg, $reg",
	[(set GPRegs:$reg, (and GPRegs:$reg1, sconst5:$immarg))]>;

def sub_r : TMSInst<(outs GPRegs:$reg), (ins GPRegs:$reg1, GPRegs:$reg2),
	"sub $reg1, $reg2, $reg",
	[(set GPRegs:$reg, (sub GPRegs:$reg1, GPRegs:$reg2))]>;

def sub_i5 : TMSInst<(outs GPRegs:$reg),
	(ins GPRegs:$reg1, i32imm:$immarg),
	"sub $reg1, $immarg, $reg",
	[(set GPRegs:$reg, (sub GPRegs:$reg1, sconst5:$immarg))]>;

// Loading consts: almost always as halfwords
def mvk : TMSInst<(outs GPRegs:$reg), (ins i32imm:$val),
	"mvk $val, $reg",
	[(set GPRegs:$reg, mvk_pred:$val)]>;

// Match ints in the mvk range
def : Pat<(i32 mvk_pred:$val), (mvk imm:$val)>;

// mvkl/mvkh: doesn't look like llvm can express setting a bitfield in a
// register, and these two insns only set a halfword in one reg. So the only
// full way of expressing this would be to split gpregs into halfword regs.
// Instead, don't let mvkl/mvkh match anything, and use a pattern to match
// and set 32 bit constants
def mvkl : TMSInst<(outs GPRegs:$reg), (ins i32imm:$val),
	"mvkl $val, $reg", []>;

// For following pattern we have to output something (the reg loaded) but also
// need to input a register to set (from mvkl). So make this 2addr.
def mvkh : TMSInst<(outs GPRegs:$reg), (ins i32imm:$val, GPRegs:$bees),
	"mvkh $val, $bees", []> {
	let Constraints = "$bees = $reg";
}

def : Pat<(i32 mvk_all_pred:$val),
	(mvkh mvk_all_pred:$val, (mvkl mvk_all_pred:$val))>;
def : Pat<(i32 (Wrapper tglobaladdr:$val)),
	(mvkh tglobaladdr:$val, (mvkl tglobaladdr:$val))>;

def lea_fail : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
		"add ${ptr:facebees}, $dst", [(set GPRegs:$dst, idxaddr:$ptr)]>;

// Loads: we can handle all post/pre inc/dec modes, and all indexing situations,
// with a) alignment limitation, b) offset limitation, c) sign extension
def ldb : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldb $ptr, $dst",
	[(set GPRegs:$dst, (sextloadi8 addr:$ptr))]>;

def ldb_idx : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldb $ptr, $dst",
	[(set GPRegs:$dst, (sextloadi8 idxaddr:$ptr))]>;

def ldbu : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldbu $ptr, $dst",
	[(set GPRegs:$dst, (zextloadi8 addr:$ptr))]>;

def ldbu_idx : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldbu $ptr, $dst",
	[(set GPRegs:$dst, (zextloadi8 idxaddr:$ptr))]>;

// FIXME: Memory alignment ;_;
def ldh : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldh $ptr, $dst",
	[(set GPRegs:$dst, (sextloadi16 addr:$ptr))]>;

def ldhu : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldhu $ptr, $dst",
	[(set GPRegs:$dst, (zextloadi16 addr:$ptr))]>;

def ldw : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldw $ptr, $dst",
	[(set GPRegs:$dst, (load addr:$ptr))]>;

def ldw_idx : TMSInst<(outs GPRegs:$dst), (ins mem_operand:$ptr),
	"ldw $ptr, $dst",
	[(set GPRegs:$dst, (load idxaddr:$ptr))]>;

def stb : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"stb $reg, $ptr",
	[(truncstorei8 GPRegs:$reg, addr:$ptr)]>;

def stb_idx : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"stb $reg, $ptr",
	[(truncstorei8 GPRegs:$reg, idxaddr:$ptr)]>;

def sth : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"sth $reg, $ptr",
	[(truncstorei16 GPRegs:$reg, addr:$ptr)]>;

def stw : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"stw $reg, $ptr",
	[(store GPRegs:$reg, addr:$ptr)]>;

def stw_idx : TMSInst<(outs), (ins mem_operand:$ptr, GPRegs:$reg),
	"stw $reg, $ptr",
	[(store GPRegs:$reg, idxaddr:$ptr)]>;


// Comparisons

def cmpeq : TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpeq $src1, $src2, $out",
		[(set GPRegs:$out, (cmpeq_node GPRegs:$src1, GPRegs:$src2))]>;

def cmpgt :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpgt $src1, $src2, $out",
		[(set GPRegs:$out, (cmpgt_node GPRegs:$src1, GPRegs:$src2))]>;

def cmpgtu :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpgtu $src1, $src2, $out",
		[(set GPRegs:$out, (cmpgtu_node GPRegs:$src1, GPRegs:$src2))]>;

def cmplt :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmplt $src1, $src2, $out",
		[(set GPRegs:$out, (cmplt_node GPRegs:$src1, GPRegs:$src2))]>;

def cmpltu :TMSInst<(outs GPRegs:$out), (ins GPRegs:$src1, GPRegs:$src2),
		"cmpltu $src1, $src2, $out",
		[(set GPRegs:$out, (cmpltu_node GPRegs:$src1, GPRegs:$src2))]>;

// Branch / call instructions

def branch : TMSInst<(outs), (ins brtarget:$block), "b $block",
			[(br bb:$block)]>;

def ti_brcond : TMSInst<(outs), (ins brtarget:$block, nodefault_pred:$bees),
	"b $block", [(brcond_node bb:$block, predwrapper:$bees)]> {
	let InOperandList = (ins brtarget:$block, nodefault_pred:$bees);
}

def callp : TMSInst<(outs), (ins calltarget:$dst),
			"callp $dst", [(TMS320C64Xcall tglobaladdr:$dst)]> {
	let isCall = 1;
	let Defs = [A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A16, A17, A18, A19,
		A20, A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, A31,
		B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B16, B17, B18, B19,
		B20, B21, B22, B23, B24, B25, B26, B27, B28, B29, B30, B31];
}

let isReturn = 1, isTerminator = 1 in {
	// NB: branch-to-register can only work on unit S2
	// Epilogue code should juggle the return addr into B3.
	def ret : inst<(outs), (ins), "br B3", [(retflag)], 1, unit_s>;
}
