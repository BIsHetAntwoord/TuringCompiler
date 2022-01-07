#include "backend/turingcompiler.hpp"
#include "backend/instr.hpp"

#include <iostream>

const TuringCompiler::CallbackPtr TuringCompiler::GENERATOR_CALLBACKS[] = {
    TuringCompiler::genPush8,
    TuringCompiler::genPush16,
    TuringCompiler::genPush32,
    TuringCompiler::genPop8,
    TuringCompiler::genPop16,
    TuringCompiler::genPop32,
    TuringCompiler::genDup8,
    TuringCompiler::genDup16,
    TuringCompiler::genDup32,
    TuringCompiler::genSwap8,
    TuringCompiler::genSwap16,
    TuringCompiler::genSwap32,
    TuringCompiler::genEnter,
    TuringCompiler::genAlloc,
    TuringCompiler::genFree,
    TuringCompiler::genGetLocal8,
    TuringCompiler::genGetLocal16,
    TuringCompiler::genGetLocal32,
    TuringCompiler::genGetLocalInd8,
    TuringCompiler::genGetLocalInd16,
    TuringCompiler::genGetLocalInd32,
    TuringCompiler::genSetLocal8,
    TuringCompiler::genSetLocal16,
    TuringCompiler::genSetLocal32,
    TuringCompiler::genSetLocalInd8,
    TuringCompiler::genSetLocalInd16,
    TuringCompiler::genSetLocalInd32,
    TuringCompiler::genGetArg8,
    TuringCompiler::genGetArg16,
    TuringCompiler::genGetArg32,
    TuringCompiler::genGetArgInd8,
    TuringCompiler::genGetArgInd16,
    TuringCompiler::genGetArgInd32,
    TuringCompiler::genSetArg8,
    TuringCompiler::genSetArg16,
    TuringCompiler::genSetArg32,
    TuringCompiler::genSetArgInd8,
    TuringCompiler::genSetArgInd16,
    TuringCompiler::genSetArgInd32,
    TuringCompiler::genMakeArgs,
    TuringCompiler::genGetGlobal8,
    TuringCompiler::genGetGlobal16,
    TuringCompiler::genGetGlobal32,
    TuringCompiler::genGetGlobalInd8,
    TuringCompiler::genGetGlobalInd16,
    TuringCompiler::genGetGlobalInd32,
    TuringCompiler::genSetGlobal8,
    TuringCompiler::genSetGlobal16,
    TuringCompiler::genSetGlobal32,
    TuringCompiler::genSetGlobalInd8,
    TuringCompiler::genSetGlobalInd16,
    TuringCompiler::genSetGlobalInd32,
    TuringCompiler::genAdd8,
    TuringCompiler::genAdd16,
    TuringCompiler::genAdd32,
    TuringCompiler::genSub8,
    TuringCompiler::genSub16,
    TuringCompiler::genSub32,
    TuringCompiler::genAnd8,
    TuringCompiler::genAnd16,
    TuringCompiler::genAnd32,
    TuringCompiler::genOr8,
    TuringCompiler::genOr16,
    TuringCompiler::genOr32,
    TuringCompiler::genXor8,
    TuringCompiler::genXor16,
    TuringCompiler::genXor32,
    TuringCompiler::genIdxShft,
    TuringCompiler::genJmp,
    TuringCompiler::genJf,
    TuringCompiler::genJt,
    TuringCompiler::genCall,
    TuringCompiler::genRet,
    TuringCompiler::genSetRet8,
    TuringCompiler::genSetRet16,
    TuringCompiler::genSetRet32,
    TuringCompiler::genAccept,
    TuringCompiler::genReject
};

TuringCompiler::TuringCompiler(Instr* instr, size_t num_instr) : instr(instr), num_instr(num_instr) {
    TuringTransition self_trans = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::STAY, 0};

    TuringState accept_state;
    accept_state.def_transition = self_trans;
    this->states.push_back(accept_state);

    self_trans.next_state = 1;
    TuringState reject_state;
    reject_state.def_transition = self_trans;
    this->states.push_back(reject_state);

    this->analyzeJumps();
}

size_t TuringCompiler::addState() {
    TuringTransition reject_trans = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::STAY, 1};
    TuringState new_state;
    new_state.def_transition = reject_trans;

    size_t result = this->states.size();
    this->states.push_back(new_state);
    return result;
}

size_t TuringCompiler::getStateForIP(size_t ip) {
    if(this->state_map.count(ip) == 0)
        this->state_map[ip] = this->addState();
    return this->state_map[ip];
}

void TuringCompiler::analyzeJumps() {
    auto add_jt = [&](size_t target) {
        this->jump_idx_map[target] = this->jump_target_ips.size();
        this->jump_target_ips.push_back(target);
    };

    for(size_t i = 0; i < num_instr; ++i) {
        const Instr& in = this->instr[i];

        switch(in.opcode) {
            case Opcode::CALL:
                add_jt(i + 1);
                break;
            default:
                break;
        }
    }
}

void TuringCompiler::genPush(size_t start_state, uint64_t integer, size_t bytes, size_t next_state) {
    size_t current_state = start_state;
    for(size_t i = 0; i < bytes; ++i) {
        uint8_t constant = (integer >> (i * 8)) & 0xFF;
        size_t trans_state = (i == (bytes - 1) ? next_state : this->addState());

        TuringTransition trans = {TRANS_WILDCARD, constant, TuringDirection::RIGHT, trans_state};
        this->states[current_state].def_transition = trans;
        current_state = trans_state;
    }
}

void TuringCompiler::genPop(size_t start_state, size_t bytes, size_t next_state) {
    size_t current_state = start_state;
    for(size_t i = 0; i < bytes; ++i) {
        size_t trans_state = (i == (bytes - 1) ? next_state : this->addState());

        TuringTransition trans = {TRANS_WILDCARD, 0, TuringDirection::LEFT, trans_state};
        this->states[current_state].def_transition = trans;
        current_state = trans_state;
    }
}

void TuringCompiler::genDup(size_t start_state, size_t bytes, size_t next_state) {
    size_t current_state = start_state;
    for(size_t i = 0; i < bytes; ++i) {
        size_t inter_state = (i == (bytes - 1)) ? next_state : this->addState();
        this->genCopyByte(current_state, bytes, inter_state);
        current_state = inter_state;
    }
}

void TuringCompiler::genSwap(size_t start_state, size_t bytes, size_t final_state, size_t offset) {
    size_t current_state = start_state;
    for(size_t i = 0; i < bytes; ++i) {
        size_t next_state = this->addState();
        TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, next_state};
        this->states[current_state].def_transition = move_left;
        current_state = next_state;
    }

    for(size_t i = 0; i < bytes; ++i) {
        size_t merge_state = (i == (bytes - 1)) ? final_state : this->addState();

        size_t back_states[256];
        for(size_t j = 0; j < 256; ++j)
            back_states[j] = this->addState();

        for(size_t j = 0; j < 256; ++j) {
            size_t next_state = this->addState();
            TuringTransition split_up = {j, j, TuringDirection::LEFT, next_state};
            this->states[current_state].transitions.push_back(split_up);

            for(size_t k = 1; k < (bytes + offset); ++k) {
                size_t inter_state = this->addState();
                TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, inter_state};
                this->states[next_state].def_transition = move_left;
                next_state = inter_state;
            }

            for(size_t k = 0; k < 256; ++k) {
                TuringTransition write_back = {k, j, TuringDirection::RIGHT, back_states[k]};
                this->states[next_state].transitions.push_back(write_back);
            }
        }

        for(size_t j = 0; j < 256; ++j) {
            size_t next_state = back_states[j];
            for(size_t k = 1; k < (bytes + offset); ++k) {
                size_t inter_state = this->addState();
                TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, inter_state};
                this->states[next_state].def_transition = move_right;
                next_state = inter_state;
            }

            TuringTransition write_back = {TRANS_WILDCARD, j, TuringDirection::RIGHT, merge_state};
            this->states[next_state].def_transition = write_back;
        }

        current_state = merge_state;
    }
}

void TuringCompiler::genCopyByte(size_t start_state, size_t offset, size_t next_state) {
    size_t current_state = start_state;
    for(size_t i = 0; i < offset; ++i) {
        size_t trans_state = this->addState();
        TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, trans_state};
        this->states[current_state].def_transition = move_left;
        current_state = trans_state;
    }

    for(size_t i = 0; i < 256; ++i) {
        size_t inter_state = this->addState();
        TuringTransition switch_trans = {i, TRANS_WILDCARD, TuringDirection::RIGHT, inter_state};
        this->states[current_state].transitions.push_back(switch_trans);

        for(size_t j = 1; j < offset; ++j) {
            size_t new_state = this->addState();
            TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, new_state};
            this->states[inter_state].def_transition = move_right;
            inter_state = new_state;
        }

        TuringTransition write_back = {TRANS_WILDCARD, i, TuringDirection::RIGHT, next_state};
        this->states[inter_state].def_transition = write_back;
    }
}

void TuringCompiler::genAdd(size_t start_state, size_t bytes, size_t next_state) {
    size_t current_state = start_state;
    for(size_t i = 0; i < bytes; ++i) {
        size_t trans_state = this->addState();
        TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, trans_state};
        this->states[current_state].def_transition = move_left;
        current_state = trans_state;
    }

    size_t normal_state = current_state;
    size_t carry_state = current_state;

    size_t end_state = next_state;

    for(size_t i = 0; i < bytes; ++i) {
        size_t next_normal_state = i == (bytes - 1) ? end_state : this->addState();
        size_t next_carry_state = i == (bytes - 1) ? end_state : this->addState();
        for(size_t j = 0; j < 256; ++j) {
            for(size_t k = 0; k < (1 + (i > 0)); ++k) {
                size_t trans_state = this->addState();
                TuringTransition split = {j, 0, TuringDirection::LEFT, trans_state};

                size_t opt_carry_state = k == 0 ? normal_state : carry_state;
                this->states[opt_carry_state].transitions.push_back(split);

                for(size_t l = 1; l < bytes; ++l) {
                    size_t inter_state = this->addState();
                    TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, inter_state};
                    this->states[trans_state].def_transition = move_left;
                    trans_state = inter_state;
                }

                for(size_t l = 0; l < 256; ++l) {
                    size_t inter_state = (j + l + k) >= 256 ? next_carry_state : next_normal_state;
                    TuringTransition write_back = {l, (j + l + k) % 256, TuringDirection::RIGHT, inter_state};
                    this->states[trans_state].transitions.push_back(write_back);
                }
            }
        }

        if(i != (bytes - 1)) {
            for(size_t j = 0; j < bytes; ++j) {
                size_t inter_state_normal = this->addState();
                size_t inter_state_carry = this->addState();
                TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, inter_state_normal};
                this->states[next_normal_state].def_transition = move_right;
                move_right.next_state = inter_state_carry;
                this->states[next_carry_state].def_transition = move_right;
                next_normal_state = inter_state_normal;
                next_carry_state = inter_state_carry;
            }
        }

        normal_state = next_normal_state;
        carry_state = next_carry_state;
    }
}

void TuringCompiler::genSub(size_t start_state, size_t bytes, size_t next_state) {
    size_t current_state = start_state;
    for(size_t i = 0; i < bytes; ++i) {
        size_t trans_state = this->addState();
        TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, trans_state};
        this->states[current_state].def_transition = move_left;
        current_state = trans_state;
    }

    size_t normal_state = current_state;
    size_t carry_state = current_state;

    size_t end_state = next_state;

    for(size_t i = 0; i < bytes; ++i) {
        size_t next_normal_state = i == (bytes - 1) ? end_state : this->addState();
        size_t next_carry_state = i == (bytes - 1) ? end_state : this->addState();
        for(size_t j = 0; j < 256; ++j) {
            for(size_t k = 0; k < (1 + (i > 0)); ++k) {
                size_t trans_state = this->addState();
                TuringTransition split = {j, 0, TuringDirection::LEFT, trans_state};

                size_t opt_carry_state = k == 0 ? normal_state : carry_state;
                this->states[opt_carry_state].transitions.push_back(split);

                for(size_t l = 1; l < bytes; ++l) {
                    size_t inter_state = this->addState();
                    TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, inter_state};
                    this->states[trans_state].def_transition = move_left;
                    trans_state = inter_state;
                }

                for(size_t l = 0; l < 256; ++l) {
                    size_t inter_state = (l < (j + k)) ? next_carry_state : next_normal_state;
                    TuringTransition write_back = {l, (l - j - k) % 256, TuringDirection::RIGHT, inter_state};
                    this->states[trans_state].transitions.push_back(write_back);
                }
            }
        }

        if(i != (bytes - 1)) {
            for(size_t j = 0; j < bytes; ++j) {
                size_t inter_state_normal = this->addState();
                size_t inter_state_carry = this->addState();
                TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, inter_state_normal};
                this->states[next_normal_state].def_transition = move_right;
                move_right.next_state = inter_state_carry;
                this->states[next_carry_state].def_transition = move_right;
                next_normal_state = inter_state_normal;
                next_carry_state = inter_state_carry;
            }
        }

        normal_state = next_normal_state;
        carry_state = next_carry_state;
    }
}

void TuringCompiler::genAnd(size_t start_state, size_t bytes, size_t next_state) {
    size_t trans_state = this->addState();
    TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, trans_state};
    this->states[start_state].def_transition = move_left;

    for(size_t i = 0; i < bytes; ++i) {
        size_t end_state = (bytes == 1) ? next_state : this->addState();

        for(size_t j = 0; j < 256; ++j) {
            size_t inter_state = this->addState();
            TuringTransition split_byte = {j, 0, TuringDirection::LEFT, inter_state};
            this->states[trans_state].transitions.push_back(split_byte);

            for(size_t k = 1; k < bytes; ++k) {
                move_left.next_state = this->addState();
                this->states[inter_state].def_transition = move_left;
                inter_state = move_left.next_state;
            }

            for(size_t k = 0; k < 256; ++k) {
                TuringTransition write_back = {k, (j & k), TuringDirection::RIGHT, end_state};
                this->states[inter_state].transitions.push_back(write_back);
            }
        }

        trans_state = end_state;

        for(size_t j = 2; j < bytes; ++j) {
            TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, this->addState()};
            this->states[trans_state].def_transition = move_right;
            trans_state = move_right.next_state;
        }
    }

    if(bytes > 1) {
        TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, next_state};
        this->states[trans_state].def_transition = move_right;
    }
}

void TuringCompiler::genOr(size_t start_state, size_t bytes, size_t next_state) {
    size_t trans_state = this->addState();
    TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, trans_state};
    this->states[start_state].def_transition = move_left;

    for(size_t i = 0; i < bytes; ++i) {
        size_t end_state = (bytes == 1) ? next_state : this->addState();

        for(size_t j = 0; j < 256; ++j) {
            size_t inter_state = this->addState();
            TuringTransition split_byte = {j, 0, TuringDirection::LEFT, inter_state};
            this->states[trans_state].transitions.push_back(split_byte);

            for(size_t k = 1; k < bytes; ++k) {
                move_left.next_state = this->addState();
                this->states[inter_state].def_transition = move_left;
                inter_state = move_left.next_state;
            }

            for(size_t k = 0; k < 256; ++k) {
                TuringTransition write_back = {k, (j | k), TuringDirection::RIGHT, end_state};
                this->states[inter_state].transitions.push_back(write_back);
            }
        }

        trans_state = end_state;

        for(size_t j = 2; j < bytes; ++j) {
            TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, this->addState()};
            this->states[trans_state].def_transition = move_right;
            trans_state = move_right.next_state;
        }
    }

    if(bytes > 1) {
        TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, next_state};
        this->states[trans_state].def_transition = move_right;
    }
}

void TuringCompiler::genXor(size_t start_state, size_t bytes, size_t next_state) {
    size_t trans_state = this->addState();
    TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, trans_state};
    this->states[start_state].def_transition = move_left;

    for(size_t i = 0; i < bytes; ++i) {
        size_t end_state = (bytes == 1) ? next_state : this->addState();

        for(size_t j = 0; j < 256; ++j) {
            size_t inter_state = this->addState();
            TuringTransition split_byte = {j, 0, TuringDirection::LEFT, inter_state};
            this->states[trans_state].transitions.push_back(split_byte);

            for(size_t k = 1; k < bytes; ++k) {
                move_left.next_state = this->addState();
                this->states[inter_state].def_transition = move_left;
                inter_state = move_left.next_state;
            }

            for(size_t k = 0; k < 256; ++k) {
                TuringTransition write_back = {k, (j ^ k), TuringDirection::RIGHT, end_state};
                this->states[inter_state].transitions.push_back(write_back);
            }
        }

        trans_state = end_state;

        for(size_t j = 2; j < bytes; ++j) {
            TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, this->addState()};
            this->states[trans_state].def_transition = move_right;
            trans_state = move_right.next_state;
        }
    }

    if(bytes > 1) {
        TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, next_state};
        this->states[trans_state].def_transition = move_right;
    }
}

void TuringCompiler::genLoad(size_t start_state, size_t bytes, size_t end_state, size_t offset, size_t base_token) {
    size_t current_state = start_state;
    for(size_t j = 0; j < bytes; ++j) {
        size_t writeback_state = (j == (bytes - 1)) ? end_state : this->addState();
        size_t next_state = this->addState();
        TuringTransition write_temp = {TRANS_WILDCARD, TAPE_TEMP1, TuringDirection::LEFT, next_state};
        this->states[current_state].def_transition = write_temp;

        current_state = next_state;
        next_state = this->addState();
        TuringTransition move_to_base_loop = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, current_state};
        TuringTransition move_to_base_found = {base_token, base_token, TuringDirection::RIGHT, next_state};
        this->states[current_state].def_transition = move_to_base_loop;
        this->states[current_state].transitions.push_back(move_to_base_found);
        current_state = next_state;

        for(size_t i = 0; i < (offset + j); ++i) {
            next_state = this->addState();
            TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, next_state};
            this->states[current_state].def_transition = move_right;
            current_state = next_state;
        }

        for(size_t i = 0; i < 256; ++i) {
            size_t split_state = this->addState();
            TuringTransition split_trans = {i, i, TuringDirection::RIGHT, split_state};
            this->states[current_state].transitions.push_back(split_trans);

            TuringTransition loop_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, split_state};
            this->states[split_state].def_transition = loop_right;

            TuringTransition write_back = {TAPE_TEMP1, i, TuringDirection::RIGHT, writeback_state};
            this->states[split_state].transitions.push_back(write_back);
        }

        current_state = writeback_state;
    }
}

void TuringCompiler::genLoadInd(size_t start_state, size_t bytes, size_t end_state, size_t base_offset, size_t max_ind, size_t base_token) {
    std::vector<size_t> state_tables[4];

    size_t current_state = this->addState();
    TuringTransition move_left = {TRANS_WILDCARD, 0, TuringDirection::LEFT, current_state};
    this->states[start_state].def_transition = move_left;

    for(size_t i = 0; i < max_ind; ++i) {
        state_tables[3].push_back(this->addState());

        for(size_t j = 0; j < 3; ++j) {
            if(i % (1 << (8*(j+1))) == 0)
                state_tables[4-j-2].push_back(this->addState());
        }
    }

    for(size_t i = 0; i < state_tables[0].size(); ++i) {
        TuringTransition split_trans = {i, 0, TuringDirection::LEFT, state_tables[0][i]};
        this->states[current_state].transitions.push_back(split_trans);
    }

    for(size_t i = 0; i < 3; ++i) {
        for(size_t j = 0; j < state_tables[i+1].size(); ++j) {
            size_t last_idx = j / 256;
            size_t cur_idx = j % 256;

            TuringDirection tape_dir = (i == 2) ? TuringDirection::STAY : TuringDirection::LEFT;

            TuringTransition split_trans = {cur_idx, 0, tape_dir, state_tables[i+1][j]};
            this->states[state_tables[i][last_idx]].transitions.push_back(split_trans);
        }
    }

    for(size_t i = 0; i < state_tables[3].size(); ++i) {
        this->genLoad(state_tables[3][i], bytes, end_state, base_offset + i, base_token);
    }
}

void TuringCompiler::genStore(size_t start_state, size_t bytes, size_t end_state, size_t offset, size_t base_token) {
    size_t current_state = start_state;

    for(size_t k = 0; k < bytes; ++k) {
        size_t next_iter_state = (k == (bytes - 1)) ? end_state : this->addState();
        size_t next_state = this->addState();
        TuringTransition move_left = {TRANS_WILDCARD, 0, TuringDirection::LEFT, next_state};
        this->states[current_state].def_transition = move_left;

        current_state = next_state;
        size_t loopback_state = this->addState();
        for(size_t i = 0; i < 256; ++i) {
            size_t split_state = this->addState();
            TuringTransition split_writeback = {i, TAPE_TEMP1, TuringDirection::LEFT, split_state};
            this->states[current_state].transitions.push_back(split_writeback);

            TuringTransition move_to_base_loop = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, split_state};
            this->states[split_state].def_transition = move_to_base_loop;

            next_state = this->addState();
            TuringTransition move_to_base_found = {base_token, base_token, TuringDirection::RIGHT, next_state};
            this->states[split_state].transitions.push_back(move_to_base_found);

            size_t move_pos = offset + (bytes - k - 1);
            for(size_t j = 0; j < move_pos; ++j) {
                size_t sub_state = this->addState();
                TuringTransition move_right = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, sub_state};
                this->states[next_state].def_transition = move_right;
                next_state = sub_state;
            }

            TuringTransition write_back = {TRANS_WILDCARD, i, TuringDirection::RIGHT, loopback_state};
            this->states[next_state].def_transition = write_back;
        }

        TuringTransition loop_to_temp = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, loopback_state};
        this->states[loopback_state].def_transition = loop_to_temp;
        TuringTransition found_trans = {TAPE_TEMP1, 0, TuringDirection::STAY, next_iter_state};
        this->states[loopback_state].transitions.push_back(found_trans);

        current_state = next_iter_state;
    }
}

void TuringCompiler::genStoreInd(size_t start_state, size_t bytes, size_t end_state, size_t base_offset, size_t max_ind, size_t base_token) {
    std::vector<size_t> state_tables[4];

    size_t current_state = this->addState();
    TuringTransition move_left = {TRANS_WILDCARD, 0, TuringDirection::LEFT, current_state};
    this->states[start_state].def_transition = move_left;

    for(size_t i = 0; i < max_ind; ++i) {
        state_tables[3].push_back(this->addState());

        for(size_t j = 0; j < 3; ++j) {
            if(i % (1 << (8*(j+1))) == 0)
                state_tables[4-j-2].push_back(this->addState());
        }
    }

    for(size_t i = 0; i < state_tables[0].size(); ++i) {
        TuringTransition split_trans = {i, 0, TuringDirection::LEFT, state_tables[0][i]};
        this->states[current_state].transitions.push_back(split_trans);
    }

    for(size_t i = 0; i < 3; ++i) {
        for(size_t j = 0; j < state_tables[i+1].size(); ++j) {
            size_t last_idx = j / 256;
            size_t cur_idx = j % 256;

            TuringDirection tape_dir = (i == 2) ? TuringDirection::STAY : TuringDirection::LEFT;

            TuringTransition split_trans = {cur_idx, 0, tape_dir, state_tables[i+1][j]};
            this->states[state_tables[i][last_idx]].transitions.push_back(split_trans);
        }
    }

    for(size_t i = 0; i < state_tables[3].size(); ++i) {
        this->genStore(state_tables[3][i], bytes, end_state, base_offset + i, base_token);
    }
}

void TuringCompiler::genSetRet(size_t start_state, size_t bytes, size_t end_state) {
    size_t current_state = start_state;
    for(size_t i = 0; i < bytes; ++i) {
        size_t final_state = (i == (bytes - 1)) ? end_state : this->addState();
        size_t next_state = this->addState();
        TuringTransition move_left = {TRANS_WILDCARD, 0, TuringDirection::LEFT, next_state};
        this->states[current_state].def_transition = move_left;

        current_state = next_state;
        size_t join_state = this->addState();
        for(size_t j = 0; j < 256; ++j) {
            next_state = this->addState();
            TuringTransition split_trans = {j, TAPE_TEMP1, TuringDirection::LEFT, next_state};
            this->states[current_state].transitions.push_back(split_trans);

            size_t found_state = this->addState();
            TuringTransition loop_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, next_state};
            this->states[next_state].def_transition = loop_left;
            TuringTransition found_ap = {TAPE_AP, TAPE_AP, TuringDirection::LEFT, found_state};
            this->states[next_state].transitions.push_back(found_ap);

            for(size_t k = 0; k < (2 + i); ++k) {
                size_t inter_state = this->addState();
                TuringTransition move_left_inter = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, inter_state};
                this->states[found_state].def_transition = move_left_inter;
                found_state = inter_state;
            }

            TuringTransition write_back = {TRANS_WILDCARD, j, TuringDirection::RIGHT, join_state};
            this->states[found_state].def_transition = write_back;
        }

        TuringTransition move_back = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::RIGHT, join_state};
        this->states[join_state].def_transition = move_back;
        TuringTransition found_temp = {TAPE_TEMP1, 0, TuringDirection::STAY, final_state};
        this->states[join_state].transitions.push_back(found_temp);

        current_state = final_state;
    }
}

void TuringCompiler::genPush8(size_t ip, const Instr& instr) {
    this->genPush(this->getStateForIP(ip), instr.integer, 1, this->getStateForIP(ip + 1));
}

void TuringCompiler::genPush16(size_t ip, const Instr& instr) {
    this->genPush(this->getStateForIP(ip), instr.integer, 2, this->getStateForIP(ip + 1));
}

void TuringCompiler::genPush32(size_t ip, const Instr& instr) {
    this->genPush(this->getStateForIP(ip), instr.integer, 4, this->getStateForIP(ip + 1));
}

void TuringCompiler::genPop8(size_t ip, const Instr& instr) {
    this->genPop(this->getStateForIP(ip), 1, this->getStateForIP(ip + 1));
}

void TuringCompiler::genPop16(size_t ip, const Instr& instr) {
    this->genPop(this->getStateForIP(ip), 2, this->getStateForIP(ip + 1));
}

void TuringCompiler::genPop32(size_t ip, const Instr& instr) {
    this->genPop(this->getStateForIP(ip), 4, this->getStateForIP(ip + 1));
}

void TuringCompiler::genDup8(size_t ip, const Instr& instr) {
    this->genDup(this->getStateForIP(ip), 1, this->getStateForIP(ip + 1));
}

void TuringCompiler::genDup16(size_t ip, const Instr& instr) {
    this->genDup(this->getStateForIP(ip), 2, this->getStateForIP(ip + 1));
}

void TuringCompiler::genDup32(size_t ip, const Instr& instr) {
    this->genDup(this->getStateForIP(ip), 4, this->getStateForIP(ip + 1));
}

void TuringCompiler::genSwap8(size_t ip, const Instr& instr) {
    this->genSwap(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer);
}

void TuringCompiler::genSwap16(size_t ip, const Instr& instr) {
    this->genSwap(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer);
}

void TuringCompiler::genSwap32(size_t ip, const Instr& instr) {
    this->genSwap(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer);
}

void TuringCompiler::genEnter(size_t ip, const Instr& instr) {
    size_t state = this->getStateForIP(ip);
    size_t next_state = this->getStateForIP(ip+1);
    TuringTransition trans = {TRANS_WILDCARD, TAPE_BP, TuringDirection::RIGHT, next_state};
    this->states[state].def_transition = trans;
}

void TuringCompiler::genAlloc(size_t ip, const Instr& instr) {
    size_t current = this->getStateForIP(ip);
    size_t end_state = this->getStateForIP(ip+1);

    for(size_t i = 0; i < instr.integer; ++i) {
        size_t next_state = i == (instr.integer - 1) ? end_state : this->addState();
        TuringTransition write_zero = {TRANS_WILDCARD, 0, TuringDirection::RIGHT, next_state};
        this->states[current].def_transition = write_zero;
        current = next_state;
    }
}

void TuringCompiler::genFree(size_t ip, const Instr& instr) {
    size_t current = this->getStateForIP(ip);
    size_t end_state = this->getStateForIP(ip+1);

    for(size_t i = 0; i < instr.integer; ++i) {
        size_t next_state = this->addState();
        TuringTransition write_zero = {TRANS_WILDCARD, 0, TuringDirection::LEFT, next_state};
        this->states[current].def_transition = write_zero;
        current = next_state;
    }
    this->states[current].def_transition = {TRANS_WILDCARD, 0, TuringDirection::STAY, end_state};
}

void TuringCompiler::genGetLocal8(size_t ip, const Instr& instr) {
    this->genLoad(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, TAPE_BP);
}

void TuringCompiler::genGetLocal16(size_t ip, const Instr& instr) {
    this->genLoad(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, TAPE_BP);
}

void TuringCompiler::genGetLocal32(size_t ip, const Instr& instr) {
    this->genLoad(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, TAPE_BP);
}

void TuringCompiler::genGetLocalInd8(size_t ip, const Instr& instr) {
    this->genLoadInd(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_BP);
}

void TuringCompiler::genGetLocalInd16(size_t ip, const Instr& instr) {
    this->genLoadInd(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_BP);
}

void TuringCompiler::genGetLocalInd32(size_t ip, const Instr& instr) {
    this->genLoadInd(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_BP);
}

void TuringCompiler::genSetLocal8(size_t ip, const Instr& instr) {
    this->genStore(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, TAPE_BP);
}

void TuringCompiler::genSetLocal16(size_t ip, const Instr& instr) {
    this->genStore(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, TAPE_BP);
}

void TuringCompiler::genSetLocal32(size_t ip, const Instr& instr) {
    this->genStore(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, TAPE_BP);
}

void TuringCompiler::genSetLocalInd8(size_t ip, const Instr& instr) {
    this->genStoreInd(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_BP);
}

void TuringCompiler::genSetLocalInd16(size_t ip, const Instr& instr) {
    this->genStoreInd(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_BP);
}

void TuringCompiler::genSetLocalInd32(size_t ip, const Instr& instr) {
    this->genStoreInd(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_BP);
}

void TuringCompiler::genGetArg8(size_t ip, const Instr& instr) {
    this->genLoad(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, TAPE_AP);
}

void TuringCompiler::genGetArg16(size_t ip, const Instr& instr) {
    this->genLoad(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, TAPE_AP);
}

void TuringCompiler::genGetArg32(size_t ip, const Instr& instr) {
    this->genLoad(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, TAPE_AP);
}

void TuringCompiler::genGetArgInd8(size_t ip, const Instr& instr) {
    this->genLoadInd(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_AP);
}

void TuringCompiler::genGetArgInd16(size_t ip, const Instr& instr) {
    this->genLoadInd(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_AP);
}

void TuringCompiler::genGetArgInd32(size_t ip, const Instr& instr) {
    this->genLoadInd(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_AP);
}

void TuringCompiler::genSetArg8(size_t ip, const Instr& instr) {
    this->genStore(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, TAPE_AP);
}

void TuringCompiler::genSetArg16(size_t ip, const Instr& instr) {
    this->genStore(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, TAPE_AP);
}

void TuringCompiler::genSetArg32(size_t ip, const Instr& instr) {
    this->genStore(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, TAPE_AP);
}

void TuringCompiler::genSetArgInd8(size_t ip, const Instr& instr) {
    this->genStoreInd(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_AP);
}

void TuringCompiler::genSetArgInd16(size_t ip, const Instr& instr) {
    this->genStoreInd(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_AP);
}

void TuringCompiler::genSetArgInd32(size_t ip, const Instr& instr) {
    this->genStoreInd(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_AP);
}

void TuringCompiler::genMakeArgs(size_t ip, const Instr& instr) {
    size_t current_state = this->getStateForIP(ip);
    size_t final_state = this->getStateForIP(ip+1);

    size_t bytes = instr.integer;

    if(bytes == 0) {
        TuringTransition write_ap = {TRANS_WILDCARD, TAPE_AP, TuringDirection::RIGHT, final_state};
        this->states[current_state].def_transition = write_ap;
        return;
    }

    for(size_t i = 0; i < bytes; ++i) {
        size_t next_state = this->addState();
        TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, next_state};
        this->states[current_state].def_transition = move_left;
        current_state = next_state;
    }

    size_t current_states[256];
    size_t next_states[256];

    auto make_buffer = [&]() {
        for(size_t i = 0; i < 256; ++i)
            next_states[i] = this->addState();
    };

    auto move_buffer = [&]() {
        for(size_t i = 0; i < 256; ++i)
            current_states[i] = next_states[i];
    };

    for(size_t i = 0; i < 256; ++i) {
        current_states[i] = this->addState();
        TuringTransition write_ap = {i, TAPE_AP, TuringDirection::RIGHT, current_states[i]};
        this->states[current_state].transitions.push_back(write_ap);
    }

    for(size_t i = 1; i < bytes; ++i) {
        make_buffer();

        for(size_t j = 0; j < 256; ++j) {
            size_t cur = current_states[j];
            for(size_t k = 0; k < 256; ++k) {
                TuringTransition write_back = {k, j, TuringDirection::RIGHT, next_states[k]};
                this->states[cur].transitions.push_back(write_back);
            }
        }
        move_buffer();
    }

    for(size_t i = 0; i < 256; ++i) {
        size_t cur = current_states[i];
        TuringTransition write_back = {TRANS_WILDCARD, i, TuringDirection::RIGHT, final_state};
        this->states[cur].def_transition = write_back;
    }
}

void TuringCompiler::genGetGlobal8(size_t ip, const Instr& instr) {
    this->genLoad(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, TAPE_GP);
}

void TuringCompiler::genGetGlobal16(size_t ip, const Instr& instr) {
    this->genLoad(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, TAPE_GP);
}

void TuringCompiler::genGetGlobal32(size_t ip, const Instr& instr) {
    this->genLoad(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, TAPE_GP);
}

void TuringCompiler::genGetGlobalInd8(size_t ip, const Instr& instr) {
    this->genLoadInd(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_GP);
}

void TuringCompiler::genGetGlobalInd16(size_t ip, const Instr& instr) {
    this->genLoadInd(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_GP);
}

void TuringCompiler::genGetGlobalInd32(size_t ip, const Instr& instr) {
    this->genLoadInd(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_GP);
}

void TuringCompiler::genSetGlobal8(size_t ip, const Instr& instr) {
    this->genStore(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, TAPE_GP);
}

void TuringCompiler::genSetGlobal16(size_t ip, const Instr& instr) {
    this->genStore(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, TAPE_GP);
}

void TuringCompiler::genSetGlobal32(size_t ip, const Instr& instr) {
    this->genStore(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, TAPE_GP);
}

void TuringCompiler::genSetGlobalInd8(size_t ip, const Instr& instr) {
    this->genStoreInd(this->getStateForIP(ip), 1, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_GP);
}

void TuringCompiler::genSetGlobalInd16(size_t ip, const Instr& instr) {
    this->genStoreInd(this->getStateForIP(ip), 2, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_GP);
}

void TuringCompiler::genSetGlobalInd32(size_t ip, const Instr& instr) {
    this->genStoreInd(this->getStateForIP(ip), 4, this->getStateForIP(ip+1), instr.integer, instr.integer2, TAPE_GP);
}

void TuringCompiler::genAdd8(size_t ip, const Instr& instr) {
    this->genAdd(this->getStateForIP(ip), 1, this->getStateForIP(ip + 1));
}

void TuringCompiler::genAdd16(size_t ip, const Instr& instr) {
    this->genAdd(this->getStateForIP(ip), 2, this->getStateForIP(ip + 1));
}

void TuringCompiler::genAdd32(size_t ip, const Instr& instr) {
    this->genAdd(this->getStateForIP(ip), 4, this->getStateForIP(ip + 1));
}

void TuringCompiler::genSub8(size_t ip, const Instr& instr) {
    this->genSub(this->getStateForIP(ip), 1, this->getStateForIP(ip + 1));
}

void TuringCompiler::genSub16(size_t ip, const Instr& instr) {
    this->genSub(this->getStateForIP(ip), 2, this->getStateForIP(ip + 1));
}

void TuringCompiler::genSub32(size_t ip, const Instr& instr) {
    this->genSub(this->getStateForIP(ip), 4, this->getStateForIP(ip + 1));
}

void TuringCompiler::genAnd8(size_t ip, const Instr& instr) {
    this->genAnd(this->getStateForIP(ip), 1, this->getStateForIP(ip + 1));
}

void TuringCompiler::genAnd16(size_t ip, const Instr& instr) {
    this->genAnd(this->getStateForIP(ip), 2, this->getStateForIP(ip + 1));
}

void TuringCompiler::genAnd32(size_t ip, const Instr& instr) {
    this->genAnd(this->getStateForIP(ip), 4, this->getStateForIP(ip + 1));
}

void TuringCompiler::genOr8(size_t ip, const Instr& instr) {
    this->genOr(this->getStateForIP(ip), 1, this->getStateForIP(ip + 1));
}

void TuringCompiler::genOr16(size_t ip, const Instr& instr) {
    this->genOr(this->getStateForIP(ip), 2, this->getStateForIP(ip + 1));
}

void TuringCompiler::genOr32(size_t ip, const Instr& instr) {
    this->genOr(this->getStateForIP(ip), 4, this->getStateForIP(ip + 1));
}

void TuringCompiler::genXor8(size_t ip, const Instr& instr) {
    this->genXor(this->getStateForIP(ip), 1, this->getStateForIP(ip + 1));
}

void TuringCompiler::genXor16(size_t ip, const Instr& instr) {
    this->genXor(this->getStateForIP(ip), 2, this->getStateForIP(ip + 1));
}

void TuringCompiler::genXor32(size_t ip, const Instr& instr) {
    this->genXor(this->getStateForIP(ip), 4, this->getStateForIP(ip + 1));
}

void TuringCompiler::genIdxShft(size_t ip, const Instr& instr) {
    size_t current_state = this->getStateForIP(ip);
    size_t final_state = this->getStateForIP(ip+1);
    for(size_t i = 0; i < 4; ++i) {
        size_t next_state = this->addState();
        TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, next_state};
        this->states[current_state].def_transition = move_left;
        current_state = next_state;
    }

    size_t current_states[256];
    for(size_t i = 0; i < 256; ++i) {
        current_states[i] = this->addState();
        size_t output = (i << instr.integer) & 0xFF;
        TuringTransition shift_bottom = {i, output, TuringDirection::RIGHT, current_states[i]};
        this->states[current_state].transitions.push_back(shift_bottom);
    }

    size_t next_states[256];

    auto make_states = [&]() {
        for(size_t i = 0; i < 256; ++i)
            next_states[i] = this->addState();
    };
    auto move_states = [&]() {
        for(size_t i = 0; i < 256; ++i)
            current_states[i] = next_states[i];
    };

    for(size_t i = 1; i < 4; ++i) {
        if(i < 3)
            make_states();
        for(size_t j = 0; j < 256; ++j) {
            for(size_t k = 0; k < 256; ++k) {
                size_t next_state = (i == 3) ? final_state : next_states[k];

                size_t result = ((j << instr.integer) >> 8) & 0xFF;
                result |= (k << instr.integer) & 0xFF;

                TuringTransition write_back = {k, result, TuringDirection::RIGHT, next_state};
                this->states[current_states[j]].transitions.push_back(write_back);
            }
        }
        move_states();
    }
}

void TuringCompiler::genJmp(size_t ip, const Instr& instr) {
    size_t current_state = this->getStateForIP(ip);
    size_t next_state = this->getStateForIP(instr.integer);
    TuringTransition trans = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::STAY, next_state};
    this->states[current_state].def_transition = trans;
}

void TuringCompiler::genJf(size_t ip, const Instr& instr) {
    size_t current_state = this->getStateForIP(ip);
    size_t inter_state = this->addState();

    TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, inter_state};
    this->states[current_state].def_transition = move_left;

    size_t next_state = this->getStateForIP(ip+1);
    size_t jump_state = this->getStateForIP(instr.integer);

    TuringTransition take_jump = {0, 0, TuringDirection::STAY, jump_state};
    TuringTransition no_jump = {TRANS_WILDCARD, 0, TuringDirection::STAY, next_state};

    this->states[inter_state].transitions.push_back(take_jump);
    this->states[inter_state].def_transition = no_jump;
}

void TuringCompiler::genJt(size_t ip, const Instr& instr) {
    size_t current_state = this->getStateForIP(ip);
    size_t inter_state = this->addState();

    TuringTransition move_left = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, inter_state};
    this->states[current_state].def_transition = move_left;

    size_t next_state = this->getStateForIP(ip+1);
    size_t jump_state = this->getStateForIP(instr.integer);

    TuringTransition take_jump = {TRANS_WILDCARD, 0, TuringDirection::STAY, jump_state};
    TuringTransition no_jump = {0, 0, TuringDirection::STAY, next_state};

    this->states[inter_state].transitions.push_back(no_jump);
    this->states[inter_state].def_transition = take_jump;
}

void TuringCompiler::genCall(size_t ip, const Instr& instr) {
    size_t current_state = this->getStateForIP(ip);
    uint16_t call_ret_id = this->jump_idx_map[ip+1];

    for(size_t i = 0; i < 2; ++i) {
        size_t final_state = (i == 1) ? this->getStateForIP(instr.integer) : this->addState();

        size_t next_state = this->addState();
        TuringTransition write_temp = {TRANS_WILDCARD, TAPE_TEMP1, TuringDirection::LEFT, next_state};
        this->states[current_state].def_transition = write_temp;
        current_state = next_state;

        uint8_t target_byte = (call_ret_id >> (i * 8)) & 0xFF;
        next_state = this->addState();

        TuringTransition find_ap = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::LEFT, current_state};
        this->states[current_state].def_transition = find_ap;
        find_ap = {TAPE_AP, target_byte, TuringDirection::RIGHT, next_state};
        this->states[current_state].transitions.push_back(find_ap);

        size_t temp_states[256];
        for(size_t j = 0; j < 256; ++j) {
            temp_states[j] = this->addState();

            TuringTransition write_ap = {j, TAPE_AP, TuringDirection::RIGHT, temp_states[j]};
            this->states[next_state].transitions.push_back(write_ap);
        }
        TuringTransition end_trans = {TAPE_TEMP1, TAPE_AP, TuringDirection::RIGHT, final_state};
        this->states[next_state].transitions.push_back(end_trans);

        for(size_t j = 0; j < 256; ++j) {
            for(size_t k = 0; k < 256; ++k) {
                TuringTransition write_back = {k, j, TuringDirection::RIGHT, temp_states[k]};
                this->states[temp_states[j]].transitions.push_back(write_back);
            }

            end_trans.output = j;
            this->states[temp_states[j]].transitions.push_back(end_trans);
        }

        current_state = final_state;
    }
}

void TuringCompiler::genRet(size_t ip, const Instr& instr) {
    size_t current_state = this->getStateForIP(ip);
    size_t next_state = this->addState();

    TuringTransition remove_to_ap = {TRANS_WILDCARD, 0, TuringDirection::LEFT, current_state};
    this->states[current_state].def_transition = remove_to_ap;
    TuringTransition ap_found = {TAPE_AP, 0, TuringDirection::LEFT, next_state};
    this->states[current_state].transitions.push_back(ap_found);

    current_state = next_state;

    if(this->jump_target_ips.size() > 0) {
        size_t entry_points = this->jump_target_ips.size() - 1;
        size_t upper_byte = (entry_points >> 8) & 0xFF;
        size_t lower_byte = entry_points & 0xFF;

        for(size_t i = 0; i <= upper_byte; ++i) {
            next_state = this->addState();
            TuringTransition func_ptr_1 = {i, 0, TuringDirection::LEFT, next_state};
            this->states[current_state].transitions.push_back(func_ptr_1);

            size_t lower_byte_range = i == upper_byte ? lower_byte : 255;

            for(size_t j = 0; j <= lower_byte_range; ++j) {
                size_t call_ip = this->jump_target_ips[(i << 8) | j];
                size_t call_state = this->getStateForIP(call_ip);
                TuringTransition perform_ret = {j, 0, TuringDirection::STAY, call_state};
                this->states[next_state].transitions.push_back(perform_ret);
            }
        }
    }
}

void TuringCompiler::genSetRet8(size_t ip, const Instr& instr) {
    this->genSetRet(this->getStateForIP(ip), 1, this->getStateForIP(ip+1));
}

void TuringCompiler::genSetRet16(size_t ip, const Instr& instr) {
    this->genSetRet(this->getStateForIP(ip), 2, this->getStateForIP(ip+1));
}

void TuringCompiler::genSetRet32(size_t ip, const Instr& instr) {
    this->genSetRet(this->getStateForIP(ip), 4, this->getStateForIP(ip+1));
}

void TuringCompiler::genAccept(size_t ip, const Instr& instr) {
    size_t current_state = this->getStateForIP(ip);
    TuringTransition trans = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::STAY, 0};
    this->states[current_state].def_transition = trans;
}

void TuringCompiler::genReject(size_t ip, const Instr& instr) {
    size_t current_state = this->getStateForIP(ip);
    TuringTransition trans = {TRANS_WILDCARD, TRANS_WILDCARD, TuringDirection::STAY, 1};
    this->states[current_state].def_transition = trans;
}

void TuringCompiler::compileInstr(size_t ip) {
    const Instr& instr = this->instr[ip];

    (this->*(TuringCompiler::GENERATOR_CALLBACKS[static_cast<size_t>(instr.opcode)]))(ip, instr);
}

TuringMachine TuringCompiler::compile() {
    TuringMachine machine;
    machine.accept_state = 0;
    machine.reject_state = 1;

    size_t start_state = this->addState();
    TuringTransition push_global_pointer = {TRANS_WILDCARD, TAPE_GP, TuringDirection::RIGHT, this->getStateForIP(0)};
    this->states[start_state].def_transition = push_global_pointer;
    machine.start_state = start_state;

    for(size_t i = 0; i < this->num_instr; ++i) {
        this->compileInstr(i);
    }

    machine.states = this->states;
    return machine;
}