//
// Copyright (C) 2025 CM Lee, SJ Ye, Seoul Sational University
//
// Licensed to the Apache Software Foundation(ASF) under one
// or more contributor license agreements.See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// 	"License"); you may not use this file except in compliance
// 	with the License.You may obtain a copy of the License at
// 
// 	http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.See the License for the
// specific language governing permissionsand limitations
// under the License.

/**
 * @file    RT2builder/region.cpp
 * @brief   RT2builder region definitions
 * @author  CM Lee
 * @date    05/23/2023
 */


#include "region.hpp"


namespace mcutil {


    template <>
    ArgumentCard InputCardFactory<geo::Region>::_setCard() {
        ArgumentCard arg_card("REGION");

        arg_card.insert<std::string>("name", 
            "The name of region. The region name must be "
            "unique.",
            1);

        arg_card.insert<std::string>("equation",
            "The list of equation element. It is interpreted "
            "as operator, operand, or decorator.");

        return arg_card;
    }


}


namespace geo {


    EquationElem::EquationElem(BOOLEAN_OPERATOR op_type, const std::string& name)
        : type(op_type), surface_id(-1), operand_name(name) {};


    EquationElem::EquationElem(BOOLEAN_OPERATOR op_type)
        : type(op_type), surface_id(-1), operand_name("") {};


    EquationElem::EquationElem(const std::string& name)
        : type(BOOLEAN_OPERATOR::OPERAND), surface_id(-1), operand_name(name) {};


    std::map<std::string, BOOLEAN_OPERATOR> Region::_OPERATOR = {
        {"@", BOOLEAN_OPERATOR::RECUR},
        {"+", BOOLEAN_OPERATOR::NONE },
        {"-", BOOLEAN_OPERATOR::NOT  },
        {"&", BOOLEAN_OPERATOR::AND  },
        {"|", BOOLEAN_OPERATOR::OR   },
        {"(", BOOLEAN_OPERATOR::BRA  },
        {")", BOOLEAN_OPERATOR::KET  }
    };


    bool Region::_toPostfix() {
        typedef std::list<EquationElem> List;
        std::deque<EquationElem> temp_stack;
        std::list<EquationElem> eq_post;
        for (List::iterator iter = this->_equation.begin(); 
            iter != this->_equation.end(); ++iter) {
            const EquationElem& e = *iter;
            if (e.type == BOOLEAN_OPERATOR::NONE)
                continue;
            else if (e.type == BOOLEAN_OPERATOR::OPERAND)
                eq_post.push_back(e);
            else if (e.type == BOOLEAN_OPERATOR::KET) {
                if (temp_stack.empty()) {
                    return false;
                }
                while (temp_stack.back().type != BOOLEAN_OPERATOR::BRA && temp_stack.size() > 1) {
                    eq_post.push_back(temp_stack.back());
                    temp_stack.pop_back();
                }
                if (temp_stack.back().type != BOOLEAN_OPERATOR::BRA)
                    return false;
                else {
                    temp_stack.pop_back();
                }
            }
            else if (temp_stack.empty())
                temp_stack.push_back(e);
            else if (temp_stack.back().type < e.type || e.type == BOOLEAN_OPERATOR::BRA)
                temp_stack.push_back(e);
            else {
                while (!temp_stack.empty()) {
                    if (temp_stack.back().type > e.type) {
                        eq_post.push_back(temp_stack.back());
                        temp_stack.pop_back();
                    }
                    else
                        break;
                }
                temp_stack.push_back(e);
            }
        }

        while (!temp_stack.empty()) {
            if (temp_stack.back().type == BOOLEAN_OPERATOR::BRA ||
                temp_stack.back().type == BOOLEAN_OPERATOR::KET) {
                return false;
            }
            eq_post.push_back(temp_stack.back());
            temp_stack.pop_back();
        }
        this->_equation.swap(eq_post);
        return true;
    }


    Region::Region(mcutil::ArgInput& args) :
        _surface_id_end(0), _good(true) {
        typedef std::map<std::string, BOOLEAN_OPERATOR> Op;

        // check name is valid
        std::string name = args["name"].cast<std::string>()[0];
        if (mcutil::isHasSpecialSymbol(name) ||
            mcutil::isHasSpecialSymbol(name))
            mclog::fatalInvalidNameFormat(name);
        this->_name = name;

        // check equation is valid
        std::vector<std::string> equation = args["equation"].cast<std::string>();
        if (equation.size() < 1)
            mclog::fatalValueSize("equation", 1, equation.size());

        // join and split
        std::string             equation_packed  = mcutil::join(equation, "");
        std::deque<std::string> equation_splited = mcutil::split(equation_packed, "@+-&|()");

        // read infix equation
        for (size_t i = 0; i < equation_splited.size(); ++i) {
            const std::string& e = equation_splited[i];
            Op::iterator iter = Region::_OPERATOR.find(e);
            if (iter != Region::_OPERATOR.end())
                this->_equation.push_back(EquationElem(iter->second));
            else
                this->_equation.push_back(EquationElem(e));
        }
        // convert infix to postfix
        if (!this->_toPostfix()) {
            mclog::fatal() << "Boolean equation of REGION '"
                           << this->_name << "' has problem";
        }
    }


    Region::Region(const std::string& name) :
        _surface_id_end(0), _good(true), _name(name) {}


    const std::string& Region::name() const {
        return this->_name;
    }


    const std::list<EquationElem>& Region::equation() const {
        return this->_equation;
    }


    bool Region::unfold(const std::vector<Region>& others) {
        typedef std::list<EquationElem> List;
        for (List::iterator iter = this->_equation.begin(), prev = this->_equation.end(); 
            iter != this->_equation.end(); prev = iter, ++iter) {
            if (iter->type == BOOLEAN_OPERATOR::RECUR) {
                if ((prev == this->_equation.end()) || 
                    (prev->type != BOOLEAN_OPERATOR::OPERAND)) {
                    mclog::fatal() << "Boolean equation of REGION '"
                                   << this->_name << "' has problem";
                }

                const std::string& target = prev->operand_name;
                size_t j = 0;
                for (; j < others.size(); ++j) {
                    if (others[j].name() == target)
                        break;
                }
                if (j == others.size())
                    mclog::fatalNameNotExist(target);

                const std::list<EquationElem>& equation = others[j].equation();
                this->_equation.erase(prev);
                iter = this->_equation.erase(iter);
                this->_equation.insert(iter, equation.begin(), equation.end());
            }
        }
        return true;
    }


    bool Region::mark(const std::vector<std::string>& surface_namelist) {
        typedef std::list<EquationElem> List;
        for (List::iterator iter = this->_equation.begin();
            iter != this->_equation.end(); ++iter) {
            if (iter->type == BOOLEAN_OPERATOR::OPERAND) {
                const std::string& target = iter->operand_name;
                size_t j = 0;
                for (; j < surface_namelist.size(); ++j) {
                    if (surface_namelist[j] == target)
                        break;
                }
                if (j == surface_namelist.size())
                    mclog::fatalNameNotExist(target);
                iter->surface_id = (int)j;
                this->_surface_id_end = std::max(this->_surface_id_end, j);
            }
        }
        return true;
    }


    size_t Region::maximumSurfaceIndex() const {
        return this->_surface_id_end;
    }


    bool Region::good() const {
        if (!this->_good) return false;
        bool is_valid = true;
        std::vector<bool> tester(this->_surface_id_end + 1, false);
        std::deque<bool> state;
        bool temp, s1, s2;

        // empty equation (voxel)
        if (this->_equation.empty())
            return true;

        // test
        for (std::list<EquationElem>::const_iterator iter = this->_equation.begin();
            iter != this->_equation.end() && is_valid; ++iter) {
            switch (iter->type) {
            case (BOOLEAN_OPERATOR::OPERAND):
                state.push_back(tester[iter->surface_id]);
                break;
            case (BOOLEAN_OPERATOR::BRA):
            case (BOOLEAN_OPERATOR::KET):
                is_valid = false;
                break;
            case (BOOLEAN_OPERATOR::AND):
                if (state.size() < 2) is_valid = false;
                else {
                    s1 = state.back();
                    state.pop_back();
                    s2 = state.back();
                    state.pop_back();
                    temp = s1 && s2;
                    state.push_back(temp);
                }
                break;
            case (BOOLEAN_OPERATOR::NOT):
                if (state.size() < 1) is_valid = false;
                else {
                    s1 = state.back();
                    state.pop_back();
                    s1 ^= true;
                    state.push_back(s1);
                }
                break;
            case (BOOLEAN_OPERATOR::OR):
                if (state.size() < 2) is_valid = false;
                else {
                    s1 = state.back();
                    state.pop_back();
                    s2 = state.back();
                    state.pop_back();
                    temp = s1 || s2;
                    state.push_back(temp);
                }
                break;
            }
        }
        if (state.size() != 1) is_valid = false;
        return is_valid;
    }


    bool Region::inside(const bool* state) const {
        std::deque<bool> stack;
        bool temp, s1, s2;

        // boolean series
        for (std::list<EquationElem>::const_iterator iter = this->_equation.begin();
            iter != this->_equation.end(); ++iter) {
            switch (iter->type) {
            case (BOOLEAN_OPERATOR::OPERAND):
                stack.push_back(state[iter->surface_id]);
                break;
            case (BOOLEAN_OPERATOR::AND):
                s1 = stack.back();
                stack.pop_back();
                s2 = stack.back();
                stack.pop_back();
                temp = s1 && s2;
                stack.push_back(temp);
                break;
            case (BOOLEAN_OPERATOR::NOT):
                s1 = stack.back();
                stack.pop_back();
                s1 ^= true;
                stack.push_back(s1);
                break;
            case (BOOLEAN_OPERATOR::OR):
                s1 = stack.back();
                stack.pop_back();
                s2 = stack.back();
                stack.pop_back();
                temp = s1 || s2;
                stack.push_back(temp);
                break;
            }
        }
        return stack.front();
    }


    bool Region::related(int surface_idx) const {
        for (std::list<EquationElem>::const_iterator iter = this->_equation.begin();
            iter != this->_equation.end(); ++iter) {
            if (iter->type == BOOLEAN_OPERATOR::OPERAND)
                if (iter->surface_id == surface_idx)
                    return true;
        }
        return false;
    }


    std::vector<size_t> Region::correspondingIndex() const {
        std::vector<size_t> corr;
        for (std::list<EquationElem>::const_iterator iter = this->_equation.begin();
            iter != this->_equation.end(); ++iter) {
            if (iter->type == BOOLEAN_OPERATOR::OPERAND)
                corr.push_back(iter->surface_id);
        }
        return corr;
    }

}
