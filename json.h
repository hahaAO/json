//
// Created by 12552 on 2022/5/29.
//

#ifndef JSON_JSON_H
#define JSON_JSON_H

#include <vector>
#include <map>

namespace vgd {
    class Json_exception : public std::exception {
    private:
        std::string string_;
    public:
        Json_exception(const std::string &s) : string_(s) {};

        const char *what() const noexcept override { return string_.c_str(); };
    };

    enum class json_element_type {
        object,
        array,
        string,
        number,
        bool_true,
        bool_false,
        null
    };

    class json {
    private:
        json() = default;

        static std::map<std::string, json> parse_object(const std::string &json_string, size_t &index);

        static std::vector<json> parse_array(const std::string &json_string, size_t &index);

        static std::string parse_string(const std::string &json_string, size_t &index);

        static double parse_number(const std::string &json_string, size_t &index);

        static void parse_true(const std::string &json_string, size_t &index);

        static void parse_false(const std::string &json_string, size_t &index);

        static void parse_null(const std::string &json_string, size_t &index);

        static void value_in_stream(const json &current_json, std::stringstream &json_string_stream);

        static void object_in_stream(const json &current_json, std::stringstream &json_string_stream);

        static void array_in_stream(const json &current_json, std::stringstream &json_string_stream);

    public:
        static json parse_json(const std::string &json_string);

        static std::string to_string(const json &current_json);

        json_element_type my_type_;
        std::map<std::string, json> object_;
        std::vector<json> array_;
        double number_;
        std::string string_;//TODO： 保存转义字符时没有去除转义，不能直接使用

    };

}// namespace vgd



#endif //JSON_JSON_H
