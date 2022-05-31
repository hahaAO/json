//
// Created by 12552 on 2022/5/29.
//
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include "json.h"

#define   CURRENT_CHAR json_string[index]


namespace vgd {

    inline void throw_unexpected_char(const std::string &parse_func_name, const char &current_char) {
        throw Json_exception(std::string("[") + parse_func_name + "] 意外的字符 \'" + current_char + '\'');
    }

    inline void throw_state_error(const std::string &parse_func_name, const char &current_char) {
        throw Json_exception(std::string("[") + parse_func_name + "] 状态与等待的字符 \'" + current_char + "\'不匹配");
    }

    inline unsigned int HexToBCD(char c) {
        switch (c) {
            case '0':
                return 0;
            case '1':
                return 1;
            case '2':
                return 2;
            case '3':
                return 3;
            case '4':
                return 4;
            case '5':
                return 5;
            case '6':
                return 6;
            case '7':
                return 7;
            case '8':
                return 8;
            case '9':
                return 9;
            case 'a':
            case 'A':
                return 10;
            case 'b':
            case 'B':
                return 11;
            case 'c':
            case 'C':
                return 12;
            case 'd':
            case 'D':
                return 13;
            case 'e':
            case 'E':
                return 14;
            case 'f':
            case 'F':
                return 15;
        }
        throw_unexpected_char(__func__, c);
        return EXIT_SUCCESS;// 编译器居然识别不出上面的语句会throw，认为该函数无返回，只能加个返回以取消警告
    }


    std::string json::parse_string(const std::string &json_string, size_t &index) {
        assert(json_string[index] == '\"');
        ++index;

        auto end = json_string.size();
        std::stringstream res_buf;
        while (index < end) {
            switch (CURRENT_CHAR) {
                case '\\': {
                    ++index;
                    if (index >= end) {
                        throw Json_exception(R"([parse_string] 没找到 '"' )");
                    }
                    switch (CURRENT_CHAR) {
                        case '\"':
                        case '\\':
                        case '/': {
                            res_buf << '\\' << CURRENT_CHAR;
                            ++index;
                            break;
                        }
                        case 'b': {
                            res_buf << '\\' << 'b';
                            ++index;
                            break;
                        }
                        case 'f': {
                            res_buf << '\\' << 'f';
                            ++index;
                            break;
                        }
                        case 'n': {
                            res_buf << '\\' << 'n';
                            ++index;
                            break;
                        }
                        case 'r': {
                            res_buf << '\\' << 'r';
                            ++index;
                            break;
                        }
                        case 't': {
                            res_buf << '\\' << 't';
                            ++index;
                            break;
                        }
                        case 'u': {
                            ++index;
                            if (end - index < 4) {
                                throw Json_exception(R"([parse_string] 4 hex digits 长度不足)");
                            }
                            auto hex_4 = json_string[index++];
                            auto hex_3 = json_string[index++];
                            auto hex_2 = json_string[index++];
                            auto hex_1 = json_string[index++];

                            HexToBCD(hex_4);
                            HexToBCD(hex_3);
                            HexToBCD(hex_2);
                            HexToBCD(hex_1);

                            res_buf << '\\' << 'u' << hex_4 << hex_3 << hex_2 << hex_1;// TODO：unicode编码转换
                            break;
                        }
                        default:
                            throw_unexpected_char(__func__, CURRENT_CHAR);
                    }
                    break;
                }
                case '\"': {
                    ++index;
                    std::string res;
                    res = res_buf.str();
                    return res;
                }
                default:
                    res_buf << json_string[index++];
                    break;
            }
        }
        throw Json_exception(R"([parse_string] 没找到 '"' )");
    }

    double json::parse_number(const std::string &json_string, size_t &index) {
        auto end = json_string.size();
        std::stringstream number_string;
        bool is_float = false;
        bool is_exponential = false;

        double res;

        while (index < end) {
            switch (CURRENT_CHAR) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    number_string << json_string[index++];
                    break;
                }
                case '.': {
                    if (is_float || is_exponential) {
                        throw_unexpected_char(__func__, CURRENT_CHAR);
                    } else {
                        is_float = true;
                        number_string << json_string[index++];
                        break;
                    }
                }
                case 'e':
                case 'E':
                    if (is_exponential) {
                        throw_unexpected_char(__func__, CURRENT_CHAR);
                    } else {
                        number_string << json_string[index++];
                        if (index == end - 1 || (CURRENT_CHAR != '+' && CURRENT_CHAR != '-')) {
                            throw Json_exception("[parse_number] 指数后没有正负号");
                        } else {
                            is_exponential = true;
                            number_string << json_string[index++];
                            break;
                        }
                    }
                case ']':
                case '}':
                case ',':
                case ' ': {
                    number_string >> res;
                    return res;
                }
                default:
                    throw_unexpected_char(__func__, CURRENT_CHAR);
            }
        }

        number_string >> res;
        return res;
    }

    void json::parse_null(const std::string &json_string, size_t &index) {
        auto end = json_string.size();
        if (end - index < 4) {
            throw Json_exception(R"([parse_null] 长度不足)");
        }
        if (
                'n' == json_string[index++] &&
                'u' == json_string[index++] &&
                'l' == json_string[index++] &&
                'l' == json_string[index++]
                ) {
            return;
        } else {
            --index;
            throw_unexpected_char(__func__, CURRENT_CHAR);
        }
    }

    void json::parse_true(const std::string &json_string, size_t &index) {
        auto end = json_string.size();
        if (end - index < 4) {
            throw Json_exception(R"([parse_null] 长度不足)");
        }
        if (
                't' == json_string[index++] &&
                'r' == json_string[index++] &&
                'u' == json_string[index++] &&
                'e' == json_string[index++]
                ) {
            return;
        } else {
            --index;
            throw_unexpected_char(__func__, CURRENT_CHAR);
        }
    }

    void json::parse_false(const std::string &json_string, size_t &index) {
        auto end = json_string.size();
        if (end - index < 4) {
            throw Json_exception(R"([parse_null] 长度不足)");
        }
        if (
                'f' == json_string[index++] &&
                'a' == json_string[index++] &&
                'l' == json_string[index++] &&
                's' == json_string[index++] &&
                'e' == json_string[index++]
                ) {
            return;
        } else {
            --index;
            throw_unexpected_char(__func__, CURRENT_CHAR);
        }
    }

    std::vector<json> json::parse_array(const std::string &json_string, size_t &index) {
        enum class state {
            waiting_value,// 等待一个value
            waiting_others// 等待一个逗号或右括号
        };
        auto end = json_string.size();

        assert(json_string[index] == '[');
        if (end - index < 2) {
            throw Json_exception(R"([parse_array] 长度不足)");
        }

        ++index;
        std::vector<json> res;
        if (CURRENT_CHAR == ']') {
            ++index;
            return res;
        }

        auto state = state::waiting_value;

        while (index < end) {
            switch (CURRENT_CHAR) {
                case ' ': {
                    ++index;
                    break;
                }
                case '{': {
                    if (state == state::waiting_value) {
                        auto object = parse_object(json_string, index);
                        json json_object;
                        json_object.my_type_ = json_element_type::object;
                        json_object.object_ = object;
                        res.emplace_back(std::move(json_object));
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case '[': {
                    if (state == state::waiting_value) {
                        auto array = parse_array(json_string, index);
                        json json_array;
                        json_array.my_type_ = json_element_type::array;
                        json_array.array_ = array;
                        res.emplace_back(std::move(json_array));
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case '\"': {
                    if (state == state::waiting_value) {
                        auto str = parse_string(json_string, index);
                        json json_string;
                        json_string.my_type_ = json_element_type::string;
                        json_string.string_ = str;
                        res.emplace_back(json_string);
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    if (state == state::waiting_value) {
                        auto number = parse_number(json_string, index);
                        json json_number;
                        json_number.my_type_ = json_element_type::number;
                        json_number.number_ = number;
                        res.emplace_back(std::move(json_number));
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case 't': {
                    if (state == state::waiting_value) {
                        parse_true(json_string, index);
                        json json_true;
                        json_true.my_type_ = json_element_type::bool_true;
                        res.emplace_back(std::move(json_true));
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case 'f': {
                    if (state == state::waiting_value) {
                        parse_false(json_string, index);
                        json json_false;
                        json_false.my_type_ = json_element_type::bool_false;
                        res.emplace_back(std::move(json_false));
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case 'n': {
                    if (state == state::waiting_value) {
                        parse_null(json_string, index);
                        json json_null;
                        json_null.my_type_ = json_element_type::null;
                        res.emplace_back(std::move(json_null));
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case ',': {
                    if (state == state::waiting_others) {
                        ++index;
                        state = state::waiting_value;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case ']': {
                    if (state == state::waiting_others) {
                        ++index;
                        return res;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                default:
                    throw_unexpected_char(__func__, CURRENT_CHAR);
            }
        }

        throw Json_exception("[parse_array]没找到']'");
    }

    std::map<std::string, json> json::parse_object(const std::string &json_string, size_t &index) {
        enum class state {
            waiting_key,// 等待一个key
            waiting_value,// 等待一个value
            waiting_colon,// 等待一个冒号
            waiting_others// 等待一个逗号或终止符
        };
        std::string key_string;// 当前key的string
        auto end = json_string.size();

        assert(json_string[index] == '{');
        if (end - index < 2) {
            throw Json_exception(R"([parse_object] 长度不足)");
        }

        ++index;
        std::map<std::string, json> res;
        if (CURRENT_CHAR == '}') {
            ++index;
            return res;
        }

        auto state = state::waiting_key;
        while (index < end) {
            switch (CURRENT_CHAR) {
                case ' ': {
                    ++index;
                    break;
                }
                case '{': {
                    if (state == state::waiting_value) {
                        auto object = parse_object(json_string, index);
                        json json_object;
                        json_object.my_type_ = json_element_type::object;
                        json_object.object_ = object;
                        res.insert({key_string, std::move(json_object)});
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case '[': {
                    if (state == state::waiting_value) {
                        auto array = parse_array(json_string, index);
                        json json_array;
                        json_array.my_type_ = json_element_type::array;
                        json_array.array_ = array;
                        res.insert({key_string, std::move(json_array)});
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case '\"': {
                    if (state == state::waiting_key) {
                        auto str = parse_string(json_string, index);
                        if (res.find(str) != res.end()) {
                            throw Json_exception("[parse_object] 存在重复键");
                        }
                        key_string = str;
                        state = state::waiting_colon;
                        break;
                    } else if (state == state::waiting_value) {
                        auto str = parse_string(json_string, index);
                        json json_value;
                        json_value.my_type_ = json_element_type::string;
                        json_value.string_ = str;
                        res.insert({key_string, std::move(json_value)});
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    if (state == state::waiting_value) {
                        auto number = parse_number(json_string, index);
                        json json_number;
                        json_number.my_type_ = json_element_type::number;
                        json_number.number_ = number;
                        res.insert({key_string, std::move(json_number)});
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case 't': {
                    if (state == state::waiting_value) {
                        parse_true(json_string, index);
                        json json_true;
                        json_true.my_type_ = json_element_type::bool_true;
                        res.insert({key_string, std::move(json_true)});
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case 'f': {
                    if (state == state::waiting_value) {
                        parse_false(json_string, index);
                        json json_false;
                        json_false.my_type_ = json_element_type::bool_false;
                        res.insert({key_string, std::move(json_false)});
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case 'n': {
                    if (state == state::waiting_value) {
                        parse_null(json_string, index);
                        json json_null;
                        json_null.my_type_ = json_element_type::null;
                        res.insert({key_string, std::move(json_null)});
                        state = state::waiting_others;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case ':': {
                    if (state == state::waiting_colon) {
                        ++index;
                        state = state::waiting_value;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case ',': {
                    if (state == state::waiting_others) {
                        ++index;
                        state = state::waiting_key;
                        break;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                case '}': {
                    if (state == state::waiting_others) {
                        ++index;
                        return res;
                    } else {
                        throw_state_error(__func__, CURRENT_CHAR);
                    }
                }
                default:
                    throw_unexpected_char(__func__, CURRENT_CHAR);
            }
        }
        throw Json_exception("[parse_object]没找到'}'");
    }

    json json::parse_json(const std::string &json_string) {
        size_t index = 0;
        json res;
        auto end = json_string.size();

        while (index < end) {
            switch (CURRENT_CHAR) {
                case ' ': {
                    ++index;
                    break;
                }
                case '{': {
                    res.my_type_ = json_element_type::object;
                    res.object_ = parse_object(json_string, index);
                    return res;
                }
                case '[': {
                    res.my_type_ = json_element_type::array;
                    res.array_ = parse_array(json_string, index);
                    return res;
                }
                case '\"': {
                    res.my_type_ = json_element_type::string;
                    res.string_ = parse_string(json_string, index);
                    return res;
                }
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    res.my_type_ = json_element_type::number;
                    res.number_ = parse_number(json_string, index);
                    return res;
                }
                case 't': {
                    parse_true(json_string, index);
                    res.my_type_ = json_element_type::bool_true;
                    return res;
                }
                case 'f': {
                    parse_false(json_string, index);
                    res.my_type_ = json_element_type::bool_false;
                    return res;

                }
                case 'n': {
                    parse_null(json_string, index);
                    res.my_type_ = json_element_type::null;
                    return res;
                }
                default:
                    throw_unexpected_char(__func__, CURRENT_CHAR);
            }
        }
        throw Json_exception("[parse_json] 没有匹配的json对象'}'");
    }

    std::string json::to_string(const json &current_json) {
        std::stringstream json_string_stream;

        value_in_stream(current_json, json_string_stream);

        return json_string_stream.str();
    }

    void json::value_in_stream(const json &current_json, std::stringstream &json_string_stream) {
        switch (current_json.my_type_) {
            case json_element_type::object:
                object_in_stream(current_json, json_string_stream);
                break;
            case json_element_type::array:
                array_in_stream(current_json, json_string_stream);
                break;
            case json_element_type::string:
                json_string_stream << '"' << current_json.string_ << '"';
                break;
            case json_element_type::number:
                json_string_stream << current_json.number_;
                break;
            case json_element_type::bool_true:
                json_string_stream << "true";
                break;
            case json_element_type::bool_false:
                json_string_stream << "false";
                break;
            case json_element_type::null:
                json_string_stream << "null";
                break;
        }
    }

    void json::object_in_stream(const json &current_json, std::stringstream &json_string_stream) {
        assert(current_json.my_type_ == json_element_type::object);
        json_string_stream << '{';
        for (auto iter = current_json.object_.cbegin(); iter != current_json.object_.cend();) {
            json_string_stream << '"' << iter->first << '"' << ':';
            value_in_stream(iter->second, json_string_stream);
            if (++iter != current_json.object_.cend()) {
                json_string_stream << ',';
            }
        }
        json_string_stream << '}';
    }

    void json::array_in_stream(const json &current_json, std::stringstream &json_string_stream) {
        assert(current_json.my_type_ == json_element_type::array);
        json_string_stream << '[';

        for (size_t i = 0; i < current_json.array_.size();) {
            value_in_stream(current_json.array_[i], json_string_stream);
            if (++i != current_json.array_.size()) {
                json_string_stream << ',';
            }
        }
        json_string_stream << ']';
    }

}// namespace vgd