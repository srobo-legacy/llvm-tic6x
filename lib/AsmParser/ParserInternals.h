//===-- ParserInternals.h - Definitions internal to the parser ---*- C++ -*--=//
//
//  This header file defines the various variables that are shared among the 
//  different components of the parser...
//
//===----------------------------------------------------------------------===//

#ifndef PARSER_INTERNALS_H
#define PARSER_INTERNALS_H

#include <stdio.h>
#define __STDC_LIMIT_MACROS

#include "llvm/InstrTypes.h"
#include "llvm/BasicBlock.h"
#include "llvm/ConstPoolVals.h"
#include "llvm/iOther.h"
#include "llvm/Method.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Assembly/Parser.h"
#include "llvm/Support/StringExtras.h"

class Module;

// Global variables exported from the lexer...
extern FILE *llvmAsmin;
extern int llvmAsmlineno;

// Globals exported by the parser...
extern string CurFilename;
Module *RunVMAsmParser(const string &Filename, FILE *F);


// UnEscapeLexed - Run through the specified buffer and change \xx codes to the
// appropriate character.  If AllowNull is set to false, a \00 value will cause
// an exception to be thrown.
//
// If AllowNull is set to true, the return value of the function points to the
// last character of the string in memory.
//
char *UnEscapeLexed(char *Buffer, bool AllowNull = false);


// ThrowException - Wrapper around the ParseException class that automatically
// fills in file line number and column number and options info.
//
// This also helps me because I keep typing 'throw new ParseException' instead 
// of just 'throw ParseException'... sigh...
//
static inline void ThrowException(const string &message,
				  int LineNo = -1) {
  if (LineNo == -1) LineNo = llvmAsmlineno;
  // TODO: column number in exception
  throw ParseException(CurFilename, message, LineNo);
}

// ValID - Represents a reference of a definition of some sort.  This may either
// be a numeric reference or a symbolic (%var) reference.  This is just a 
// discriminated union.
//
// Note that I can't implement this class in a straight forward manner with 
// constructors and stuff because it goes in a union, and GCC doesn't like 
// putting classes with ctor's in unions.  :(
//
struct ValID {
  enum {
    NumberVal, NameVal, ConstSIntVal, ConstUIntVal, ConstStringVal, 
    ConstFPVal, ConstNullVal
  } Type;

  union {
    int      Num;         // If it's a numeric reference
    char    *Name;        // If it's a named reference.  Memory must be free'd.
    int64_t  ConstPool64; // Constant pool reference.  This is the value
    uint64_t UConstPool64;// Unsigned constant pool reference.
    double   ConstPoolFP; // Floating point constant pool reference
  };

  static ValID create(int Num) {
    ValID D; D.Type = NumberVal; D.Num = Num; return D;
  }

  static ValID create(char *Name) {
    ValID D; D.Type = NameVal; D.Name = Name; return D;
  }

  static ValID create(int64_t Val) {
    ValID D; D.Type = ConstSIntVal; D.ConstPool64 = Val; return D;
  }

  static ValID create(uint64_t Val) {
    ValID D; D.Type = ConstUIntVal; D.UConstPool64 = Val; return D;
  }

  static ValID create_conststr(char *Name) {
    ValID D; D.Type = ConstStringVal; D.Name = Name; return D;
  }

  static ValID create(double Val) {
    ValID D; D.Type = ConstFPVal; D.ConstPoolFP = Val; return D;
  }

  static ValID createNull() {
    ValID D; D.Type = ConstNullVal; return D;
  }

  inline void destroy() const {
    if (Type == NameVal || Type == ConstStringVal)
      free(Name);    // Free this strdup'd memory...
  }

  inline ValID copy() const {
    if (Type != NameVal && Type != ConstStringVal) return *this;
    ValID Result = *this;
    Result.Name = strdup(Name);
    return Result;
  }

  inline string getName() const {
    switch (Type) {
    case NumberVal     : return string("#") + itostr(Num);
    case NameVal       : return Name;
    case ConstStringVal: return string("\"") + Name + string("\"");
    case ConstFPVal    : return ftostr(ConstPoolFP);
    case ConstNullVal  : return "null";
    case ConstUIntVal  :
    case ConstSIntVal  : return string("%") + itostr(ConstPool64);
    default:
      assert(0 && "Unknown value!");
      abort();
    }
  }
};



template<class SuperType>
class PlaceholderValue : public SuperType {
  ValID D;
  int LineNum;
public:
  PlaceholderValue(const Type *Ty, const ValID &d) : SuperType(Ty), D(d) {
    LineNum = llvmAsmlineno;
  }
  ValID &getDef() { return D; }
  int getLineNum() const { return LineNum; }
};

struct TypePlaceHolderHelper : public OpaqueType {
  TypePlaceHolderHelper(const Type *Ty) : OpaqueType() {
    assert(Ty == Type::TypeTy);
  }
};


struct InstPlaceHolderHelper : public Instruction {
  InstPlaceHolderHelper(const Type *Ty) : Instruction(Ty, UserOp1, "") {}

  virtual Instruction *clone() const { abort(); }
  virtual const char *getOpcodeName() const { return "placeholder"; }
};

struct BBPlaceHolderHelper : public BasicBlock {
  BBPlaceHolderHelper(const Type *Ty) : BasicBlock() {
    assert(Ty->isLabelType());
  }
};

struct MethPlaceHolderHelper : public Method {
  MethPlaceHolderHelper(const Type *Ty) : Method(cast<const MethodType>(Ty)) {}
};

typedef PlaceholderValue<TypePlaceHolderHelper>  TypePlaceHolder;
typedef PlaceholderValue<InstPlaceHolderHelper>  ValuePlaceHolder;
typedef PlaceholderValue<BBPlaceHolderHelper>    BBPlaceHolder;
typedef PlaceholderValue<MethPlaceHolderHelper>  MethPlaceHolder;

static inline ValID &getValIDFromPlaceHolder(const Value *Val) {
  switch (Val->getType()->getPrimitiveID()) {
  case Type::TypeTyID:   return ((TypePlaceHolder*)Val)->getDef();
  case Type::LabelTyID:  return ((BBPlaceHolder*)Val)->getDef();
  case Type::MethodTyID: return ((MethPlaceHolder*)Val)->getDef();
  default:               return ((ValuePlaceHolder*)Val)->getDef();
  }
}

static inline int getLineNumFromPlaceHolder(const Value *Val) {
  switch (Val->getType()->getPrimitiveID()) {
  case Type::TypeTyID:   return ((TypePlaceHolder*)Val)->getLineNum();
  case Type::LabelTyID:  return ((BBPlaceHolder*)Val)->getLineNum();
  case Type::MethodTyID: return ((MethPlaceHolder*)Val)->getLineNum();
  default:               return ((ValuePlaceHolder*)Val)->getLineNum();
  }
}

#endif
