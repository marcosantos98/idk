#include "str_helper.hpp"

Vec<String> chop_delimiter(String const &str, char delimiter)
{
    Vec<String> words = {};

    int i = 0;

    String tmp;

    while (str[i])
    {
        if (str[i] != delimiter)
        {
            tmp.append({str[i]});
        }
        else
        {
            words.emplace_back(tmp);
            tmp = "";
        }
        i++;
    }

    words.emplace_back(tmp);

    return words;
}