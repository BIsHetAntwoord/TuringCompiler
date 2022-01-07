#ifndef _TURINGCOMPILER_BACKEND_TURINGCOMPILER_HPP
#define _TURINGCOMPILER_BACKEND_TURINGCOMPILER_HPP

#include <cstddef>
#include <cstdint>
#include <vector>
#include <unordered_map>

#include "backend/turingstate.hpp"

struct Instr;

class TuringCompiler {
    private:
        Instr* instr;
        size_t num_instr;

        std::vector<TuringState> states;
        std::unordered_map<size_t, size_t> state_map;
        std::vector<size_t> jump_target_ips;
        std::unordered_map<size_t, uint16_t> jump_idx_map;

        size_t addState();
        size_t getStateForIP(size_t);
        void analyzeJumps();
        void compileInstr(size_t);
        void genPush(size_t, uint64_t, size_t, size_t);
        void genPop(size_t, size_t, size_t);
        void genDup(size_t, size_t, size_t);
        void genSwap(size_t, size_t, size_t, size_t);
        void genCopyByte(size_t, size_t, size_t);
        void genAdd(size_t, size_t, size_t);
        void genSub(size_t, size_t, size_t);
        void genAnd(size_t, size_t, size_t);
        void genOr(size_t, size_t, size_t);
        void genXor(size_t, size_t, size_t);
        void genLoad(size_t, size_t, size_t, size_t, size_t);
        void genLoadInd(size_t, size_t, size_t, size_t, size_t, size_t);
        void genStore(size_t, size_t, size_t, size_t, size_t);
        void genStoreInd(size_t, size_t, size_t, size_t, size_t, size_t);
        void genSetRet(size_t, size_t, size_t);

        void genPush8(size_t, const Instr&);
        void genPush16(size_t, const Instr&);
        void genPush32(size_t, const Instr&);
        void genPop8(size_t, const Instr&);
        void genPop16(size_t, const Instr&);
        void genPop32(size_t, const Instr&);
        void genDup8(size_t, const Instr&);
        void genDup16(size_t, const Instr&);
        void genDup32(size_t, const Instr&);
        void genSwap8(size_t, const Instr&);
        void genSwap16(size_t, const Instr&);
        void genSwap32(size_t, const Instr&);
        void genEnter(size_t, const Instr&);
        void genAlloc(size_t, const Instr&);
        void genFree(size_t, const Instr&);
        void genGetLocal8(size_t, const Instr&);
        void genGetLocal16(size_t, const Instr&);
        void genGetLocal32(size_t, const Instr&);
        void genGetLocalInd8(size_t, const Instr&);
        void genGetLocalInd16(size_t, const Instr&);
        void genGetLocalInd32(size_t, const Instr&);
        void genSetLocal8(size_t, const Instr&);
        void genSetLocal16(size_t, const Instr&);
        void genSetLocal32(size_t, const Instr&);
        void genSetLocalInd8(size_t, const Instr&);
        void genSetLocalInd16(size_t, const Instr&);
        void genSetLocalInd32(size_t, const Instr&);
        void genGetArg8(size_t, const Instr&);
        void genGetArg16(size_t, const Instr&);
        void genGetArg32(size_t, const Instr&);
        void genGetArgInd8(size_t, const Instr&);
        void genGetArgInd16(size_t, const Instr&);
        void genGetArgInd32(size_t, const Instr&);
        void genSetArg8(size_t, const Instr&);
        void genSetArg16(size_t, const Instr&);
        void genSetArg32(size_t, const Instr&);
        void genSetArgInd8(size_t, const Instr&);
        void genSetArgInd16(size_t, const Instr&);
        void genSetArgInd32(size_t, const Instr&);
        void genMakeArgs(size_t, const Instr&);
        void genGetGlobal8(size_t, const Instr&);
        void genGetGlobal16(size_t, const Instr&);
        void genGetGlobal32(size_t, const Instr&);
        void genGetGlobalInd8(size_t, const Instr&);
        void genGetGlobalInd16(size_t, const Instr&);
        void genGetGlobalInd32(size_t, const Instr&);
        void genSetGlobal8(size_t, const Instr&);
        void genSetGlobal16(size_t, const Instr&);
        void genSetGlobal32(size_t, const Instr&);
        void genSetGlobalInd8(size_t, const Instr&);
        void genSetGlobalInd16(size_t, const Instr&);
        void genSetGlobalInd32(size_t, const Instr&);
        void genAdd8(size_t, const Instr&);
        void genAdd16(size_t, const Instr&);
        void genAdd32(size_t, const Instr&);
        void genSub8(size_t, const Instr&);
        void genSub16(size_t, const Instr&);
        void genSub32(size_t, const Instr&);
        void genAnd8(size_t, const Instr&);
        void genAnd16(size_t, const Instr&);
        void genAnd32(size_t, const Instr&);
        void genOr8(size_t, const Instr&);
        void genOr16(size_t, const Instr&);
        void genOr32(size_t, const Instr&);
        void genXor8(size_t, const Instr&);
        void genXor16(size_t, const Instr&);
        void genXor32(size_t, const Instr&);
        void genIdxShft(size_t, const Instr&);
        void genJmp(size_t, const Instr&);
        void genJf(size_t, const Instr&);
        void genJt(size_t, const Instr&);
        void genCall(size_t, const Instr&);
        void genRet(size_t, const Instr&);
        void genSetRet8(size_t, const Instr&);
        void genSetRet16(size_t, const Instr&);
        void genSetRet32(size_t, const Instr&);
        void genAccept(size_t, const Instr&);
        void genReject(size_t, const Instr&);

        using CallbackPtr = void(TuringCompiler::*)(size_t, const Instr&);

        const static CallbackPtr GENERATOR_CALLBACKS[];
    public:
        TuringCompiler(Instr*, size_t);

        TuringMachine compile();
};

#endif
