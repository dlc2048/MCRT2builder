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
 * @file    mcutil/mclog/logger.cpp
 * @brief   RT2 log system
 * @author  CM Lee
 * @date    05/23/2023
 */

 #include <iomanip>
 #include <iostream>
 #include <ctime>

#include "logger.hpp"


namespace mclog {


    McrtLogger::McrtLogger() :
        _max_length(80),
        _log_level(LEVEL::WARN)
    {
        this->_header.insert({ LEVEL::OFF,   "" });
        this->_header.insert({ LEVEL::FATAL, " [FATAL] " });
        this->_header.insert({ LEVEL::WARN,  " [WARNING] " });
        this->_header.insert({ LEVEL::PRINT, "" });
        this->_header.insert({ LEVEL::INFO,  " [INFO] " });
        this->_header.insert({ LEVEL::LINE,  " [LINE] " });
        this->_header.insert({ LEVEL::DEBUG, " [DEBUG] " });
    };


    McrtLogger::~McrtLogger() {
        this->close();
    }


    void McrtLogger::_printCout(std::stringstream& ss) {
        std::string token;
        while (std::getline(ss, token, '\n')) {
            std::cout << token << std::endl;
        }
    }


    void McrtLogger::_printFile(std::stringstream& ss) {
        std::string token;
        while (std::getline(ss, token, '\n')) {
            this->_file << token << std::endl;
        }
    }


    void McrtLogger::setMaxLength(size_t length) {
        this->_max_length = length;
    }


    void McrtLogger::setLevel(LEVEL level) {
        this->_log_level = level;
    }


    bool McrtLogger::open(const std::string& file_name) {
        this->_file = std::ofstream(file_name);
        return this->_file.is_open();
    }


    void McrtLogger::close() {
        if (this->_file.is_open())
            this->_file.close();
    }


    void McrtLogger::print(LEVEL level, std::stringstream& ss, bool force_newline) {
        bool is_empty_line = true;
        const std::string& header = this->_header.find(level)->second;
        size_t header_size = header.size();
        std::stringstream ns;

        // generate empty spaceline
        std::string space("");
        {
            std::stringstream space_ss;
            for (size_t i = 0; i < header_size; ++i) {
                space_ss << ' ';
            }
            space = space_ss.str();
        }

        // calculate message stride
        if (this->_max_length <= header_size)
            throw std::length_error("member variable 'McrtLogger::_max_length' must be larger than header size");

        if (force_newline) {
            size_t stride = this->_max_length - header_size;
            std::string message;
            while (std::getline(ss, message, '\n')) {
                is_empty_line = false;
                for (size_t i = 0; i < message.size(); i += stride) {
                    if (!i) ns << header;
                    else    ns << space;
                    ns << message.substr(i, stride);
                    ns << std::endl;
                }
            }
        }
        else {
            std::string message;
            while (std::getline(ss, message, '\n')) {
                is_empty_line = false;
                ns << message << std::endl;
            }
        }

        if (is_empty_line)
            ns << std::endl << std::endl;

        if (level <= this->_log_level) {
            if (this->_file.is_open())
                this->_printFile(ns);
            else
                this->_printCout(ns);
        }

        if (level == LEVEL::FATAL)
            throw McrtLoggerException("Program terminated by bad input");

    }


    void McrtLogger::time() {
        std::time_t time = std::time({});
        char time_string[std::size("yyyy-mm-dd hh:mm:ss")];
        std::strftime(std::data(time_string), std::size(time_string), "%F %T", std::localtime(&time));
        std::stringstream ss;
        ss << " [" << time_string << "] " << std::endl;
        if (this->_file.is_open())
            this->_printFile(ss);
        else
            this->_printCout(ss);
    }


    McrtLogStream::McrtLogStream(LEVEL level, const std::string& prefix) 
        : _level(level) {
        this->_ss << prefix;
    }


    McrtLogStream::~McrtLogStream() {
        McrtLogger::getInstance().print(this->_level, this->_ss);
    }


    bool setLogger(const std::string& file_name) {
        return McrtLogger::getInstance().open(file_name);
    }


    bool setLogger() {
        McrtLogger::getInstance().close();
        return true;
    }


    void setMaxLength(size_t length) {
        McrtLogger::getInstance().setMaxLength(length);
    }


    void setLevel(LEVEL level) {
        McrtLogger::getInstance().setLevel(level);
    }


    McrtLogStream fatal() {
        return McrtLogStream(LEVEL::FATAL);
    }


    void fatalTypeCasting(
        const std::string& field,
        const std::string& value
    ) {
        fatal() << "Type casting error detected in '"
                << field << "', its value '"
                << value << "' has strange format";
    }


    void fatalOutOfRangeCeil(
        const std::string& field,
        const std::string& value,
        const std::string& ceil
    ) {
        fatal() << "Value " << value 
                << " must be equal to or less than "
                << ceil << " in field '" << field << "'";
    }


    void fatalOutOfRangeFloor(
        const std::string& field,
        const std::string& value,
        const std::string& floor
    ) {
        fatal() << "Value " << value
                << " must be equal to or larger than "
                << floor << " in field '" << field << "'";
    }


    void fatalFieldRequired(const std::string& field) {
        fatal() << "Field '" << field << "' is necessary";
    }


    void fatalValueSize(
        const std::string& field,
        size_t required_size,
        size_t entered_size
    ) {
        fatal() << "Field '" << field << "' requires "
                << required_size << " values, but "
                << entered_size << " values are entered";
    }


    void fatalInvalidNameFormat(const std::string& name) {
        fatal() << "Name '" << name << "' has invalid format";
    }


    void fatalNameAlreadyExist(const std::string& name) {
        fatal() << "Name '" << name << "' is already defined";
    }


    void fatalNameNotExist(const std::string& name) {
        fatal() << "Name '" << name << "' is not defined";
    }


    void fatalFileNotExist(const std::string& name) {
        fatal() << "Fail to open file '" << name << "'";
    }


    void fatalNecessary(const std::string& key) {
        fatal() << "At least one '" << key << "'"
                << " card is necessary";
    }


    void fatalInsufficient(const std::string& key) {
        fatal() << "Card '" << key
                << "' has insufficient parameters";
    }


    void fatalInvalidOption(const std::string& key, const std::vector<std::string>& options) {
        McrtLogStream fatal_stream(LEVEL::FATAL);
        fatal_stream << "'" << key << "' must be ";
        for (size_t i = 0; i < options.size(); ++i) {
            fatal_stream << "'" << options[i] << "'";
            if (i < options.size() - 2)
                fatal_stream << ", ";
            else if (i == options.size() - 2) {
                if (options.size() == 2)
                    fatal_stream << " or ";
                else
                    fatal_stream << ", or ";
            }
        }
    }


    McrtLogStream warning() {
        return McrtLogStream(LEVEL::WARN);
    }


    void warningUseDefaultField(const std::string& field, const std::string& value) {
        warning() << "Field '" << field << "' is not found. "
                  << "Use default value (" << value << ")";
    }


    McrtLogStream line(size_t idx) {
        std::stringstream ss;
        ss << std::setw(6) << idx << " ";
        return McrtLogStream(LEVEL::LINE, ss.str());
    }


    McrtLogStream info() {
        return McrtLogStream(LEVEL::INFO);
    }


    McrtLogStream print() {
        return McrtLogStream(LEVEL::PRINT);
    }


    void printName(const std::string& name, size_t max_length) {
        print() << " [ " << std::setw(max_length) << mcutil::truncate(name, max_length) << " ]";
    }


    void printVar(const std::string& var, const std::string& value, const std::string& unit) {
        std::stringstream ss;
        size_t sh;
        ss << "  " << var << " ";
        sh = ss.str().size();
        ss << std::setw(60 - sh) << value;
        if (!unit.empty())
            ss << " (" << unit << ")";
        print() << ss.str();
    }


    void printVar(const std::string& var, double value, const std::string& unit) {
        std::stringstream ss;
        ss << value;
        printVar(var, ss.str(), unit);
    }


    void printVar(const std::string& var, size_t value, const std::string& unit) {
        std::stringstream ss;
        ss << value;
        printVar(var, ss.str(), unit);
    }


    void printVar(const std::string& var, int    value, const std::string& unit) {
        std::stringstream ss;
        ss << value;
        printVar(var, ss.str(), unit);
    }


    void printVar(const std::string& var, float  value, const std::string& unit) {
        std::stringstream ss;
        ss << value;
        printVar(var, ss.str(), unit);
    }


    void printSyntax(size_t indent, const std::string& divider, const std::string& field, const std::string& datatype, const std::string& desc) {
        McrtLogger& logger  = McrtLogger::getInstance();
        size_t max_length   = logger.maxLength();
        size_t field_length = INDENT_DEFAULT_DESC + indent + divider.size() + 1;

        if (field_length > max_length)
            mclog::fatal() << "member variable 'McrtLogger::_max_length' must be larger than field size";

        std::string space_indent = std::string(field_length, ' ');

        std::stringstream form_ss;
        std::stringstream desc_ss(desc);

        form_ss << std::string(INDENT_DEFAULT_DESC , ' ') << field << std::string(indent - field.size(), ' ') << divider << " ";
        size_t pos = field_length;

        bool is_first = true;
        while (true) {
            std::string token;
            if (is_first && !datatype.empty()) {
                token = "(" + datatype + ")";
            }
            else {
                desc_ss >> token;
            }
            is_first = false;

            if (pos + token.size() < max_length) {  // accpeted
                form_ss << token << " ";
                pos += token.size() + 1;
            }
            else {
                form_ss << std::endl;  // newline
                form_ss << space_indent << token << " ";
                pos = field_length + token.size();
            }

            if (desc_ss.eof()) {
                form_ss << std::endl;
                break;
            }
        }

        logger.print(LEVEL::PRINT, form_ss, false);
    }


    McrtLogStream debug() {
        return McrtLogStream(LEVEL::DEBUG);
    }


    void time() {
        McrtLogger::getInstance().time();
    }


    FormattedTable::FormattedTable(const std::vector<size_t>& length, size_t offset, FORMAT_TYPE type, int digit) :
        _len(length), _lpos(0), _offset(offset), _type(type), _digit(digit) {
        this->_ss << std::string(_offset, ' ');
    }


    void FormattedTable::clear() {
        this->_lpos = 0x0u;
        this->_ss   = std::stringstream();
        this->_ss << std::string(_offset, ' ');
    }


    //template <>
    //FormattedTable& operator<< <std::string>(FormattedTable& cls, std::string _val) {
    //    size_t len = cls._len[cls._lpos];
    //    cls._lpos++;
    //    cls._ss << std::setw(len);
    //    cls._ss << _val;
    //    cls._ss << " ";
    //    return cls;
    //}
    

    FormattedTable& FormattedTable::operator<<(const std::string& _val) {
        size_t len = this->_len[this->_lpos];
        this->_lpos++;
        this->_ss << std::setw(len);
        this->_ss << mcutil::truncate(_val, len);
        this->_ss << " ";
        return *this;
    }


    FormattedTable& FormattedTable::operator<<(const char _val[]) {
        return this->operator<<(std::string(_val));
    }


}


namespace mcutil {

    std::string truncate(const std::string& str, size_t size) {
        std::string out;
        size_t str_size = str.size();
        if (str_size > size) {
            out = str.substr(0, size - 2);
            out += "..";
        }
        else
            out = str;
        return out;
    }

}
